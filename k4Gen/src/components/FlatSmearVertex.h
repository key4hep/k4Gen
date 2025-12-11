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
#ifndef GENERATION_FLATSMEARVERTEX_H
#define GENERATION_FLATSMEARVERTEX_H

#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/RndmGenerators.h"
#include "GaudiKernel/SystemOfUnits.h"

#include "Generation/IVertexSmearingTool.h"

#include "HepMC3/GenEvent.h"

/** @class FlatSmearVertex FlatSmearVertex.h "FlatSmearVertex.h"
 *
 *  Tool to smear vertex with flat smearing along the x- y- and z-axis.
 *  Concrete implementation of a IVertexSmearingTool.
 *
 *  @author Patrick Robbe
 *  @author Daniel Funke
 *  @date   2008-05-18
 */
class FlatSmearVertex : public AlgTool, virtual public IVertexSmearingTool {
public:
  /// Standard constructor
  FlatSmearVertex(const std::string& type, const std::string& name, const IInterface* parent);

  virtual ~FlatSmearVertex(); ///< Destructor

  /// Initialize method
  virtual StatusCode initialize();

  /** Implements IVertexSmearingTool::smearVertex.
   */
  virtual StatusCode smearVertex(HepMC3::GenEvent& theEvent);

private:
  /// Minimum value for the x coordinate of the vertex (set by options)
  Gaudi::Property<double> m_xmin{this, "xVertexMin", 0.0 * Gaudi::Units::mm, "Min value for x coordinate"};

  /// Minimum value for the y coordinate of the vertex (set by options)
  Gaudi::Property<double> m_ymin{this, "yVertexMin", 0.0 * Gaudi::Units::mm, "Min value for y coordinate"};

  /// Minimum value for the z coordinate of the vertex (set by options)
  Gaudi::Property<double> m_zmin{this, "zVertexMin", 0.0 * Gaudi::Units::mm, "Min value for z coordinate"};

  /// Maximum value for the x coordinate of the vertex (set by options)
  Gaudi::Property<double> m_xmax{this, "xVertexMax", 0.0 * Gaudi::Units::mm, "Max value for x coordinate"};

  /// Maximum value for the y coordinate of the vertex (set by options)
  Gaudi::Property<double> m_ymax{this, "yVertexMax", 0.0 * Gaudi::Units::mm, "Max value for y coordinate"};

  /// Maximum value for the z coordinate of the vertex (set by options)
  Gaudi::Property<double> m_zmax{this, "zVertexMax", 0.0 * Gaudi::Units::mm, "Max value for z coordinate"};

  /// Direction of the beam to take into account TOF vs nominal IP8, can have
  /// only values -1 or 1, or 0 to switch off the TOF and set time of
  /// interaction to zero (default = 1, as for beam 1)
  Gaudi::Property<int> m_zDir{this, "beamDirection", 1, "Direction of the beam, possible values: -1, 1 or 0"};

  /// Flat random number generator
  Rndm::Numbers m_flatDist;
};

#endif // GENERATION_FLATSMEARVERTEX_H
