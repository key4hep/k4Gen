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
#ifndef GENERATION_IVERTEXSMEARINGTOOL_H
#define GENERATION_IVERTEXSMEARINGTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "HepMC3/GenEvent.h"

/** @class IVertexSmearingTool IVertexSmearingTool.h "Generation/IVertexSmearingTool.h"
 *
 *  Abstract interface to vertex smearing tools. Concrete implementations
 *  apply vertex smearing algorithms to each generated pile-up interactions.
 *
 *  @author Patrick Robbe
 *  @author Daniel Funke
 *  @date   2005-08-17
 */

class IVertexSmearingTool : virtual public IAlgTool {
public:
  DeclareInterfaceID(IVertexSmearingTool, 1, 0);

  /// Smear the vertex of the interaction (independantly of the others)
  virtual StatusCode smearVertex(HepMC3::GenEvent& theEvent) = 0;
};
#endif // GENERATION_ISMEARINGTOOL_H
