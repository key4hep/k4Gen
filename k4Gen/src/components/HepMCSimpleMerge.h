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

#ifndef GENERATION_HEPMCPILEMERGETOOL_H
#define GENERATION_HEPMCPILEMERGETOOL_H

#include "Generation/IHepMCMergeTool.h"
#include "k4FWCore/DataHandle.h"

#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/RndmGenerators.h"

class HepMCSimpleMerge final : public AlgTool, virtual public IHepMCMergeTool {
public:
  HepMCSimpleMerge(const std::string& type, const std::string& name, const IInterface* parent);

  virtual ~HepMCSimpleMerge() final;

  virtual StatusCode initialize() final;

  virtual StatusCode finalize() final;

  /** Merge the events in the container into the signalEvent
   * For events in the vector, new vertices in signalEvent are created to which the final-state particles are attached.
   *  @param[in/out] signalEvent is the signal event that will be enriched with puEvents from eventVector
   *  @param[in] eventVector is the vector of pile-up GenEvents
   */
  virtual StatusCode merge(HepMC3::GenEvent& signalEvent, const std::vector<HepMC3::GenEvent>& eventVector) final;
};

#endif // GENERATION_HEPMCPILEMERGETOOL_H
