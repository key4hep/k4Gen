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
#ifndef GENERATION_IPARTICLEGUNTOOL_H
#define GENERATION_IPARTICLEGUNTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/Vector4DTypes.h"
#include "IHepMCProviderTool.h"

/** @class IParticleGunTool IParticleGunTool.h "Generation/IParticleGunTool.h"
 *
 *  Abstract interface to particle gun tool. Generates a single particle.
 *
 *  @author Patrick Robbe
 *  @date   2008-05-18
 */

class IParticleGunTool : virtual public IHepMCProviderTool {
public:
  DeclareInterfaceID(IParticleGunTool, 1, 0);
  typedef std::vector<int> PIDs;

  /** Generates one particle.
   *  @param[out] fourMomentum  four-momentum of the generated particle gun
   *  @param[out] origin        four-momentum of the origin vertex of the particle gun
   *  @param[out] pdgId         pdgId of the generated particle
   */
  virtual void generateParticle(Gaudi::LorentzVector& fourMomentum, Gaudi::LorentzVector& origin, int& pdgId) = 0;

  /// Print various counters at the end of the job
  virtual void printCounters() = 0;
};

#endif
