#ifndef GENERATION_EDMTOHEPMCCONVERTER_H
#define GENERATION_EDMTOHEPMCCONVERTER_H

#include "Gaudi/Algorithm.h"
#include "HepMC3/GenEvent.h"
#include "HepMC3/Units.h"
#include "k4FWCore/DataHandle.h"
#include "edm4hep/Constants.h"

namespace edm4hep {
class MCParticleCollection;
}

class EDMToHepMCConverter : public Gaudi::Algorithm {

public:
  /// Constructor.
  EDMToHepMCConverter(const std::string& name, ISvcLocator* svcLoc);
  /// Initialize.
  virtual StatusCode initialize();
  /// Execute.
  virtual StatusCode execute(const EventContext&) const;
  /// Finalize.
  virtual StatusCode finalize();

private:
  /// Handle for the HepMC to be read
  mutable k4FWCore::DataHandle<HepMC3::GenEvent> m_hepmchandle{"hepmc", Gaudi::DataHandle::Writer, this};
  /// Handle for the genparticles to be written
  mutable k4FWCore::DataHandle<edm4hep::MCParticleCollection> m_genphandle{edm4hep::labels::MCParticles, Gaudi::DataHandle::Reader, this};
};

#endif
