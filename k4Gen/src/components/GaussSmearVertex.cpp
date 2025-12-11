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
#include "GaussSmearVertex.h"

#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "GaudiKernel/Vector4DTypes.h"

#include "HepMC3/GenEvent.h"
#include "HepMC3/GenParticle.h"
#include "HepMC3/GenVertex.h"

/// Declaration of the Tool Factory
DECLARE_COMPONENT(GaussSmearVertex)

/// Standard constructor, initializes variables
GaussSmearVertex::GaussSmearVertex(const std::string& type, const std::string& name, const IInterface* parent)
    : AlgTool(type, name, parent) {
  declareInterface<IVertexSmearingTool>(this);
}

/// Destructor
GaussSmearVertex::~GaussSmearVertex() { ; }

//=============================================================================
// Initialize
//=============================================================================
StatusCode GaussSmearVertex::initialize() {
  StatusCode sc = AlgTool::initialize();
  if (sc.isFailure())
    return sc;

  auto randSvc = service<IRndmGenSvc>("RndmGenSvc", true);

  sc = m_gaussDist.initialize(randSvc, Rndm::Gauss(0., 1));
  if (sc.isFailure())
    return sc;

  info() << "Smearing of interaction point with normal distribution "
         << " in x, y and z " << endmsg;
  info() << " with " << m_xsig / Gaudi::Units::mm << " mm  standard deviation in x " << m_ysig / Gaudi::Units::mm
         << " mm in y and " << m_zsig / Gaudi::Units::mm << " mm in z." << endmsg;

  if (!sc.isSuccess()) {
    error() << "Could not initialize normal random number generator";
    return StatusCode::FAILURE;
  }

  return sc;
}

/// Smearing function
StatusCode GaussSmearVertex::smearVertex(HepMC3::GenEvent& theEvent) {

  double dx = m_gaussDist() * m_xsig + m_xmean;
  double dy = m_gaussDist() * m_ysig + m_ymean;
  double dz = m_gaussDist() * m_zsig + m_zmean;
  double dt = m_gaussDist() * m_tsig + m_tmean;

  Gaudi::LorentzVector dpos(dx, dy, dz, dt);

  debug() << "Smearing vertices by " << dpos << endmsg;

  for (auto vit : theEvent.vertices()) {
    Gaudi::LorentzVector pos(vit->position());
    pos += dpos;
    vit->set_position(HepMC3::FourVector(pos.x(), pos.y(), pos.z(), pos.t()));
  }

  return StatusCode::SUCCESS;
}
