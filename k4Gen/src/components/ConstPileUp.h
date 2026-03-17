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
#ifndef GENERATION_CONSTPILEUP_H
#define GENERATION_CONSTPILEUP_H

#include "GaudiKernel/AlgTool.h"
#include "Generation/IPileUpTool.h"

/** @class ConstPileUp
 *
 *  Tool to generate number of pile-up events to be mixed with signal event.
 *  Concrete implementation of a IPileUpTool -- the most trivial one, actually,
 *  returning just a constant (that can be specified as a property).
 *  The interface is kept close to the originals in LHCb's Gauss(ino).
 *
 *  @author Valentin Volkl
 *  @date   2015-12-16
 */
class ConstPileUp : public AlgTool, virtual public IPileUpTool {
public:
  ConstPileUp(const std::string& type, const std::string& name, const IInterface* parent);
  virtual ~ConstPileUp();
  virtual StatusCode initialize();
  /// in the special case of constant pileup,
  /// this method is identical to getMeanPileUp
  virtual unsigned int numberOfPileUp();
  virtual double getMeanPileUp();
  virtual void printPileUpCounters();

private:
  /// Number of min bias events to pile on signal event.
  Gaudi::Property<unsigned int> m_numPileUpEvents{this, "numPileUpEvents", 0, "number of pile-up events"};
};

#endif // GENERATION_CONSTPILEUP_H
