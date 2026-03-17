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
#ifndef GENERATION_IPILEUPTOOL_H
#define GENERATION_IPILEUPTOOL_H

#include "GaudiKernel/IAlgTool.h"

/** @class IPileUpTool IPileUpTool.h "Generation/IPileUpTool.h"
 *
 *  Abstract interface to pile up tools. Generates the number of pile-up
 *  interactions to generate for each event.
 *
 *  @author Patrick Robbe
 *  @date   2005-08-17
 */

static const InterfaceID IID_IPileUpTool("IPileUpTool", 3, 0);

class IPileUpTool : virtual public IAlgTool {
public:
  static const InterfaceID& interfaceID() { return IID_IPileUpTool; }

  /** Computes the number of pile-up interactions in the event.
   *  @param[out] currentLuminosity  Luminosity of the current event.
   *  @return Number of pile-up interactions to generate.
   */
  virtual unsigned int numberOfPileUp() = 0;

  virtual double getMeanPileUp() = 0;

  /// Print various counters at the end of the job
  virtual void printPileUpCounters() = 0;
};

#endif // GENERATION_IPILEUPTOOL_H
