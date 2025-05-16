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
#ifndef GENERATION_PYTHIAINTERFACE_H
#define GENERATION_PYTHIAINTERFACE_H

#include "GaudiKernel/AlgTool.h"
#include "Generation/IHepMCProviderTool.h"
#include "Generation/IVertexSmearingTool.h"
#include "Pythia8Plugins/HepMC3.h"
#include "Pythia8Plugins/PowhegHooks.h"
#include "ResonanceDecayFilterHook.h"
#include "k4FWCore/DataHandle.h"
#include <memory>

// Forward HepMC
namespace HepMC3 {
class GenEvent;
}
// Forward Pythia
#if PYTHIA_VERSION_INTEGER < 8300
class EvtGenDecays;
namespace Pythia8 {
class Pythia;
class SlowJet;
class JetMatchingMadgraph;
class amcnlo_unitarised_interface;
} // namespace Pythia8
#else
namespace Pythia8 {
class EvtGenDecays;
class Pythia;
class SlowJet;
class JetMatchingMadgraph;
class amcnlo_unitarised_interface;
} // namespace Pythia8

#endif

class PythiaInterface : public AlgTool, virtual public IHepMCProviderTool {

public:
  /// Constructor.
  PythiaInterface(const std::string& type, const std::string& name, const IInterface* parent);
  virtual StatusCode initialize();
  virtual StatusCode finalize();
  virtual StatusCode getNextEvent(HepMC3::GenEvent& theEvent);

private:
  /// Pythia8 engine
  std::unique_ptr<Pythia8::Pythia> m_pythiaSignal;
  /// Interface for conversion from Pythia8::Event to HepMC event.
  HepMC3::Pythia8ToHepMC3 m_pythiaToHepMC;
  /// Name of Pythia configuration file with Pythia simulation
  /// settings & input LHE file (if required)
  Gaudi::Property<std::string> m_pythiacard{this, "pythiacard", "Pythia_minbias_pp_100TeV.cmd",
                                            "Name of the Pythia cmd file"};

  /// Extra settings for Pythia
  Gaudi::Property<std::vector<std::string>> m_pythia_extrasettings{
      this, "pythiaExtraSettings", {""}, "Additional strings with Pythia settings, applied after the card."};

  /// Pythia8 engine for jet clustering
  std::unique_ptr<Pythia8::SlowJet> m_slowJet{nullptr};
  // Output handle for ME/PS matching variables
  mutable k4FWCore::DataHandle<std::vector<float>> m_handleMePsMatchingVars{"mePsMatchingVars", Gaudi::DataHandle::Writer, this};

  // Maximum number of aborts before giving up
  int m_maxAborts{0};

  // -- aMCatNLO
  bool m_doMePsMatching{false};
  bool m_doMePsMerging{false};
  /// Pythia8 engine for ME/PS matching
  std::unique_ptr<Pythia8::JetMatchingMadgraph> m_matching{nullptr};
  /// Pythia8 engine for NLO ME/PS merging
  std::unique_ptr<Pythia8::amcnlo_unitarised_interface> m_setting{nullptr};

  // Powheg
  bool m_doPowheg{false};
  unsigned long int m_nISRveto{0};
  unsigned long int m_nFSRveto{0};
  /// Pythia8 engine for Powheg ME/PS merging
  Pythia8::PowhegHooks* m_powhegHooks{nullptr};

  ResonanceDecayFilterHook* m_resonanceDecayFilterHook{nullptr};

  /// flag for additional printouts
  Gaudi::Property<bool> m_printPythiaStatistics{this, "printPythiaStatistics", false, "Print Pythia Statistics"};

  Gaudi::Property<bool> m_doEvtGenDecays{this, "doEvtGenDecays", false, "Do decays with EvtGen"};
  Gaudi::Property<std::string> m_EvtGenDecayFile{this, "EvtGenDecayFile", "Generation/data/DECAY.DEC",
                                                 "Name of the global EvtGen Decay File"};
  Gaudi::Property<std::string> m_UserDecayFile{this, "UserDecayFile", "", "Name of the  EvtGen User Decay File"};
  Gaudi::Property<std::string> m_EvtGenParticleDataFile{this, "EvtGenParticleDataFile", "Generation/data/evt.pdl",
                                                        "Name of the EvtGen Particle Data File"};

  Gaudi::Property<std::vector<int>> m_evtGenExcludes{
      this, "EvtGenExcludes", {}, "PDG IDs of particles not to decay with EvtGen"};
  Pythia8::EvtGenDecays* m_evtgen = nullptr;
};

#endif // GENERATION_PYTHIAINTERFACE_H
