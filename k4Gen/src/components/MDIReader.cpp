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

#include "MDIReader.h"

#include "GaudiKernel/IEventProcessor.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/Incident.h"

#include "edm4hep/MCParticleCollection.h"

#include <algorithm>
#include <iostream>
#include <math.h>

DECLARE_COMPONENT(MDIReader)

MDIReader::MDIReader(const std::string& name, ISvcLocator* svcLoc) : Gaudi::Algorithm(name, svcLoc), m_filename() {

  declareProperty("MDIFilename", m_filename = "", "Name of the MDI file to read");
  declareProperty("GenParticles", m_genphandle, "Generated particles collection (output)");
  declareProperty("CrossingAngle", xing, "Half the crossing angle beam in [rad]");
  declareProperty("LongitudinalCut", cut_z, "the value for cut_z used in GP++ in [um]");
  declareProperty("InputType", input_type, "string: guineapig, xtrack");
  declareProperty("BeamEnergy", beam_energy, "beam energy [GeV], necessary for xtrack type");
}

StatusCode MDIReader::initialize() {
  StatusCode sc = Gaudi::Algorithm::initialize();

  debug() << "Reading file: " << m_filename << endmsg;

  m_input.open(m_filename.c_str(), std::ifstream::in);

  if (!m_input.good()) {
    error() << "Failed to open input stream:" + m_filename << endmsg;
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode MDIReader::execute(const EventContext&) const {
  // First check the input file status
  if (m_input.eof()) {
    error() << "End of file reached" << endmsg;
    return StatusCode::FAILURE;
  }

  // Check the input type flag
  if (input_type != "guineapig" && input_type != "xtrack") {
    error() << "Input type flag - wrong definition: " << input_type << endmsg;
    return StatusCode::FAILURE;
  } else {
    debug() << "Selected input type : " << input_type << endmsg;
  }

  //  Loop over particles
  int ISTHEP = 1; // status code
  int IDHEP = 0;  // PDG code
  int CHARGE = 0; // charge
  // int JMOHEP1;  // first mother
  // int JMOHEP2;  // last mother
  // int JDAHEP1;  // first daughter
  // int JDAHEP2;  // last daughter
  double PHEP1 = 0.0; // px in GeV/c
  double PHEP2 = 0.0; // py in GeV/c
  double PHEP3 = 0.0; // pz in GeV/c
  double PHEP4;       // energy in GeV
  double PHEP5;       // mass in GeV/c**2
  double VHEP1 = 0.0; // x vertex position in mm
  double VHEP2 = 0.0; // y vertex position in mm
  double VHEP3 = 0.0; // z vertex position in mm
  double VHEP4 = 0;   // production time in mm/c

  // guineapig IPP variables which do not end up in the EDM format
  double process; // 1=Breit-Wheeler 2=Bethe-Heitler 3=Landau-Lifshitz
  double trash;   // trash variable
  double id_ee;   // same id means they are a pair
  double temp_x, temp_z, temp_px, temp_pz, temp_e;

  debug() << "The crossing angle is " << xing << " [rad]" << endmsg;
  // std::cout <<"The crossing angle is "<<xing<<" [rad]"<< endmsg;
  edm4hep::MCParticleCollection* particles = new edm4hep::MCParticleCollection();

  size_t pcount = 0;
  PHEP5 = 5.11e-4;
  while (m_input.good()) {
    if (input_type == "guineapig") {
      m_input >> PHEP4 >> PHEP1 >> PHEP2 >> PHEP3 >> VHEP1 >> VHEP2 >> VHEP3 >> process >> trash >> id_ee;

      // std::cout<<PHEP4<<" "<<sqrt(PHEP1*PHEP1 + PHEP2*PHEP2 + PHEP3*PHEP3)<<" "<<sqrt((PHEP1*PHEP1 + PHEP2*PHEP2 +
      // PHEP3*PHEP3)*PHEP4*PHEP4 + PHEP5*PHEP5)<<std::endl;

      if (m_input.eof())
        break;
      else if (!m_input.good()) {
        debug() << "End of file reached before reading all the hits" << endmsg;
        error() << "End of file reached before reading all the hits" << endmsg;
        return StatusCode::FAILURE;
      }

      IDHEP = 11;
      CHARGE = -1;
      if (PHEP4 < 0) {
        IDHEP = -11;
        CHARGE = 1;
      }

      VHEP1 *= 1e-9; // convert from nm to m
      VHEP2 *= 1e-9; // convert from nm to m
      VHEP3 *= 1e-9; // convert from nm to m

      //---boost section
      PHEP4 = abs(PHEP4);
      temp_x = cut_z * 1e-6 * tan(xing) + VHEP1 * sqrt(1 + pow(tan(xing), 2));
      temp_px = PHEP4 * tan(xing) + PHEP1 * PHEP4 * sqrt(1 + pow(tan(xing), 2));
      temp_e = abs(PHEP4 * sqrt(1 + pow(tan(xing), 2)) + PHEP1 * PHEP4 * tan(xing));

      VHEP1 = temp_x;
      PHEP1 = temp_px;
      PHEP2 = PHEP2 * PHEP4;
      PHEP3 = PHEP3 * PHEP4;
      PHEP4 = temp_e;

      //---end boost section

      VHEP1 *= 1e3; // convert from m to mm
      VHEP2 *= 1e3; // convert from m to mm
      VHEP3 *= 1e3; // convert from m to mm

      // boost to lab frame

      VHEP1 = 0;
      VHEP2 = 0;
      VHEP3 = 0;
    } // end if guineapig

    else if (input_type == "xtrack") {
      m_input >> VHEP3 >> VHEP1 >> VHEP2 >> PHEP1 >> PHEP2 >> temp_z >> PHEP4;

      if (m_input.eof())
        break;
      else if (!m_input.good()) {
        debug() << "End of file reached before reading all the hits" << endmsg;
        error() << "End of file reached before reading all the hits" << endmsg;
        return StatusCode::FAILURE;
      }

      // VHEP3 = VHEP3 + temp_z; //is ct necessary? prob not

      //---position rotation in CLD frame
      temp_z = VHEP3 * cos(-xing) + VHEP1 * sin(-xing);
      temp_x = -VHEP3 * sin(-xing) + VHEP1 * cos(-xing);
      VHEP1 = temp_x;
      VHEP3 = temp_z;

      VHEP1 *= 1e3; // convert from m to mm
      VHEP2 *= 1e3; // convert from m to mm
      VHEP3 *= 1e3; // convert from m to mm

      PHEP4 = (1. + PHEP4) * beam_energy;
      PHEP1 = PHEP1 * beam_energy;
      PHEP2 = PHEP2 * beam_energy;
      PHEP3 = sqrt(pow(PHEP4, 2) - pow(PHEP1, 2) - pow(PHEP2, 2));

      //---momentum rotation in CLD frame
      temp_pz = PHEP3 * cos(-xing) + PHEP1 * sin(-xing);
      temp_px = -PHEP3 * sin(-xing) + PHEP1 * cos(-xing);
      PHEP3 = temp_pz;
      PHEP1 = temp_px;

      IDHEP = 11;
      CHARGE = -1;

    } // end if xtrack

    edm4hep::MutableMCParticle particle = particles->create();

    particle.setPDG(IDHEP);
    particle.setCharge(CHARGE);
    particle.setGeneratorStatus(ISTHEP);
    particle.setMomentum({
        (float)(PHEP1),
        (float)(PHEP2),
        (float)(PHEP3),
    });
    particle.setMass(PHEP5);
    particle.setVertex({
        VHEP1,
        VHEP2,
        VHEP3,
    });
    particle.setTime(VHEP4);
    pcount++;

    debug() << "Read in particle (" << pcount << "):" << endmsg;
    debug() << "  - PDG ID: " << particle.getPDG() << endmsg;
    debug() << "  - Generator status: " << particle.getGeneratorStatus() << endmsg;
    debug() << "  - Charge: " << particle.getCharge() << endmsg;
    debug() << "  - Energy: " << particle.getEnergy() << " GeV" << endmsg;
    debug() << "  - Mass: " << particle.getMass() << " GeV/c**2" << endmsg;
    debug() << "  - Momentum: x = " << particle.getMomentum().x << ", y = " << particle.getMomentum().y
            << ", z = " << particle.getMomentum().z << " GeV/c" << endmsg;
    debug() << "  - Vertex: x = " << particle.getVertex().x << ", y = " << particle.getVertex().y
            << ", z = " << particle.getVertex().z << " mm" << endmsg;
  }

  m_genphandle.put(particles);
  return StatusCode::SUCCESS;
}

StatusCode MDIReader::finalize() {
  m_input.close();
  debug() << "MDIReader finalization" << endmsg;
  return Gaudi::Algorithm::finalize();
}
