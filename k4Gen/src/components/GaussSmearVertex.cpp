#include "GaussSmearVertex.h"

#include "GaudiKernel/PhysicalConstants.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "GaudiKernel/Vector4DTypes.h"

#include "HepMC3/GenEvent.h"
#include "HepMC3/GenParticle.h"
#include "HepMC3/GenVertex.h"

#include <random>

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

  m_uniqueIDSvc = service("UniqueIDGenSvc");
  if (!m_uniqueIDSvc) {
    error() << "Unable to locate the UniqueIDGenSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  info() << "Smearing of interaction point with normal distribution "
         << " in x, y and z " << endmsg;
  info() << " with " << m_xsig / Gaudi::Units::mm << " mm  standard deviation in x " << m_ysig / Gaudi::Units::mm
         << " mm in y and " << m_zsig / Gaudi::Units::mm << " mm in z." << endmsg;

  return sc;
}

/// Smearing function
StatusCode GaussSmearVertex::smearVertex(HepMC3::GenEvent& theEvent, const EventContext& ctx) {

  const size_t uid = m_uniqueIDSvc->getUniqueID(ctx.evt(), 0, name());
  std::mt19937_64 engine(uid);
  std::normal_distribution<double> gaussDist(0., 1.);

  double dx = gaussDist(engine) * m_xsig + m_xmean;
  double dy = gaussDist(engine) * m_ysig + m_ymean;
  double dz = gaussDist(engine) * m_zsig + m_zmean;
  // HepMC3 stores c*t as a length
  double dt = (gaussDist(engine) * m_tsig + m_tmean) * Gaudi::Units::c_light;

  Gaudi::LorentzVector dpos(dx, dy, dz, dt);

  debug() << "Smearing vertices by " << dpos << endmsg;

  // Shift all vertices and the event position (production vertex of the beam particles)
  theEvent.shift_position_by(HepMC3::FourVector(dpos.x(), dpos.y(), dpos.z(), dpos.t()));

  return StatusCode::SUCCESS;
}
