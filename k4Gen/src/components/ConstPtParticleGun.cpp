/*
 * Copyright (c) 2020-2024 Key4hep-Project.
 *
 * This file is part of Key4hep.
 * See https://key4hep.github.io/key4hep-doc/ for further info.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "ConstPtParticleGun.h"
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "HepMC3/GenEvent.h"
#include "HepMC3/GenParticle.h"
#include "HepMC3/GenVertex.h"
#include "HepPDT/ParticleID.hh"
#include "Pythia8/ParticleData.h"
#include <cmath>

DECLARE_COMPONENT(ConstPtParticleGun)

ConstPtParticleGun::ConstPtParticleGun(const std::string& type, const std::string& name, const IInterface* parent)
    : AlgTool(type, name, parent) {
  declareInterface<IParticleGunTool>(this);
}

ConstPtParticleGun::~ConstPtParticleGun() {}

StatusCode ConstPtParticleGun::initialize() {
  StatusCode sc = AlgTool::initialize();
  if (!sc.isSuccess())
    return sc;
  // initialize random number generator
  auto randSvc = service<IRndmGenSvc>("RndmGenSvc", true);
  sc = m_flatGenerator.initialize(randSvc, Rndm::Flat(0., 1.));
  if (!sc.isSuccess()) {
    error() << "Cannot initialize flat generator";
    return StatusCode::FAILURE;
  }
  // check momentum and angles
  if ((m_minEta > m_maxEta) || (m_minPhi > m_maxPhi)) {
    error() << "Incorrect values for eta or phi!";
    return StatusCode::FAILURE;
  }
  m_deltaPhi = m_maxPhi - m_minPhi;
  m_deltaEta = m_maxEta - m_minEta;
  // setup particle information
  m_masses.clear();
  auto pd = Pythia8::ParticleData();
  info() << "Particle type chosen randomly from :";
  PIDs::iterator icode;
  for (icode = m_pdgCodes.begin(); icode != m_pdgCodes.end(); ++icode) {
    info() << " " << *icode;
    m_masses.push_back(pd.m0(*icode));
  }
  info() << endmsg;
  info() << "Eta range: " << m_minEta << "  <-> " << m_maxEta << endmsg;
  info() << "Phi range: " << m_minPhi / Gaudi::Units::rad << " rad <-> " << m_maxPhi / Gaudi::Units::rad << " rad"
         << endmsg;
  // write additional branches
  if (m_writeParticleGunBranches) {
    m_datahandle_particlegun_pt =
        std::make_unique<k4FWCore::DataHandle<float>>("ParticleGun_Pt", Gaudi::DataHandle::Writer, this);
    m_datahandle_particlegun_eta =
        std::make_unique<k4FWCore::DataHandle<float>>("ParticleGun_Eta", Gaudi::DataHandle::Writer, this);
    m_datahandle_particlegun_costheta =
        std::make_unique<k4FWCore::DataHandle<float>>("ParticleGun_costheta", Gaudi::DataHandle::Writer, this);
    m_datahandle_particlegun_phi =
        std::make_unique<k4FWCore::DataHandle<float>>("ParticleGun_Phi", Gaudi::DataHandle::Writer, this);
  }
  return sc;
}

/// Generate the particles
void ConstPtParticleGun::generateParticle(Gaudi::LorentzVector& momentum, Gaudi::LorentzVector& origin, int& pdgId) {
  // smearing of the vertex is done with a vertexsmeartool
  origin.SetCoordinates(0., 0., 0., 0.);
  double px(0.), py(0.), pz(0.);
  // Generate values for eta  and phi
  double phi = m_minPhi + m_flatGenerator() * (m_deltaPhi);
  double eta = m_minEta + m_flatGenerator() * (m_deltaEta);
  double pt = m_minPt + m_flatGenerator() * (m_maxPt - m_minPt);
  if (m_logSpacedPt) {
    pt = pow(10, std::log10(m_minPt) + (std::log10(m_maxPt) - std::log10(m_minPt)) * m_flatGenerator());
  }
  /// if user has provided the list options, use the list
  if (!m_ptList.empty()) {
    pt = m_ptList[m_flatGenerator() * m_ptList.size()];
  }
  if (!m_etaList.empty()) {
    eta = m_etaList[m_flatGenerator() * m_etaList.size()];
  }
  // Transform to x,y,z coordinates
  px = pt * cos(phi);
  py = pt * sin(phi);
  pz = pt * sinh(eta);
  // randomly choose a particle type
  unsigned int currentType = (unsigned int)(m_pdgCodes.size() * m_flatGenerator());
  // protect against funnies
  if (currentType >= m_pdgCodes.size())
    currentType = 0;
  // set momenta
  momentum.SetPx(px);
  momentum.SetPy(py);
  momentum.SetPz(pz);
  momentum.SetE(std::sqrt(m_masses[currentType] * m_masses[currentType] + momentum.P2()));
  pdgId = m_pdgCodes[currentType];
  debug() << " -> " << pdgId << endmsg << "   P   = " << momentum << endmsg;
  /// additional branches in rootfile
  if (m_writeParticleGunBranches) {
    const double hepmcMomentumConversionFactor = 0.001;
    float* _particlegun_phi = new float(phi);
    float* _particlegun_eta = new float(eta);
    float* _particlegun_costheta = new float(cos(2 * atan(exp(eta))));
    float* _particlegun_pt = new float(pt * hepmcMomentumConversionFactor);
    m_datahandle_particlegun_pt->put(_particlegun_pt);
    m_datahandle_particlegun_eta->put(_particlegun_eta);
    m_datahandle_particlegun_costheta->put(_particlegun_costheta);
    m_datahandle_particlegun_phi->put(_particlegun_phi);
  }
}

StatusCode ConstPtParticleGun::getNextEvent(HepMC3::GenEvent& theEvent) {
  Gaudi::LorentzVector theFourMomentum;
  Gaudi::LorentzVector origin;
  // note: pgdid is set in function generateParticle
  int thePdgId;
  generateParticle(theFourMomentum, origin, thePdgId);

  // create HepMC Vertex --
  // by calling add_vertex(), the hepmc event is given ownership of the vertex

  auto v = std::make_shared<HepMC3::GenVertex>(HepMC3::FourVector(origin.X(), origin.Y(), origin.Z(), origin.T()));
  // create HepMC particle --
  // by calling add_particle_out(), the hepmc vertex is given ownership of the particle
  // need to convert from Gaudi Units  (MeV) to (GeV)
  const double hepmcMomentumConversionFactor = 0.001;
  auto p =
      std::make_shared<HepMC3::GenParticle>(HepMC3::FourVector(theFourMomentum.Px() * hepmcMomentumConversionFactor,
                                                               theFourMomentum.Py() * hepmcMomentumConversionFactor,
                                                               theFourMomentum.Pz() * hepmcMomentumConversionFactor,
                                                               theFourMomentum.E() * hepmcMomentumConversionFactor),
                                            thePdgId,
                                            1); // hepmc status code for final state particle
  v->add_particle_out(p);
  theEvent.add_vertex(v);
  // no longer needed in hepmc3?
  // theEvent.set_signal_process_vertex(v);
  return StatusCode::SUCCESS;
}
