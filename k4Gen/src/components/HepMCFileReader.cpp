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

#include "HepMCFileReader.h"

#include "GaudiKernel/IEventProcessor.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/Incident.h"

DECLARE_COMPONENT(HepMCFileReader)

HepMCFileReader::HepMCFileReader(const std::string& type, const std::string& name, const IInterface* parent)
    : AlgTool(type, name, parent), m_file(nullptr) {
  declareInterface<IHepMCProviderTool>(this);
}

HepMCFileReader::~HepMCFileReader() { ; }

StatusCode HepMCFileReader::initialize() {
  if (m_filename.empty()) {
    error() << "Input file name is not specified!" << endmsg;
    return StatusCode::FAILURE;
  }
  // open file using HepMC routines
  m_file = std::make_unique<HepMC3::ReaderAscii>(m_filename.value());
  StatusCode sc = AlgTool::initialize();
  return sc;
}

StatusCode HepMCFileReader::getNextEvent(HepMC3::GenEvent& event) {
  if (!m_file->read_event(event)) {
    error() << "Premature end of file: Please set the number of events according to hepMC file." << endmsg;
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode HepMCFileReader::finalize() {
  m_file.reset();
  return AlgTool::finalize();
}
