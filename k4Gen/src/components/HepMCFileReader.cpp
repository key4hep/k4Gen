
#include "HepMCFileReader.h"

#include "GaudiKernel/IEventProcessor.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/Incident.h"

#include "HepMC3/GenParticle.h"
#include "HepMC3/GenVertex.h"

DECLARE_COMPONENT(HepMCFileReader)

HepMCFileReader::HepMCFileReader(const std::string& type, const std::string& name, const IInterface* parent)
    : AlgTool(type, name, parent), m_file(nullptr) {
  declareInterface<IHepMCProviderTool>(this);
}

HepMCFileReader::~HepMCFileReader() { ; }

StatusCode HepMCFileReader::initialize() {
  if (m_filename.empty()) {
    error() << "Input file name is not specified!" << endmsg;
    return StatusCode::FAILURE;
  }
  // open file using HepMC routines
  m_file = std::make_unique<HepMC3::ReaderAscii>(m_filename.value());
  StatusCode sc = AlgTool::initialize();
  return sc;
}

StatusCode HepMCFileReader::getNextEvent(HepMC3::GenEvent& event) {
  if (!m_file->read_event(event)) {
    error() << "Premature end of file: Please set the number of events according to hepMC file." << endmsg;
    return StatusCode::FAILURE;
  }
  // read_event takes the units from the file, convert them to GeV and mm
  // set_units does not convert the event position (root vertex), so do it by hand
  auto eventPos = event.event_pos();
  HepMC3::Units::convert(eventPos, event.length_unit(), HepMC3::Units::MM);
  event.set_units(HepMC3::Units::GEV, HepMC3::Units::MM);
  if (!event.beams().empty()) {
    event.beams().front()->production_vertex()->set_position(eventPos);
  }
  return StatusCode::SUCCESS;
}

StatusCode HepMCFileReader::finalize() {
  m_file.reset();
  return AlgTool::finalize();
}
