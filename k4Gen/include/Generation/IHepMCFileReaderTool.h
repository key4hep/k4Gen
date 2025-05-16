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
#ifndef GENERATION_IHEPMCFILEREADERTOOL_H
#define GENERATION_IHEPMCFILEREADERTOOL_H

#include "GaudiKernel/IAlgTool.h"

namespace HepMC3 {
class GenEvent;
}

/** @class IHepMCFileReaderTool IHepMCFileReaderTool.h "Generation/IHepMCFileReaderTool.h"
 *
 *  Abstract interface to read hepmc files.
 *
 */

class IHepMCFileReaderTool : virtual public IAlgTool {
public:
  DeclareInterfaceID(IHepMCFileReaderTool, 3, 0);

  virtual StatusCode readNextEvent(HepMC3::GenEvent&) = 0;

  virtual StatusCode open(const std::string& filename) = 0;
};

#endif // GENERATION_IHEPMCFILEREADERTOOL_H
