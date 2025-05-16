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
#ifndef GENERATION_POISSONPILEUP_H
#define GENERATION_POISSONPILEUP_H

#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/RndmGenerators.h"
#include "Generation/IPileUpTool.h"

/** @class PoissonPileUp
 *
 *  Tool to generate number of pile-up events to be mixed with signal event.
 *  Concrete implementation of a IPileUpTool, returning a random variable
 *   drawn from a Poisson distribution centered around the average number
 *   of pileup events.
 *
 *  @author Valentin Volkl
 *  @date   2016-01-18
 */
class PoissonPileUp : public AlgTool, virtual public IPileUpTool {
public:
  PoissonPileUp(const std::string& type, const std::string& name, const IInterface* parent);
  virtual ~PoissonPileUp();
  virtual StatusCode initialize();
  virtual unsigned int numberOfPileUp();
  virtual double getMeanPileUp();
  virtual void printPileUpCounters();

private:
  /// average number of min bias events to pile on signal event.
  Gaudi::Property<double> m_meanPileUpEvents{this, "numPileUpEvents", 0, "Average number of pile-up events"};
  /// holds last realization of the random variable
  unsigned int m_currentNumPileUpEvents;
  /// Poisson random number generator
  Rndm::Numbers m_PoissonDist;
};

#endif // GENERATION_POISSONPILEUP_H
