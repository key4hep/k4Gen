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
#include "MomentumRangeParticleGun.h"

#include <cmath>

#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "GaudiKernel/SystemOfUnits.h"

#include "HepMC3/GenEvent.h"
#include "HepMC3/GenParticle.h"
#include "HepMC3/GenVertex.h"
#include "HepPDT/ParticleID.hh"
#include "Pythia8/ParticleData.h"

DECLARE_COMPONENT(MomentumRangeParticleGun)

/// Constructor
MomentumRangeParticleGun::MomentumRangeParticleGun(const std::string& type, const std::string& name,
                                                   const IInterface* parent)
    : AlgTool(type, name, parent) {
  declareInterface<IParticleGunTool>(this);
}

/// Destructor
MomentumRangeParticleGun::~MomentumRangeParticleGun() {}

/// Initialize Particle Gun parameters
StatusCode MomentumRangeParticleGun::initialize() {
  StatusCode sc = AlgTool::initialize();
  if (!sc.isSuccess())
    return sc;

  auto randSvc = service<IRndmGenSvc>("RndmGenSvc", true);
  sc = m_flatGenerator.initialize(randSvc, Rndm::Flat(0., 1.));
  if (!sc.isSuccess()) {
    error() << "Cannot initialize flat generator";
    return StatusCode::FAILURE;
  }

  // Get the mass of the particle to be generated
  //

  // check momentum and angles
  if ((m_minMom > m_maxMom) || (m_minTheta > m_maxTheta) || (m_minPhi > m_maxPhi)) {
    error() << "Incorrect values for momentum, theta or phi!" << endmsg;
    return StatusCode::FAILURE;
  }

  m_deltaMom = m_maxMom - m_minMom;
  m_deltaPhi = m_maxPhi - m_minPhi;
  m_deltaTheta = m_maxTheta - m_minTheta;

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

  info() << "Momentum range: " << m_minMom / Gaudi::Units::GeV << " GeV <-> " << m_maxMom / Gaudi::Units::GeV << " GeV"
         << endmsg;
  info() << "Theta range: " << m_minTheta / Gaudi::Units::rad << " rad <-> " << m_maxTheta / Gaudi::Units::rad << " rad"
         << endmsg;
  info() << "Phi range: " << m_minPhi / Gaudi::Units::rad << " rad <-> " << m_maxPhi / Gaudi::Units::rad << " rad"
         << endmsg;

  return sc;
}

/// Generate the particles
void MomentumRangeParticleGun::generateParticle(Gaudi::LorentzVector& momentum, Gaudi::LorentzVector& origin,
                                                int& pdgId) {

  origin.SetCoordinates(0., 0., 0., 0.);
  double px(0.), py(0.), pz(0.);

  // Generate values for energy, theta and phi
  double p = m_minMom + m_flatGenerator() * (m_deltaMom);
  double theta = m_minTheta + m_flatGenerator() * (m_deltaTheta);
  double phi = m_minPhi + m_flatGenerator() * (m_deltaPhi);

  // Transform to x,y,z coordinates
  double pt = p * sin(theta);
  px = pt * cos(phi);
  py = pt * sin(phi);
  pz = p * cos(theta);

  // randomly choose a particle type
  unsigned int currentType = (unsigned int)(m_pdgCodes.size() * m_flatGenerator());
  // protect against funnies
  if (currentType >= m_pdgCodes.size())
    currentType = 0;

  momentum.SetPx(px);
  momentum.SetPy(py);
  momentum.SetPz(pz);
  momentum.SetE(std::sqrt(m_masses[currentType] * m_masses[currentType] + momentum.P2()));

  pdgId = m_pdgCodes[currentType];

  debug() << " -> " << pdgId << endmsg << "   P   = " << momentum << endmsg;
}

StatusCode MomentumRangeParticleGun::getNextEvent(HepMC3::GenEvent& theEvent) {
  Gaudi::LorentzVector theFourMomentum;
  Gaudi::LorentzVector origin;
  // note: pgdid is set in function generateParticle
  int thePdgId;
  generateParticle(theFourMomentum, origin, thePdgId);

  // create HepMC3 Vertex --
  // by calling add_vertex(), the hepmc event is given ownership of the vertex
  auto v = std::make_shared<HepMC3::GenVertex>(HepMC3::FourVector(origin.X(), origin.Y(), origin.Z(), origin.T()));
  // create HepMC3 particle --
  // by calling add_particle_out(), the hepmc vertex is given ownership of the particle
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
  // no longer needed in hepmc3
  // theEvent.set_signal_process_vertex(v);

  return StatusCode::SUCCESS;
}
