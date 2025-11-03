#include "GenEventFilter.h"

// Gaudi
#include "GaudiKernel/IEventProcessor.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/StatusCode.h"

// Datamodel
#include "edm4hep/MCParticleCollection.h"

// ROOT
#include "TGlobal.h"
#include "TInterpreter.h"
#include "TROOT.h"
#include "TSystem.h"

GenEventFilter::GenEventFilter(const std::string& name, ISvcLocator* svcLoc) : Gaudi::Algorithm(name, svcLoc) {
  declareProperty("particles", m_inColl, "Generated particles to decide on (input)");
}

StatusCode GenEventFilter::initialize() {
  {
    StatusCode sc = Gaudi::Algorithm::initialize();
    if (sc.isFailure()) {
      return sc;
    }
  }

  m_property = service("ApplicationMgr", m_property);
  Gaudi::Property<int> evtMax;
  evtMax.assign(m_property->getProperty("EvtMax"));
  m_nEventsTarget = evtMax;
  debug() << "Targeted number of events: " << m_nEventsTarget << endmsg;
  m_nEventsAccepted = 0;
  m_nEventsSeen = 0;

  m_incidentSvc = service("IncidentSvc");

  m_eventProcessor = service("ApplicationMgr");

  if (m_filterRuleStr.value().empty() && m_filterRulePath.value().empty()) {
    error() << "Filter rule not found!" << endmsg;
    error() << "Provide it as a string or in the cxx file." << endmsg;
    return StatusCode::FAILURE;
  }

  if (!m_filterRuleStr.value().empty() && !m_filterRulePath.value().empty()) {
    error() << "Multiple ilter rules found!" << endmsg;
    error() << "Provide either a string or the cxx file." << endmsg;
    return StatusCode::FAILURE;
  }

  if (!m_filterRuleStr.value().empty()) {
    // Include(s) needed
    {
      bool success = gInterpreter->Declare("#include \"edm4hep/MCParticleCollection.h\"");
      if (!success) {
        error() << "Unable to find edm4hep::MCParticleCollection header file!" << endmsg;
        return StatusCode::FAILURE;
      }
      debug() << "Found edm4hep::MCParticleCollection header file." << endmsg;
    }

    // Filter rule provided directly as a string
    {
      bool success = gInterpreter->Declare(m_filterRuleStr.value().c_str());
      if (!success) {
        error() << "Unable to compile filter rule!" << endmsg;
        return StatusCode::FAILURE;
      }
      debug() << "Filter rule compiled successfully." << endmsg;
    }
  }

  if (!m_filterRulePath.value().empty()) {
    if (gSystem->AccessPathName(m_filterRulePath.value().c_str())) {
      error() << "Unable to access filter rule file!" << endmsg;
      error() << "Provided filter rule file path: " << m_filterRulePath.value() << endmsg;
      return StatusCode::FAILURE;
    }
    // Include and compile the file
    {
      bool success = gInterpreter->Declare(("#include \"" + m_filterRulePath + "\"").c_str());
      if (!success) {
        error() << "Unable to include filter rule file!" << endmsg;
        return StatusCode::FAILURE;
      }
      debug() << "Included filter rule file." << endmsg;
    }
  }

  // Get the address of the compiled filter rule from the interpreter
  {
    enum TInterpreter::EErrorCode err = TInterpreter::kProcessing;
    m_filterRulePtr = reinterpret_cast<bool (*)(const edm4hep::MCParticleCollection*)>(
        gInterpreter->ProcessLineSynch("&filterRule", &err));
    if (err != TInterpreter::kNoError) {
      error() << "Unable to obtain filter rule pointer!" << endmsg;
      return StatusCode::FAILURE;
    }
    debug() << "Filter rule pointer obtained successfully." << endmsg;
  }

  // Check if the filter rule pointer has correct signature
  {
    auto success = gInterpreter->Declare("auto filterRulePtr = &filterRule;");
    if (!success) {
      error() << "Unable to declare filter rule pointer in the interpreter!" << endmsg;
      return StatusCode::FAILURE;
    }
    auto global = gROOT->GetGlobal("filterRulePtr");
    if (!global) {
      error() << "Unable to obtain filter rule pointer from the interpreter!" << endmsg;
      return StatusCode::FAILURE;
    }
    std::string filterRuleType = global->GetTypeName();
    if (filterRuleType != "bool(*)(const edm4hep::MCParticleCollection*)") {
      error() << "Found filter rule pointer has wrong signature!" << endmsg;
      error() << "Required: bool(*)(const edm4hep::MCParticleCollection*)" << endmsg;
      error() << "Found:    " << filterRuleType << endmsg;
      return StatusCode::FAILURE;
    }
    debug() << "Found filter rule pointer has correct signature." << endmsg;
  }

  return StatusCode::SUCCESS;
}

StatusCode GenEventFilter::execute(const EventContext&) const {
  const edm4hep::MCParticleCollection* inParticles = m_inColl.get();

  if (m_nEventsSeen == m_nEventsSeenMax) {
    debug() << "Maximum number of " << m_nEventsSeenMax.value()
            << " seen events reached!" << endmsg;

    StatusCode sc = m_eventProcessor->stopRun();
    if (sc.isFailure()) {
      error() << "Error when attempting to stop event processing! Aborting..."
              << endmsg;
      return sc;
    }

    m_incidentSvc->fireIncident(Incident(name(), IncidentType::AbortEvent));

    return StatusCode::SUCCESS;
  }

  m_nEventsSeen++;

  if (!(*m_filterRulePtr)(inParticles)) {
    debug() << "Skipping event..." << endmsg;

    {
      StatusCode sc = m_eventProcessor->nextEvent(1);
      if (sc.isFailure()) {
        error() << "Error when attempting to skip the event! Aborting..." << endmsg;
        return sc;
      }
    }

    m_incidentSvc->fireIncident(Incident(name(), IncidentType::AbortEvent));

    return StatusCode::SUCCESS;
  }

  m_nEventsAccepted++;

  debug() << "Event contains " << inParticles->size() << " particles." << endmsg;
  debug() << "Targeted number of events to generate: " << m_nEventsTarget << endmsg;
  debug() << "Number of events already generated: " << m_nEventsAccepted << endmsg;
  debug() << "Remaining number of event to generate: " << m_nEventsTarget - m_nEventsAccepted << endmsg;
  debug() << "Number of events seen so far: " << m_nEventsSeen << endmsg;

  return StatusCode::SUCCESS;
}

StatusCode GenEventFilter::finalize() {
  debug() << "Number of events seen: " << m_nEventsSeen << endmsg;

  m_evtFilterStats.put({m_nEventsSeen, m_nEventsAccepted, m_nEventsTarget});

  return Gaudi::Algorithm::finalize();
}

DECLARE_COMPONENT(GenEventFilter)
