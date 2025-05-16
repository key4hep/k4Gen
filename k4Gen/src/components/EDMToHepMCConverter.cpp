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
#include "EDMToHepMCConverter.h"

#include "GaudiKernel/PhysicalConstants.h"

#include "HepMC3/GenParticle.h"
#include "HepMC3/GenVertex.h"

#include "edm4hep/MCParticleCollection.h"

using HepMC3::GenParticle;

DECLARE_COMPONENT(EDMToHepMCConverter)

EDMToHepMCConverter::EDMToHepMCConverter(const std::string& name, ISvcLocator* svcLoc)
    : Gaudi::Algorithm(name, svcLoc) {
  declareProperty("hepmc", m_hepmchandle, "HepMC event handle (output)");
  declareProperty("GenParticles", m_genphandle, "Generated particles collection (input)");
}

StatusCode EDMToHepMCConverter::initialize() { return Gaudi::Algorithm::initialize(); }

StatusCode EDMToHepMCConverter::execute(const EventContext&) const {

  auto particles = m_genphandle.get();
  // ownership of event given to data service at the end of execute
  auto* event = new HepMC3::GenEvent;
  event->set_units(HepMC3::Units::GEV, HepMC3::Units::MM);

  for (auto p : *particles) {
    if (p.getGeneratorStatus() == 1) { // only final state particles
      const auto& mom = p.getMomentum();
      auto pHepMC = std::make_shared<GenParticle>(HepMC3::FourVector(mom.x, mom.y, mom.z, p.getMass()), p.getPDG(),
                                                  p.getGeneratorStatus()); // hepmc status code for final state particle

      const auto& pos = p.getVertex();
      auto v = std::make_shared<HepMC3::GenVertex>(
          HepMC3::FourVector(pos.x, pos.y, pos.z, p.getTime() / Gaudi::Units::c_light));

      v->add_particle_out(pHepMC);
      event->add_vertex(v);
    }
  }

  m_hepmchandle.put(event);
  return StatusCode::SUCCESS;
}

StatusCode EDMToHepMCConverter::finalize() { return Gaudi::Algorithm::finalize(); }
