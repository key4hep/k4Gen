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
#include "FlatSmearVertex.h"

#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "GaudiKernel/Vector4DTypes.h"

#include "HepMC3/GenEvent.h"
#include "HepMC3/GenParticle.h"
#include "HepMC3/GenVertex.h"

/// Declaration of the Tool Factory
DECLARE_COMPONENT(FlatSmearVertex)

/// Standard constructor, initializes variables
FlatSmearVertex::FlatSmearVertex(const std::string& type, const std::string& name, const IInterface* parent)
    : AlgTool(type, name, parent) {
  declareInterface<IVertexSmearingTool>(this);
}

/// Destructor
FlatSmearVertex::~FlatSmearVertex() { ; }

//=============================================================================
// Initialize
//=============================================================================
StatusCode FlatSmearVertex::initialize() {
  StatusCode sc = AlgTool::initialize();
  if (sc.isFailure())
    return sc;

  auto randSvc = service<IRndmGenSvc>("RndmGenSvc", true);
  if (m_xmin > m_xmax) {
    error() << "xMin > xMax !";
    return StatusCode::FAILURE;
  }
  if (m_ymin > m_ymax) {
    error() << "yMin > yMax !";
    return StatusCode::FAILURE;
  }
  if (m_zmin > m_zmax) {
    error() << "zMin > zMax !";
    return StatusCode::FAILURE;
  }

  sc = m_flatDist.initialize(randSvc, Rndm::Flat(0., 1.));

  std::string infoMsg = " applying TOF of interaction with ";
  if (m_zDir == -1) {
    infoMsg = infoMsg + "negative beam direction";
  } else if (m_zDir == 1) {
    infoMsg = infoMsg + "positive beam direction";
  } else if (m_zDir == 0) {
    infoMsg = " with TOF of interaction equal to zero ";
  } else {
    error() << "BeamDirection can only be set to -1 or 1, or 0 to switch off TOF";
    return StatusCode::FAILURE;
  }

  info() << "Smearing of interaction point with flat distribution "
         << " in x, y and z " << endmsg;
  info() << infoMsg << endmsg;
  info() << " with " << m_xmin / Gaudi::Units::mm << " mm <= x <= " << m_xmax / Gaudi::Units::mm << " mm, "
         << m_ymin / Gaudi::Units::mm << " mm <= y <= " << m_ymax / Gaudi::Units::mm << " mm and "
         << m_zmin / Gaudi::Units::mm << " mm <= z <= " << m_zmax / Gaudi::Units::mm << " mm." << endmsg;

  if (!sc.isSuccess()) {
    error() << "Could not initialize flat random number generator";
    return StatusCode::FAILURE;
  }

  return sc;
}

/// Smearing function
StatusCode FlatSmearVertex::smearVertex(HepMC3::GenEvent& theEvent) {
  double dx, dy, dz, dt;

  dx = m_xmin + m_flatDist() * (m_xmax - m_xmin);
  dy = m_ymin + m_flatDist() * (m_ymax - m_ymin);
  dz = m_zmin + m_flatDist() * (m_zmax - m_zmin);
  dt = m_zDir * dz / Gaudi::Units::c_light;

  Gaudi::LorentzVector dpos(dx, dy, dz, dt);

  debug() << "Smearing vertices by " << dpos << endmsg;

  for (auto vit : theEvent.vertices()) {
    Gaudi::LorentzVector pos(vit->position());
    pos += dpos;
    vit->set_position(HepMC3::FourVector(pos.x(), pos.y(), pos.z(), pos.t()));
  }

  return StatusCode::SUCCESS;
}
