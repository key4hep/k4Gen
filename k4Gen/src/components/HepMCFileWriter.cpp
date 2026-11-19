#include "HepMCFileWriter.h"
#include "HepMC3/GenEvent.h"
#include "HepMC3/WriterAscii.h"
#include "HepMC3/WriterGZ.h"
#include "HepMC3/WriterRootTree.h"

#include <exception>

DECLARE_COMPONENT(HepMCFileWriter)

namespace {
// Appends suffix to filename if not already present, logging the change.
std::string ensureSuffix(const std::string& filename, const std::string& suffix, MsgStream& log) {
  if (filename.size() >= suffix.size() &&
      filename.compare(filename.size() - suffix.size(), suffix.size(), suffix) == 0) {
    return filename;
  }
  log << MSG::INFO << "Appending " << suffix << " to output filename " << filename << endmsg;
  return filename + suffix;
}
} // namespace

HepMCFileWriter::HepMCFileWriter(const std::string& name, ISvcLocator* svcLoc) : Gaudi::Algorithm(name, svcLoc) {
  declareProperty("hepmc", m_hepmchandle, "The HepMC event to write to text file (input)");
}

StatusCode HepMCFileWriter::initialize() {
  const std::string& format = m_format.value();
  try {
    if (format == "Ascii") {
      m_file = std::make_unique<HepMC3::WriterAscii>(m_filename.value());
    } else if (format == "Compressed") {
      m_filename = ensureSuffix(m_filename.value(), ".gz", msgStream());
      m_file = std::make_unique<HepMC3::WriterGZ<HepMC3::WriterAscii>>(m_filename.value());
    } else if (format == "RootTree") {
      m_filename = ensureSuffix(m_filename.value(), ".root", msgStream());
      m_file = std::make_unique<HepMC3::WriterRootTree>(m_filename.value());
    } else {
      error() << "Unknown Format '" << format << "', expected Ascii, Compressed, or RootTree" << endmsg;
      return StatusCode::FAILURE;
    }
  } catch (const std::exception& e) {
    error() << "Failed to create writer for Format '" << format << "': " << e.what() << endmsg;
    return StatusCode::FAILURE;
  }
  return Gaudi::Algorithm::initialize();
}

StatusCode HepMCFileWriter::execute(const EventContext&) const {
  const HepMC3::GenEvent* theEvent = m_hepmchandle.get();
  m_file->write_event(*theEvent);
  return StatusCode::SUCCESS;
}

StatusCode HepMCFileWriter::finalize() {
  m_file.reset();
  return Gaudi::Algorithm::finalize();
}
