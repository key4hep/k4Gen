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
#ifndef GENERATION_EDMTOHEPMCCONVERTER_H
#define GENERATION_EDMTOHEPMCCONVERTER_H

#include "Gaudi/Algorithm.h"
#include "HepMC3/GenEvent.h"
#include "HepMC3/Units.h"
#include "k4FWCore/DataHandle.h"

namespace edm4hep {
class MCParticleCollection;
}

class EDMToHepMCConverter : public Gaudi::Algorithm {

public:
  /// Constructor.
  EDMToHepMCConverter(const std::string& name, ISvcLocator* svcLoc);
  /// Initialize.
  virtual StatusCode initialize();
  /// Execute.
  virtual StatusCode execute(const EventContext&) const;
  /// Finalize.
  virtual StatusCode finalize();

private:
  /// Handle for the HepMC to be read
  mutable k4FWCore::DataHandle<HepMC3::GenEvent> m_hepmchandle{"hepmc", Gaudi::DataHandle::Writer, this};
  /// Handle for the genparticles to be written
  mutable k4FWCore::DataHandle<edm4hep::MCParticleCollection> m_genphandle{"GenParticles", Gaudi::DataHandle::Reader, this};
};

#endif
