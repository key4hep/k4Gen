#ifndef GENERATION_HEPMCTOEDMCONVERTER_H
#define GENERATION_HEPMCTOEDMCONVERTER_H

// Gaudi
#include "Gaudi/Algorithm.h"
// k4FWCore
#include "k4FWCore/DataHandle.h"
// HepMC
#include "HepMC3/GenEvent.h"
#include "HepMC3/GenParticle.h"

namespace edm4hep {
class MCParticleCollection;
class MutableMCParticle;
} // namespace edm4hep

class HepMCToEDMConverter : public Gaudi::Algorithm {

public:
  /// Constructor.
  HepMCToEDMConverter(const std::string& name, ISvcLocator* svcLoc);
  /// Initialize.
  virtual StatusCode initialize();
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
  mutable k4FWCore::DataHandle<edm4hep::MCParticleCollection> m_genphandle{"MCParticles", Gaudi::DataHandle::Writer, this};

  edm4hep::MutableMCParticle convert(std::shared_ptr<const HepMC3::GenParticle> hepmcParticle) const;
};
#endif
