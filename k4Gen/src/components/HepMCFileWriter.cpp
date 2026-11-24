#include "HepMCFileWriter.h"
#include "HepMC3/GenEvent.h"
#include "HepMC3/WriterAscii.h"
#include "HepMC3/WriterRootTree.h"

#include <algorithm>
#include <exception>
#include <string_view>
#include <vector>

DECLARE_COMPONENT(HepMCFileWriter)

namespace {
// Returns true if filename ends with one of the suffixes.
bool hasSuffix(std::string_view filename, const std::vector<std::string_view>& suffixes) {
  return std::ranges::any_of(suffixes, [filename](std::string_view suffix) { return filename.ends_with(suffix); });
}
} // namespace

HepMCFileWriter::HepMCFileWriter(const std::string& name, ISvcLocator* svcLoc) : Gaudi::Algorithm(name, svcLoc) {
  declareProperty("hepmc", m_hepmchandle, "The HepMC event to write to text file (input)");
}

StatusCode HepMCFileWriter::initialize() {
  // Conventional suffixes for the HepMC3 formats
  std::vector<std::string_view> expectedSuffixes;
  if (m_format == "Ascii") {
    expectedSuffixes = {".hepmc", ".hepmc3"};
  } else if (m_format == "RootTree") {
    expectedSuffixes = {".hepmc3.tree.root"};
  } else {
    error() << "Unknown Format '" << m_format.value() << "', expected Ascii or RootTree" << endmsg;
    return StatusCode::FAILURE;
  }
  if (!hasSuffix(m_filename.value(), expectedSuffixes)) {
    std::string suffixes;
    for (const auto suffix : expectedSuffixes) {
      suffixes += (suffixes.empty() ? "" : " or ") + std::string(suffix);
    }
    warning() << "Filename '" << m_filename.value() << "' does not end with " << suffixes
              << ", other tools might not recognise it as a HepMC3 " << m_format.value() << " file" << endmsg;
  }

  try {
    if (m_format == "Ascii") {
      m_file = std::make_unique<HepMC3::WriterAscii>(m_filename.value());
    } else {
      m_file = std::make_unique<HepMC3::WriterRootTree>(m_filename.value());
    }
  } catch (const std::exception& e) {
    error() << "Failed to create writer for Format '" << m_format.value() << "': " << e.what() << endmsg;
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
  if (m_file) {
    m_file->close();
  }
  m_file.reset();
  return Gaudi::Algorithm::finalize();
}
