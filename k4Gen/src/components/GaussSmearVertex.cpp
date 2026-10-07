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

  // Shift all vertices and the event position (production vertex of the beam particles)
  theEvent.shift_position_by(HepMC3::FourVector(dpos.x(), dpos.y(), dpos.z(), dpos.t()));

  return StatusCode::SUCCESS;
}
