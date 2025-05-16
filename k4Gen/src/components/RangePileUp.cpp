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
#include "RangePileUp.h"

DECLARE_COMPONENT(RangePileUp)

RangePileUp::RangePileUp(const std::string& type, const std::string& name, const IInterface* parent)
    : AlgTool(type, name, parent) {
  declareInterface<IPileUpTool>(this);
}

RangePileUp::~RangePileUp() { ; }

StatusCode RangePileUp::initialize() {
  StatusCode sc = AlgTool::initialize();
  if (sc.isFailure())
    return sc;
  return sc;
}

unsigned int RangePileUp::numberOfPileUp() {
  m_currentNumPileUpEvents = m_pileupRange[m_rangeIndex];
  m_rangeIndex = (m_rangeIndex + 1) % m_pileupRange.size();
  return m_currentNumPileUpEvents;
}

double RangePileUp::getMeanPileUp() { return m_currentNumPileUpEvents; }

void RangePileUp::printPileUpCounters() {
  info() << "Current number of pileup events:  " << m_currentNumPileUpEvents << endmsg;
}
