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
#include "PoissonPileUp.h"
#include "GaudiKernel/IRndmGenSvc.h"

DECLARE_COMPONENT(PoissonPileUp)

PoissonPileUp::PoissonPileUp(const std::string& type, const std::string& name, const IInterface* parent)
    : AlgTool(type, name, parent) {
  declareInterface<IPileUpTool>(this);
}

PoissonPileUp::~PoissonPileUp() { ; }

StatusCode PoissonPileUp::initialize() {
  StatusCode sc = AlgTool::initialize();
  if (sc.isFailure())
    return sc;
  auto randSvc = service<IRndmGenSvc>("RndmGenSvc", true);
  if (m_meanPileUpEvents < 0) {
    error() << "Number of Pileup events cannot be negative!";
    return StatusCode::FAILURE;
  }
  sc = m_PoissonDist.initialize(randSvc, Rndm::Poisson(m_meanPileUpEvents));
  if (!sc.isSuccess()) {
    error() << "Could not initialize Poisson random number generator";
    return StatusCode::FAILURE;
  }
  m_currentNumPileUpEvents = m_PoissonDist();
  printPileUpCounters();
  return sc;
}

unsigned int PoissonPileUp::numberOfPileUp() {
  m_currentNumPileUpEvents = m_PoissonDist();
  return m_currentNumPileUpEvents;
}

double PoissonPileUp::getMeanPileUp() { return m_meanPileUpEvents; }

void PoissonPileUp::printPileUpCounters() {
  info() << "Current number of pileup events:  " << m_currentNumPileUpEvents << endmsg;
}
