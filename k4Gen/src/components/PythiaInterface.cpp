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
#include "PythiaInterface.h"

#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/System.h"

#include "Pythia8/Pythia.h"
// Include UserHooks for Jet Matching.
#include "Pythia8Plugins/CombineMatchingInput.h"
#include "Pythia8Plugins/JetMatching.h"
// Include UserHooks for randomly choosing between integrated and
// non-integrated treatment for unitarised merging.
#include "HepMC3/GenEvent.h"
#include "Pythia8Plugins/EvtGen.h"
#include "Pythia8Plugins/aMCatNLOHooks.h"

DECLARE_COMPONENT(PythiaInterface)

PythiaInterface::PythiaInterface(const std::string& type, const std::string& name, const IInterface* parent)
    : AlgTool(type, name, parent), m_pythiaSignal(nullptr), m_maxAborts(0), m_doMePsMatching(0), m_doMePsMerging(0),
      m_matching(nullptr), m_setting(nullptr) {}

StatusCode PythiaInterface::initialize() {
  {
    StatusCode sc = AlgTool::initialize();
    if (!sc.isSuccess())
      return sc;
  }

  if (m_pythiacard.empty() && m_pythia_extrasettings.size() < 2) {
    error() << "No Pythia8 configuration file (*.cmd) defined!" << endmsg;
    return StatusCode::FAILURE;
  }

  // Set Pythia configuration directory from system variable (if set)
  std::string xmlpath = "";
  if (System::getEnv("PYTHIA8_XML") != "UNKNOWN")
    xmlpath = System::getEnv("PYTHIA8_XML");

  // Initialize Pythia8
  m_pythiaSignal = std::make_unique<Pythia8::Pythia>(xmlpath);

  // Add settings for resonance decay filter
  m_pythiaSignal->settings.addFlag("ResonanceDecayFilter:filter", false);
  m_pythiaSignal->settings.addFlag("ResonanceDecayFilter:exclusive", false);
  m_pythiaSignal->settings.addFlag("ResonanceDecayFilter:eMuAsEquivalent", false);
  m_pythiaSignal->settings.addFlag("ResonanceDecayFilter:eMuTauAsEquivalent", false);
  m_pythiaSignal->settings.addFlag("ResonanceDecayFilter:allNuAsEquivalent", false);
  m_pythiaSignal->settings.addFlag("ResonanceDecayFilter:udscAsEquivalent", false);
  m_pythiaSignal->settings.addFlag("ResonanceDecayFilter:udscbAsEquivalent", false);
  m_pythiaSignal->settings.addFlag("ResonanceDecayFilter:wzAsEquivalent", false);
  m_pythiaSignal->settings.addMVec("ResonanceDecayFilter:mothers", std::vector<int>(), false, false, 0, 0);
  m_pythiaSignal->settings.addMVec("ResonanceDecayFilter:daughters", std::vector<int>(), false, false, 0, 0);

  // Read Pythia configuration file
  m_pythiaSignal->readFile(m_pythiacard.value().c_str());

  // Apply any extra Pythia8 settings
  for (auto pythiacommand : m_pythia_extrasettings) {
    m_pythiaSignal->settings.readString(pythiacommand);
  }

  // Initialize variables from configuration file
  m_maxAborts = m_pythiaSignal->settings.mode("Main:timesAllowErrors"); // how many aborts before run stops

  // Begin ME/PS Matching specific code
  // Check if jet matching should be applied.
  m_doMePsMatching = m_pythiaSignal->settings.flag("JetMatching:merge");

  // Check if internal merging should be applied.
  m_doMePsMerging = !(m_pythiaSignal->settings.word("Merging:Process").compare("void") == 0);

  // Currently, only one scheme at a time is allowed.
  if (m_doMePsMerging && m_doMePsMatching) {
    error() << "Jet matching and merging cannot be used simultaneously!" << endmsg;
    return StatusCode::FAILURE;
  }

  // Allow to set the number of additional partons dynamically.
  if (m_doMePsMerging) {
    // Store merging scheme.
    int scheme;
    if (m_pythiaSignal->settings.flag("Merging:doUMEPSTree") || m_pythiaSignal->settings.flag("Merging:doUMEPSSubt")) {
      scheme = 1;
    } else if (m_pythiaSignal->settings.flag("Merging:doUNLOPSTree") ||
               m_pythiaSignal->settings.flag("Merging:doUNLOPSSubt") ||
               m_pythiaSignal->settings.flag("Merging:doUNLOPSLoop") ||
               m_pythiaSignal->settings.flag("Merging:doUNLOPSSubtNLO")) {
      scheme = 2;
    } else {
      scheme = 0;
    }

    m_setting = std::unique_ptr<Pythia8::amcnlo_unitarised_interface>(new Pythia8::amcnlo_unitarised_interface(scheme));
    m_pythiaSignal->setUserHooksPtr((Pythia8::UserHooksPtr)m_setting.get());
  }

  // For jet matching, initialise the respective user hooks code.
  if (m_doMePsMatching) {
    m_matching = std::unique_ptr<Pythia8::JetMatchingMadgraph>(new Pythia8::JetMatchingMadgraph());
    if (!m_matching) {
      error() << "Failed to initialise jet matching structures." << endmsg;
      return StatusCode::FAILURE;
    }
    m_pythiaSignal->setUserHooksPtr((Pythia8::UserHooksPtr)m_matching.get());
  }

  // Jet clustering needed for matching
  m_slowJet = std::make_unique<Pythia8::SlowJet>(1, 0.4, 0, 4.4, 2, 2, nullptr, false);

  // End ME/PS Matching specific code

  // --  POWHEG settings
  int vetoMode = m_pythiaSignal->settings.mode("POWHEG:veto");
  int MPIvetoMode = m_pythiaSignal->settings.mode("POWHEG:MPIveto");
  m_doPowheg = (vetoMode > 0 || MPIvetoMode > 0);

  // Add in user hooks for shower vetoing
  if (m_doPowheg) {
    // Counters for number of ISR/FSR emissions vetoed
    m_nISRveto = 0, m_nFSRveto = 0;

    // Set ISR and FSR to start at the kinematical limit
    if (vetoMode > 0) {
      m_pythiaSignal->readString("SpaceShower:pTmaxMatch = 2");
      m_pythiaSignal->readString("TimeShower:pTmaxMatch = 2");
    }

    // Set MPI to start at the kinematical limit
    if (MPIvetoMode > 0) {
      m_pythiaSignal->readString("MultipartonInteractions:pTmaxMatch = 2");
    }

    m_powhegHooks = new Pythia8::PowhegHooks();
    m_pythiaSignal->setUserHooksPtr((Pythia8::UserHooksPtr)m_powhegHooks);
  }
  bool resonanceDecayFilter = m_pythiaSignal->settings.flag("ResonanceDecayFilter:filter");
  if (resonanceDecayFilter) {
    m_resonanceDecayFilterHook = new ResonanceDecayFilterHook();
    m_pythiaSignal->addUserHooksPtr((Pythia8::UserHooksPtr)m_resonanceDecayFilterHook);
  }

  // Set up evtGen
  if (m_doEvtGenDecays) {
    m_evtgen = new Pythia8::EvtGenDecays(
        m_pythiaSignal.get(),             // the pythia instance
        m_EvtGenDecayFile.value(),        // the file name of the evtgen decay file
        m_EvtGenParticleDataFile.value(), // the file name of the evtgen data file
        nullptr, // the optional EvtExternalGenList pointer (must be be provided if the next argument is provided to
                 // avoid double initializations)
        nullptr, // the EvtAbsRadCorr pointer to pass to EvtGen
        1,       // the mixing type to pass to EvtGen
        false,   // a flag to use XML files to pass to EvtGen
        true,    // a flag to limit decays based on the Pythia criteria (based on the particle decay vertex)
        true,    // a flag to use external models with EvtGen
        false);  // a flag if an FSR model should be passed to EvtGen (pay attention to this, default is true)
    if (!m_UserDecayFile.empty()) {
      m_evtgen->readDecayFile(m_UserDecayFile);
    }
    // Possibility to force Pythia8 to do decays
    for (auto _pdgid : m_evtGenExcludes) {
      m_evtgen->exclude(_pdgid);
    }
  }

  m_pythiaSignal->init();

  return StatusCode::SUCCESS;
}

