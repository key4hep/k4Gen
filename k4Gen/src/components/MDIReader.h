/*
 * Copyright (c) 2020-2024 Key4hep-Project.
 *
 * This file is part of Key4hep.
 * See https://key4hep.github.io/key4hep-doc/ for further info.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef GENERATION_MDIREADER_H
#define GENERATION_MDIREADER_H

#include "Generation/IHepMCFileReaderTool.h"
#include "Generation/IHepMCMergeTool.h"
#include "Generation/IVertexSmearingTool.h"

#include "k4FWCore/DataHandle.h"

#include "Gaudi/Algorithm.h"
#include "GaudiKernel/ToolHandle.h"

#include "GaudiKernel/SystemOfUnits.h"

#include "HepMC3/GenEvent.h"
#include "HepMC3/ReaderAscii.h"

namespace edm4hep {
class MCParticleCollection;
}

/**@class MDIReader MDIReader.h MDIReader.h
 *
 *  This algorithm reads in events from MDI generated file and puts them into the
 *  transient event store.
 *
 *  @author  aciarma
 *  @version 1.0
 */

class MDIReader : public Gaudi::Algorithm {

public:
  /// Constructor.
  MDIReader(const std::string& name, ISvcLocator* svcLoc);
  /// Initialize.
  virtual StatusCode initialize();
  /// Execute.
  virtual StatusCode execute(const EventContext&) const;
  /// Finalize.
  virtual StatusCode finalize();

private:
  std::string m_filename;
  mutable std::ifstream m_input;
  int NHEP;
  int m_format;
  std::string input_type;
  double xing, cut_z, beam_energy;
  /// Handle for the genparticles to be written
  mutable k4FWCore::DataHandle<edm4hep::MCParticleCollection> m_genphandle{"GenParticles", Gaudi::DataHandle::Writer, this};

  /// Tools to handle input from HepMC-file
  ToolHandle<IHepMCFileReaderTool> m_signalFileReader;
  ToolHandle<IHepMCFileReaderTool> m_pileupFileReader;

  /// Tool to merge HepMC events
  ToolHandle<IHepMCMergeTool> m_HepMCMergeTool;
  // Tool to smear vertices
  ToolHandle<IVertexSmearingTool> m_vertexSmearingTool;
};

#endif // GENERATION_MDIREADER_H
