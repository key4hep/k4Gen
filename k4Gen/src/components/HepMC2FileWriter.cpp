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
#include "HepMC2FileWriter.h"
#include "HepMC3/GenEvent.h"
#include "HepMC3/WriterAsciiHepMC2.h"

DECLARE_COMPONENT(HepMC2FileWriter)

HepMC2FileWriter::HepMC2FileWriter(const std::string& name, ISvcLocator* svcLoc) : Gaudi::Algorithm(name, svcLoc) {
  declareProperty("hepmc", m_hepmchandle, "The HepMC event to write to text file (input)");
}

StatusCode HepMC2FileWriter::initialize() {

  m_file = std::make_unique<HepMC3::WriterAsciiHepMC2>(m_filename.value());
  return Gaudi::Algorithm::initialize();
}

StatusCode HepMC2FileWriter::execute(const EventContext&) const {
  const HepMC3::GenEvent* theEvent = m_hepmchandle.get();
  m_file->write_event(*theEvent);
  return StatusCode::SUCCESS;
}

StatusCode HepMC2FileWriter::finalize() {
  m_file.reset();
  return Gaudi::Algorithm::finalize();
}