StatusCode PythiaInterface::getNextEvent(HepMC3::GenEvent& theEvent) {
  // Generate events. Quit if many failures in a row
  int nAborts = 0;
  while (!m_pythiaSignal->next()) {
    if (++nAborts > m_maxAborts) {
      IIncidentSvc* incidentSvc;
      incidentSvc = service<IIncidentSvc>("IncidentSvc", false);
      incidentSvc->fireIncident(Incident(name(), IncidentType::AbortEvent));
      error() << "Event generation aborted prematurely, owing to error!" << endmsg;
      return StatusCode::FAILURE;
    } else {
      warning() << "PythiaInterface Pythia8 abort : " << nAborts << "/" << m_maxAborts << std::endl;
    }
  }

  if (m_doEvtGenDecays) {
    m_evtgen->decay();
  }
  if (m_doMePsMatching || m_doMePsMerging) {
    auto mePsMatchingVars = m_handleMePsMatchingVars.createAndPut();
    int njetNow = 0;
    std::vector<double> dijVec;

    // Construct input for jet algorithm.
    Pythia8::Event jetInput;
    jetInput.init("jet input", &(m_pythiaSignal->particleData));
    jetInput.clear();
    for (int i = 0; i < m_pythiaSignal->event.size(); ++i)
      if (m_pythiaSignal->event[i].isFinal() &&
          (m_pythiaSignal->event[i].colType() != 0 || m_pythiaSignal->event[i].isHadron()))
        jetInput.append(m_pythiaSignal->event[i]);
    m_slowJet->setup(jetInput);
    // Run jet algorithm.
    std::vector<double> result;
    while (m_slowJet->sizeAll() - m_slowJet->sizeJet() > 0) {
      result.push_back(sqrt(m_slowJet->dNext()));
      m_slowJet->doStep();
    }

    // Reorder by decreasing multiplicity.
    for (int i = int(result.size()) - 1; i >= 0; --i)
      dijVec.push_back(result[i]);

    // Now get the "number of partons" in the input event, so that
    // we may tag this event accordingly when histogramming. Note
    // that for MLM jet matching, this might not coincide with the
    // actual number of partons in the input LH event, since some
    // partons may be excluded from the matching.

    bool doShowerKt = m_pythiaSignal->settings.flag("JetMatching:doShowerKt");
    if (m_doMePsMatching && !doShowerKt)
      njetNow = m_matching->nMEpartons().first;
    else if (m_doMePsMatching && doShowerKt) {
      njetNow = m_matching->getProcessSubset().size();
    } else if (m_doMePsMerging) {
      njetNow = m_pythiaSignal->settings.mode("Merging:nRequested");
      if (m_pythiaSignal->settings.flag("Merging:doUMEPSSubt") ||
          m_pythiaSignal->settings.flag("Merging:doUNLOPSSubt") ||
          m_pythiaSignal->settings.flag("Merging:doUNLOPSSubtNLO"))
        njetNow--;
    }

    // Inclusive jet pTs as further validation plot.
    std::vector<double> ptVec;
    // Run jet algorithm.
    m_slowJet->analyze(jetInput);
    for (int i = 0; i < m_slowJet->sizeJet(); ++i)
      ptVec.push_back(m_slowJet->pT(i));

    // 0th entry = number of generated partons
    mePsMatchingVars->push_back(njetNow);

    // odd  entries: d(ij) observables --- 1): d01, 3): d12, 5): d23, 7): d34
    // even entries: pT(i) observables --- 2): pT1, 4): pT2, 6): pT3, 8): pT4
    for (unsigned int i = 0; i < 4; ++i) {
      if (dijVec.size() > i) {
        mePsMatchingVars->push_back(log10(dijVec[i]));
      } else {
        mePsMatchingVars->push_back(-999);
      }

      if (ptVec.size() > i) {
        mePsMatchingVars->push_back(ptVec[i]);
      } else {
        mePsMatchingVars->push_back(-999);
      }
    }
  }

  // Print debug: Pythia event info
  if (msgLevel() <= MSG::VERBOSE) {
    for (int i = 0; i < m_pythiaSignal->event.size(); ++i) {
      verbose() << "PythiaInterface Pythia8 aborts : " << nAborts << "/" << m_maxAborts << endmsg;

      verbose() << "Pythia: "
                << " Id: " << std::setw(3) << i << " PDG: " << std::setw(5) << m_pythiaSignal->event[i].id()
                << " Mothers: " << std::setw(3) << m_pythiaSignal->event[i].mother1() << " -> " << std::setw(3)
                << m_pythiaSignal->event[i].mother2() << " Daughters: " << std::setw(3)
                << m_pythiaSignal->event[i].daughter1() << " -> " << std::setw(3)
                << m_pythiaSignal->event[i].daughter2() << " Stat: " << std::setw(2)
                << m_pythiaSignal->event[i].status() << std::scientific << std::setprecision(2)
                << " Px: " << std::setw(9) << m_pythiaSignal->event[i].px() << std::setprecision(2)
                << " Py: " << std::setw(9) << m_pythiaSignal->event[i].py() << std::setprecision(2)
                << " Pz: " << std::setw(9) << m_pythiaSignal->event[i].pz() << std::setprecision(2)
                << " E: " << std::setw(9) << m_pythiaSignal->event[i].e() << std::setprecision(2)
                << " M: " << std::setw(9) << m_pythiaSignal->event[i].m() << std::fixed << endmsg;
    }
  } // Debug

  m_pythiaToHepMC.fill_next_event(*m_pythiaSignal, theEvent);

  // Print debug: HepMC event info
  if (msgLevel() <= MSG::VERBOSE) {
    for (auto ipart : theEvent.particles()) {

      int motherID = -1;
      int motherIDRange = 0;
      if (ipart->production_vertex() != nullptr) {
        if (!ipart->production_vertex()->particles_in().empty()) {
          motherID = ipart->production_vertex()->particles_in()[0]->id();
          motherIDRange = ipart->production_vertex()->particles_in().size() - 1;
        }
      }

      int daughterID = -1;
      int daughterIDRange = 0;
      if (ipart->end_vertex() != nullptr) {
        if (!ipart->end_vertex()->particles_out().empty()) {
          daughterID = ipart->end_vertex()->particles_out()[0]->id();
          daughterIDRange = ipart->end_vertex()->particles_out().size() - 1;
        }
      }

      verbose() << "HepMC: "
                << " ID: " << std::setw(3) << ipart->id() << " PDG: " << std::setw(5) << ipart->pdg_id()
                << " Mothers: " << std::setw(3) << motherID << " -> " << std::setw(3) << motherID + motherIDRange
                << " Daughters: " << std::setw(3) << daughterID << " -> " << std::setw(3)
                << daughterID + daughterIDRange << " Stat: " << std::setw(2) << ipart->status() << std::scientific
                << " Px: " << std::setprecision(2) << std::setw(9) << ipart->momentum().px()
                << " Py: " << std::setprecision(2) << std::setw(9) << ipart->momentum().py()
                << " Pz: " << std::setprecision(2) << std::setw(9) << ipart->momentum().pz()
                << " E: " << std::setprecision(2) << std::setw(9) << ipart->momentum().e()
                << " M: " << std::setprecision(2) << std::setw(9) << ipart->momentum().m() << std::fixed;
      if (ipart->production_vertex() != nullptr) {
        verbose() << std::scientific << " Vx: " << std::setprecision(2) << std::setw(9)
                  << ipart->production_vertex()->position().x() << " Vy: " << std::setprecision(2) << std::setw(9)
                  << ipart->production_vertex()->position().y() << " Vz: " << std::setprecision(2) << std::setw(9)
                  << ipart->production_vertex()->position().z() << " T: " << std::setprecision(2) << std::setw(9)
                  << ipart->production_vertex()->position().t() << std::fixed;
      }
      verbose() << endmsg;
    }
  } // Debug

  if (m_doPowheg) {
    m_nISRveto += m_powhegHooks->getNISRveto();
    m_nFSRveto += m_powhegHooks->getNFSRveto();
  }

  if (m_printPythiaStatistics) {
    m_pythiaSignal->stat();
  }

  return StatusCode::SUCCESS;
}

StatusCode PythiaInterface::finalize() {

  if (m_doPowheg) {
    debug() << "POWHEG INFO: Number of ISR emissions vetoed: " << m_nISRveto << endmsg;
    debug() << "POWHEG INFO: Number of FSR emissions vetoed: " << m_nFSRveto << endmsg;
  }

  m_pythiaSignal.reset();
  if (nullptr != m_evtgen) {
    delete m_evtgen;
  }
  return AlgTool::finalize();
}
