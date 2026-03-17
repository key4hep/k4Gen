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

#ifndef GENERATION_GENALG_H
#define GENERATION_GENALG_H

// Gaudi
#include "Gaudi/Algorithm.h"
#include "GaudiKernel/ToolHandle.h"

// k4FWCore
#include "k4FWCore/DataHandle.h"

// k4Gen
#include "Generation/IHepMCMergeTool.h"
#include "Generation/IHepMCProviderTool.h"
#include "Generation/IPileUpTool.h"
#include "Generation/IVertexSmearingTool.h"

namespace HepMC3 {
class GenEvent;
}

class GenAlg : public Gaudi::Algorithm {

public:
  /// Constructor.
  GenAlg(const std::string& name, ISvcLocator* svcLoc);
  /// Initialize.
  virtual StatusCode initialize();
  /// Execute.
  virtual StatusCode execute(const EventContext&) const;
  /// Finalize.
  virtual StatusCode finalize();

private:
  /// Tool to provide signal event
  mutable ToolHandle<IHepMCProviderTool> m_signalProvider{"MomentumRangeParticleGun/HepMCProviderTool", this};
  /// Tool to determine number of pileup events
  mutable ToolHandle<IPileUpTool> m_pileUpTool{"ConstPileUp/PileUpTool", this};
  /// Tool to provide pile up event(s)
  mutable ToolHandle<IHepMCProviderTool> m_pileUpProvider{"MomentumRangeParticleGun/HepMCProviderTool", this};
  // Tool to smear vertex
  mutable ToolHandle<IVertexSmearingTool> m_vertexSmearingTool{"FlatSmearVertex/VertexSmearingTool", this};
  /// Tool to merge HepMC events
  mutable ToolHandle<IHepMCMergeTool> m_hepmcMergeTool{"HepMCSimpleMerge/HepMCMergeTool", this};
  // Output handle for finished event
  mutable k4FWCore::DataHandle<HepMC3::GenEvent> m_hepmcHandle{"hepmc", Gaudi::DataHandle::Writer, this};
};

#endif // GENERATION_GENALG_H
