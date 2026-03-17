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

#include "GenAlg.h"

// Gaudi
#include "GaudiKernel/IEventProcessor.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/Incident.h"

// HepMC3
#include "HepMC3/GenEvent.h"

DECLARE_COMPONENT(GenAlg)

GenAlg::GenAlg(const std::string& name, ISvcLocator* svcLoc) : Gaudi::Algorithm(name, svcLoc) {
  declareProperty("SignalProvider", m_signalProvider, "Signal events provider tool");
  declareProperty("PileUpTool", m_pileUpTool, "Pileup tool");
  declareProperty("PileUpProvider", m_pileUpProvider, "Pileup events provider tool");
  declareProperty("VertexSmearingTool", m_vertexSmearingTool, "Vertex smearing tool");
  declareProperty("HepMCMergeTool", m_hepmcMergeTool, "Event merge tool");
  declareProperty("hepmc", m_hepmcHandle, "HepMC event handle (output)");
}

StatusCode GenAlg::initialize() {
  {
    StatusCode sc = Gaudi::Algorithm::initialize();
    if (!sc.isSuccess())
      return sc;
  }

  if (!m_signalProvider) {
    error() << "Signal event provider is missing!" << endmsg;
    return StatusCode::FAILURE;
  }

  if (!m_vertexSmearingTool) {
    error() << "Vertex smearing tool is missing!" << endmsg;
    return StatusCode::FAILURE;
  }

  if (!m_pileUpTool) {
    error() << "Pileup tool is missing!" << endmsg;
    return StatusCode::FAILURE;
  }

  if (!m_pileUpProvider) {
    error() << "Pileup provider is missing!" << endmsg;
    return StatusCode::FAILURE;
  }

  if (!m_hepmcMergeTool) {
    error() << "Event merge tool is missing!" << endmsg;
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode GenAlg::execute(const EventContext&) const {
  // Create empty event
  HepMC3::GenEvent* theEvent = m_hepmcHandle.createAndPut();
  theEvent->set_units(HepMC3::Units::GEV, HepMC3::Units::MM);

  // Get the event from the signal provider
  {
    StatusCode sc = m_signalProvider->getNextEvent(*theEvent);
    if (!sc.isSuccess())
      return sc;
  }

  // Smear vertex
  {
    StatusCode sc = m_vertexSmearingTool->smearVertex(*theEvent);
    if (!sc.isSuccess())
      return sc;
  }

  // Get number of pileup events
  const unsigned int numPileUp = m_pileUpTool->numberOfPileUp();
  debug() << "Number of pileup events: " << numPileUp << endmsg;

  // Merge in pileup
  if (numPileUp > 0) {
    std::vector<HepMC3::GenEvent> eventVector;
    eventVector.reserve(numPileUp + 1);

    if (!m_pileUpProvider.empty()) {
      for (unsigned int i_pileUp = 0; i_pileUp < numPileUp; ++i_pileUp) {
        auto puEvt = HepMC3::GenEvent();
        StatusCode sc = m_pileUpProvider->getNextEvent(puEvt);
        if (!sc.isSuccess())
          return sc;

        m_vertexSmearingTool->smearVertex(puEvt).ignore();
        eventVector.push_back(std::move(puEvt));
      }
    }

    StatusCode sc = m_hepmcMergeTool->merge(*theEvent, eventVector);
    if (!sc.isSuccess())
      return sc;
  }

  debug() << "Event number: " << theEvent->event_number() << endmsg;
  debug() << "Number of particles in the event: " << theEvent->particles().size() << endmsg;
  debug() << "Number of vertices in the event: " << theEvent->vertices().size() << endmsg;

  return StatusCode::SUCCESS;
}

StatusCode GenAlg::finalize() { return Gaudi::Algorithm::finalize(); }
