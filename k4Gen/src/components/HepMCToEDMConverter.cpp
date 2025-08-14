#include "HepMCToEDMConverter.h"
// HepMC
#include "HepMC3/GenVertex.h"
// HepPDT
#include "HepPDT/ParticleID.hh"
// EDM4hep
#include "edm4hep/MCParticleCollection.h"

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
  // TODO: Figure out what we want to store here and how to retrieve it from HepMC3
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

  if (prodVtx != nullptr) {
    auto& pos = prodVtx->position();
    edm_particle.setVertex({pos.x(), pos.y(), pos.z()});
  }

  auto endVtx = hepmcParticle->end_vertex();
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
}

StatusCode HepMCToEDMConverter::initialize() { return Gaudi::Algorithm::initialize(); }

StatusCode HepMCToEDMConverter::execute(const EventContext&) const {
  const HepMC3::GenEvent* evt = m_hepmchandle.get();
  edm4hep::MCParticleCollection* particles = new edm4hep::MCParticleCollection();

  std::unordered_map<unsigned int, edm4hep::MutableMCParticle> _map;
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
  for (auto particle_pair : _map) {
    particles->push_back(particle_pair.second);
  }
  m_genphandle.put(particles);
  return StatusCode::SUCCESS;
}

StatusCode HepMCToEDMConverter::finalize() { return Gaudi::Algorithm::finalize(); }
