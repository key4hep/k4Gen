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

  // Shift all vertices and the event position (production vertex of the beam particles)
  theEvent.shift_position_by(HepMC3::FourVector(dpos.x(), dpos.y(), dpos.z(), dpos.t()));

  return StatusCode::SUCCESS;
}
