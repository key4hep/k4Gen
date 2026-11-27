#ifndef GENERATION_HEPMCTOEDMCONVERTER_H
#define GENERATION_HEPMCTOEDMCONVERTER_H

// Gaudi
#include "Gaudi/Algorithm.h"
// k4FWCore
#include "k4FWCore/DataHandle.h"
// HepMC
#include "HepMC3/GenEvent.h"
#include "HepMC3/GenParticle.h"
// edm4hep
#include "edm4hep/Constants.h"

namespace HepMC3 {
class GenRunInfo;
} // namespace HepMC3

namespace edm4hep {
class EventHeaderCollection;
class MCParticleCollection;
class MutableMCParticle;
} // namespace edm4hep

class HepMCToEDMConverter : public Gaudi::Algorithm {

public:
  /// Constructor.
  HepMCToEDMConverter(const std::string& name, ISvcLocator* svcLoc);
  /// Execute.
  virtual StatusCode execute(const EventContext&) const;
  /// Finalize.
  virtual StatusCode finalize();

private:
  /// list of hepmc statuses that will be converted.
  /// If emtpy, all particles will be converted.
  Gaudi::Property<std::vector<unsigned int>> m_hepmcStatusList{
      this, "hepmcStatusList", {1}, "list of hepmc statuses to keep. An empty list means all statuses will be kept"};
  /// Handle for the HepMC to be read
  mutable k4FWCore::DataHandle<HepMC3::GenEvent> m_hepmchandle{"hepmc", Gaudi::DataHandle::Reader, this};
  /// Handle for the genparticles to be written
  mutable k4FWCore::DataHandle<edm4hep::MCParticleCollection> m_genphandle{edm4hep::labels::MCParticles,
                                                                           Gaudi::DataHandle::Writer, this};
  /// Handle for the event header to be written, only written if a path is set
  mutable k4FWCore::DataHandle<edm4hep::EventHeaderCollection> m_eventHeaderHandle{"", Gaudi::DataHandle::Writer, this};
  Gaudi::Property<uint32_t> m_runNumber{this, "runNumber", 0, "Run number written to the event header"};
  /// Run information of the events (weight names, generator tools), put into the metadata at finalize
  mutable std::shared_ptr<const HepMC3::GenRunInfo> m_runInfo;

  edm4hep::MutableMCParticle convert(std::shared_ptr<const HepMC3::GenParticle> hepmcParticle) const;
};
#endif
