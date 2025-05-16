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
#ifndef GENERATION_MOMENTUMRANGEPARTICLEGUN_H
#define GENERATION_MOMENTUMRANGEPARTICLEGUN_H

#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "GaudiKernel/RndmGenerators.h"
#include "GaudiKernel/SystemOfUnits.h"

#include "Generation/IParticleGunTool.h"

/** @class MomentumRangeParticleGun MomentumRangeParticleGun.h "MomentumRangeParticleGun.h"
 *
 *  Particle gun with given momentum range
 *
 *  @author Patrick Robbe (adaptation to tool structure)
 *  @author Benedikt Hegner (adaption for non LHCb use cases)
 *  @author Valentin Volkl (rewrite as tool)
 *  @date   2008-05-18
 */
class MomentumRangeParticleGun : public AlgTool, virtual public IParticleGunTool {

public:
  /// Constructor
  MomentumRangeParticleGun(const std::string& type, const std::string& name, const IInterface* parent);

  /// Destructor
  virtual ~MomentumRangeParticleGun();

  /// Initialize particle gun parameters
  virtual StatusCode initialize();

  /// Generation of particles
  virtual void generateParticle(Gaudi::LorentzVector& momentum, Gaudi::LorentzVector& origin, int& pdgId);

  /// Print counters
  virtual void printCounters() { ; };
  virtual StatusCode getNextEvent(HepMC3::GenEvent&);

private:
  /// Minimum momentum (Set by options)
  Gaudi::Property<double> m_minMom{this, "MomentumMin", 100.0 * Gaudi::Units::GeV, "Minimal momentum"};
  /// Minimum theta angle (Set by options)
  Gaudi::Property<double> m_minTheta{this, "ThetaMin", 0.1 * Gaudi::Units::rad, "Minimal theta"};
  /// Minimum phi angle (Set by options)
  Gaudi::Property<double> m_minPhi{this, "PhiMin", 0. * Gaudi::Units::rad, "Minimal phi"};

  /// Maximum momentum (Set by options)
  Gaudi::Property<double> m_maxMom{this, "MomentumMax", 100.0 * Gaudi::Units::GeV, "Maximal momentum"};
  /// Maximum theta angle (Set by options)
  Gaudi::Property<double> m_maxTheta{this, "ThetaMax", 0.4 * Gaudi::Units::rad, "Maximal theta"};
  /// Maximum phi angle (Set by options)
  Gaudi::Property<double> m_maxPhi{this, "PhiMax", Gaudi::Units::twopi* Gaudi::Units::rad, "Maximal phi"};

  /// Momentum range
  double m_deltaMom;
  /// Theta range
  double m_deltaTheta;
  /// Phi range
  double m_deltaPhi;

  /// Pdg Codes of particles to generate (Set by options)
  Gaudi::Property<std::vector<int>> m_pdgCodes{this, "PdgCodes", {-211}, "list of pdg codes to produce"};

  /// Masses of particles to generate (derived from PDG codes)
  std::vector<double> m_masses;

  /// Names of particles to generate (derived from PDG codes)
  std::vector<std::string> m_names;

  /// Flat random number generator
  Rndm::Numbers m_flatGenerator;
};

#endif // GENERATION_MOMENTUMRANGEPARTICLEGUN_H
