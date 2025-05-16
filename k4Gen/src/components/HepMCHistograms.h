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
#ifndef GENERATION_HEPMCHISTOGRAMS_H
#define GENERATION_HEPMCHISTOGRAMS_H

#include "Gaudi/Algorithm.h"
#include "GaudiKernel/ITHistSvc.h"
#include "HepMC3/GenEvent.h"
#include "k4FWCore/DataHandle.h"

#include "TH1F.h"

class HepMCHistograms : public Gaudi::Algorithm {

public:
  /// Constructor.
  HepMCHistograms(const std::string& name, ISvcLocator* svcLoc);
  /// Initialize.
  virtual StatusCode initialize();
  /// Execute.
  virtual StatusCode execute(const EventContext&) const;
  /// Finalize.
  virtual StatusCode finalize();

private:
  /// Handle for the HepMC to be read
  mutable k4FWCore::DataHandle<HepMC3::GenEvent> m_hepmchandle{"HepMC", Gaudi::DataHandle::Reader, this};

  SmartIF<ITHistSvc> m_ths; ///< THistogram service

  TH1F* m_pt{nullptr};  ///< histogram for pT of particles
  TH1F* m_eta{nullptr}; ///< histogram for pseudorapidity of particles

  TH1F* m_d0{nullptr}; ///< histogram for transversal IP
  TH1F* m_z0{nullptr}; ///< histogram for longidudinal IP
};

#endif // GENERATION_HEPMCHISTOGRAMS_H
