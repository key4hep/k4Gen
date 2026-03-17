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

#ifndef GENERATION_HEPMC2FILEREADER_H
#define GENERATION_HEPMC2FILEREADER_H

#include "GaudiKernel/AlgTool.h"
#include "Generation/IHepMCProviderTool.h"

#include "HepMC3/GenEvent.h"
#include "HepMC3/ReaderAsciiHepMC2.h"

class HepMC2FileReader : public AlgTool, virtual public IHepMCProviderTool {
public:
  HepMC2FileReader(const std::string& type, const std::string& name, const IInterface* parent);
  virtual ~HepMC2FileReader(); ///< Destructor
  virtual StatusCode initialize();
  virtual StatusCode finalize();

  /// Wrapper for HepMC's fill_next_event() --
  ///  as in the hepmc original, the user is responsible
  ///  for the deletion of the event returned from this function
  virtual StatusCode getNextEvent(HepMC3::GenEvent& event);
  /// Wrapper for HepMC file io.

private:
  void close();
  Gaudi::Property<std::string> m_filename{this, "Filename", "", "Name of the HepMC file to read"};
  std::unique_ptr<HepMC3::ReaderAsciiHepMC2> m_file;
};

#endif // GENERATION_HEPMC2FILEREADER_H
