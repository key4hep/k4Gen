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
#include "HepMCFullMerge.h"

#include "GaudiKernel/IEventProcessor.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/Incident.h"

#include "HepMC3/GenEvent.h"
#include "HepMC3/GenParticle.h"
#include "HepMC3/GenVertex.h"

DECLARE_COMPONENT(HepMCFullMerge)

HepMCFullMerge::HepMCFullMerge(const std::string& type, const std::string& name, const IInterface* parent)
    : AlgTool(type, name, parent) {
  declareInterface<IHepMCMergeTool>(this);
}

HepMCFullMerge::~HepMCFullMerge() {}

StatusCode HepMCFullMerge::initialize() {
  StatusCode sc = AlgTool::initialize();
  if (!sc.isSuccess())
    return sc;
  return sc;
}

StatusCode HepMCFullMerge::merge(HepMC3::GenEvent& signalEvent, const std::vector<HepMC3::GenEvent>& eventVector) {
  for (auto it = eventVector.cbegin(), end = eventVector.cend(); it != end; ++it) {
    // keep track of which vertex in full event corresponds to which vertex in merged event
    std::unordered_map<std::shared_ptr<const HepMC3::GenVertex>, std::shared_ptr<HepMC3::GenVertex>>
        inputToMergedVertexMap;
    for (auto& v : it->vertices()) {
      auto outvertex = std::make_shared<HepMC3::GenVertex>(v->position());
      inputToMergedVertexMap[v] = outvertex;
      signalEvent.add_vertex(outvertex);
    }
    for (auto& p : it->particles()) {
      // ownership of the particle is given to the vertex
      auto newparticle = std::make_shared<HepMC3::GenParticle>(*p);
      // attach particles to correct vertices in merged event
      if (p->end_vertex()) {
        inputToMergedVertexMap[p->end_vertex()]->add_particle_in(newparticle);
      }
      if (p->production_vertex()) {
        inputToMergedVertexMap[p->production_vertex()]->add_particle_out(newparticle);
      }
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode HepMCFullMerge::finalize() { return AlgTool::finalize(); }
