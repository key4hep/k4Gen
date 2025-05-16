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
#ifndef GENERATION_GAUSSSMEARVERTEX_H
#define GENERATION_GAUSSSMEARVERTEX_H

#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "GaudiKernel/RndmGenerators.h"

#include "Generation/IVertexSmearingTool.h"

/** @class GaussSmearVertex GaussSmearVertex.h "GaussSmearVertex.h"
 *
 *  Tool to smear vertex with gaussian smearing along the x- y- z- and t-axis.
 *  Concrete implementation of a IVertexSmearingTool.
 *
 */
class GaussSmearVertex : public AlgTool, virtual public IVertexSmearingTool {
public:
  /// Standard constructor
  GaussSmearVertex(const std::string& type, const std::string& name, const IInterface* parent);

  virtual ~GaussSmearVertex(); ///< Destructor

  /// Initialize method
  virtual StatusCode initialize();

  /** Implements IVertexSmearingTool::smearVertex.
   */
  virtual StatusCode smearVertex(HepMC3::GenEvent& theEvent);

private:
  Gaudi::Property<double> m_xsig{this, "xVertexSigma", 0.0 * Gaudi::Units::mm, "Spread of x coordinate"};
  Gaudi::Property<double> m_ysig{this, "yVertexSigma", 0.0 * Gaudi::Units::mm, "Spread of y coordinate"};
  Gaudi::Property<double> m_zsig{this, "zVertexSigma", 0.0 * Gaudi::Units::mm, "Spread of z coordinate"};
  Gaudi::Property<double> m_tsig{this, "tVertexSigma", 0.0 * Gaudi::Units::mm, "Spread of t coordinate"};

  Gaudi::Property<double> m_xmean{this, "xVertexMean", 0.0 * Gaudi::Units::mm, "Mean of x coordinate"};
  Gaudi::Property<double> m_ymean{this, "yVertexMean", 0.0 * Gaudi::Units::mm, "Mean of y coordinate"};
  Gaudi::Property<double> m_zmean{this, "zVertexMean", 0.0 * Gaudi::Units::mm, "Mean of z coordinate"};
  Gaudi::Property<double> m_tmean{this, "tVertexMean", 0.0 * Gaudi::Units::mm, "Mean of t coordinate"};

  Rndm::Numbers m_gaussDist;
};

#endif // GENERATION_GAUSSSMEARVERTEX_H
