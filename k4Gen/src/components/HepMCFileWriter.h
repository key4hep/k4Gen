#ifndef GENERATION_HEPMCFILEWRITER_H
#define GENERATION_HEPMCFILEWRITER_H

#include "Gaudi/Algorithm.h"

#include "k4FWCore/DataHandle.h"

#include "HepMC3/Writer.h"

namespace HepMC3 {
class GenEvent;
} // namespace HepMC3

/**
 *
 * \brief Algorithm to write HepMC input to a text file.
 *
 * \attention
 * The HepMC format is text-based, fairly verbose and more suitable
 * for debugging than actual storage of physics result, which should be
 * done in the fccsw event data format.
 */

class HepMCFileWriter : public Gaudi::Algorithm {

public:
  /// Constructor.
  HepMCFileWriter(const std::string& name, ISvcLocator* svcLoc);
  /// Initialize.
  virtual StatusCode initialize();
  /// Execute.
  virtual StatusCode execute(const EventContext&) const;
  /// Finalize.
  virtual StatusCode finalize();

private:
  /// Handle for the HepMC to be read
  mutable k4FWCore::DataHandle<HepMC3::GenEvent> m_hepmchandle{"HepMC", Gaudi::DataHandle::Reader, this};
  Gaudi::Property<std::string> m_filename{this, "Filename", "gen_output.hepmc", "Name of the HepMC file to write"};
  Gaudi::Property<std::string> m_format{this, "Format", "Ascii", "Output format: Ascii or RootTree (ROOT TTree)"};
  std::unique_ptr<HepMC3::Writer> m_file;
};

#endif // GENERATION_HEPMCFILEWRITER_H
