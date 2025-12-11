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
#include "GenParticleFilter.h"

#include "edm4hep/MCParticleCollection.h"

DECLARE_COMPONENT(GenParticleFilter)

GenParticleFilter::GenParticleFilter(const std::string& name, ISvcLocator* svcLoc) : Gaudi::Algorithm(name, svcLoc) {
  declareProperty("GenParticles", m_iGenpHandle, "Generator Particles to filter (input)");
  declareProperty("GenParticlesFiltered", m_oGenpHandle, "Filtered Generator particles (output)");
}

StatusCode GenParticleFilter::initialize() { return Gaudi::Algorithm::initialize(); }

StatusCode GenParticleFilter::execute(const EventContext&) const {
  const auto inparticles = m_iGenpHandle.get();
  auto particles = m_oGenpHandle.createAndPut();
  bool accept = false;
  for (auto ptc : (*inparticles)) {
    accept = false;
    for (auto status : m_accept) {
      if (ptc.getGeneratorStatus() == status) {
        accept = true;
      }
    }
    if (accept) {
      auto outptc = ptc.clone();
      particles->push_back(outptc);
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode GenParticleFilter::finalize() { return Gaudi::Algorithm::finalize(); }
