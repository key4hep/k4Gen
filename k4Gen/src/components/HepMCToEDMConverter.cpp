#include "HepMCToEDMConverter.h"
// Gaudi
#include "GaudiKernel/PhysicalConstants.h"
// HepMC
#include "HepMC3/GenRunInfo.h"
#include "HepMC3/GenVertex.h"
// std
#include <string>
#include <unordered_map>
#include <vector>
// HepPDT
#include "HepPDT/ParticleID.hh"
// EDM4hep
#include "edm4hep/EventHeaderCollection.h"
#include "edm4hep/MCParticleCollection.h"
// k4FWCore
#include "k4FWCore/MetadataUtils.h"

DECLARE_COMPONENT(HepMCToEDMConverter)

edm4hep::MutableMCParticle
HepMCToEDMConverter::convert(std::shared_ptr<const HepMC3::GenParticle> hepmcParticle) const {
  edm4hep::MutableMCParticle edm_particle;
  edm_particle.setPDG(hepmcParticle->pdg_id());
  edm_particle.setGeneratorStatus(hepmcParticle->status());
  // look up charge from pdg_id
  HepPDT::ParticleID particleID(hepmcParticle->pdg_id());
  edm_particle.setCharge(static_cast<float>(particleID.charge()));
  // convert momentum
  auto p = hepmcParticle->momentum();
  edm_particle.setMomentum({p.px(), p.py(), p.pz()});
  edm_particle.setMass(hepmcParticle->generated_mass());

#ifdef EDM4HEP_MCPARTICLE_HAS_HELICITY
  // Use the default from EDM4hep: -9
#else
  // add spin (particle helicity) information if available
  std::shared_ptr<HepMC3::VectorFloatAttribute> spin = hepmcParticle->attribute<HepMC3::VectorFloatAttribute>("spin");
  if (spin) {
    edm4hep::Vector3f hel(spin->value()[0], spin->value()[1], spin->value()[2]);
    edm_particle.setSpin(hel);
  }
#endif

  // convert vertex info
  auto prodVtx = hepmcParticle->production_vertex();
  auto endVtx  = hepmcParticle->end_vertex();

  if (prodVtx != nullptr) {
    auto& pos = prodVtx->position();
    edm_particle.setVertex({pos.x(), pos.y(), pos.z()});
    // HepMC3 stores c*t as a length, EDM4hep the time in ns
    edm_particle.setTime(pos.t() / Gaudi::Units::c_light);
  }

  if (endVtx != nullptr) {
    auto& pos = endVtx->position();
    edm_particle.setEndpoint({pos.x(), pos.y(), pos.z()});
  }

  return edm_particle;
}

HepMCToEDMConverter::HepMCToEDMConverter(const std::string& name, ISvcLocator* svcLoc)
    : Gaudi::Algorithm(name, svcLoc) {
  declareProperty("hepmc", m_hepmchandle, "HepMC event handle (input)");
  declareProperty("GenParticles", m_genphandle, "Generated particles collection (output)");
  declareProperty("EventHeader", m_eventHeaderHandle,
                  "Event header with the event weights (output), only written if a path is set");
}

StatusCode HepMCToEDMConverter::execute(const EventContext&) const {
  const HepMC3::GenEvent* evt = m_hepmchandle.get();
  edm4hep::MCParticleCollection* particles = new edm4hep::MCParticleCollection();

  // unordered_map for O(1) lookups; ordering is recovered in the flush loop below
  // by re-iterating evt->particles(), which HepMC3 guarantees is in sequential ID order.
  std::unordered_map<unsigned int, edm4hep::MutableMCParticle> _map;
  _map.reserve(evt->particles().size());
  for (auto _p : evt->particles()) {
    verbose() << "Converting HepMC particle with PDG ID \"" << _p->pdg_id() << "\" and ID \"" << _p->id() << "\""
              << endmsg;
    if (_map.find(_p->id()) == _map.end()) {
      edm4hep::MutableMCParticle edm_particle = convert(_p);
      _map.insert({_p->id(), edm_particle});
    }
    // mother/daughter links
    auto prodvertex = _p->production_vertex();
    if (nullptr != prodvertex) {
      for (auto particle_mother : prodvertex->particles_in()) {
        if (_map.find(particle_mother->id()) == _map.end()) {
          edm4hep::MutableMCParticle edm_particle = convert(particle_mother);
          _map.insert({particle_mother->id(), edm_particle});
        }
        _map[_p->id()].addToParents(_map[particle_mother->id()]);
      }
    }
    auto endvertex = _p->end_vertex();
    if (nullptr != endvertex) {
      for (auto particle_daughter : endvertex->particles_out()) {
        if (_map.find(particle_daughter->id()) == _map.end()) {
          auto edm_particle = convert(particle_daughter);
          _map.insert({particle_daughter->id(), edm_particle});
        }
        _map[_p->id()].addToDaughters(_map[particle_daughter->id()]);
      }
    }
  }
  // Flush in HepMC3 ID order: evt->particles() is already in sequential ID order.
  for (auto _p : evt->particles()) {
    particles->push_back(_map[_p->id()]);
  }
  m_genphandle.put(particles);

  if (!m_eventHeaderHandle.objKey().empty()) {
    auto* headers = new edm4hep::EventHeaderCollection();
    auto header = headers->create();
    header.setEventNumber(evt->event_number());
    header.setRunNumber(m_runNumber);
    const auto& weights = evt->weights();
    header.setWeight(weights.empty() ? 1.0 : weights[0]);
    // Only nominal weight: no weights vector, as in k4SimDelphes
    if (weights.size() > 1) {
      for (const auto weight : weights) {
        header.addToWeights(weight);
      }
    }
    m_eventHeaderHandle.put(headers);
  }

  // Metadata cannot be put during the event loop, keep the run information for finalize
  if (evt->run_info()) {
    m_runInfo = evt->run_info();
  }

  return StatusCode::SUCCESS;
}

StatusCode HepMCToEDMConverter::finalize() {
  if (m_runInfo) {
    const auto& weightNames = m_runInfo->weight_names();
    if (weightNames.size() > 1) {
      k4FWCore::putParameter(edm4hep::labels::EventWeightsNames, weightNames, this);
    }
    if (!m_runInfo->tools().empty()) {
      std::vector<std::string> names, versions, descriptions;
      for (const auto& tool : m_runInfo->tools()) {
        names.push_back(tool.name);
        versions.push_back(tool.version);
        descriptions.push_back(tool.description);
      }
      k4FWCore::putParameter(edm4hep::labels::GeneratorToolNames, names, this);
      k4FWCore::putParameter(edm4hep::labels::GeneratorToolVersions, versions, this);
      k4FWCore::putParameter(edm4hep::labels::GeneratorToolDescriptions, descriptions, this);
    }
  }
  return Gaudi::Algorithm::finalize();
}
