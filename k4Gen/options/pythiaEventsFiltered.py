#
# Copyright (c) 2020-2024 Key4hep-Project.
#
# This file is part of Key4hep.
# See https://key4hep.github.io/key4hep-doc/ for further info.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
'''
Pythia8, integrated in the Key4hep ecosystem.

Generate events according to a Pythia .cmd file and save them in EDM4hep
format.
'''

import os

from GaudiKernel import SystemOfUnits as units
from Gaudi.Configuration import INFO, DEBUG

from Configurables import ApplicationMgr, k4DataSvc, PodioOutput
from Configurables import GaussSmearVertex, PythiaInterface, GenAlg
from Configurables import HepMCToEDMConverter, GenParticleFilter
from Configurables import GenEventFilter


ApplicationMgr().EvtSel = 'NONE'
ApplicationMgr().EvtMax = 20
ApplicationMgr().OutputLevel = INFO
ApplicationMgr().ExtSvc += ["RndmGenSvc"]

# Data service
podioevent = k4DataSvc("EventDataSvc")
ApplicationMgr().ExtSvc += [podioevent]

smeartool = GaussSmearVertex()
smeartool.xVertexSigma = 0.5 * units.mm
smeartool.yVertexSigma = 0.5 * units.mm
smeartool.zVertexSigma = 40.0 * units.mm
smeartool.tVertexSigma = 180.0 * units.picosecond

pythia8gentool = PythiaInterface()
# Example of Pythia configuration file to generate events
# taken from $K4GEN if defined, locally otherwise
path_to_pythiafile = os.environ.get("K4GEN", "")
PYTHIA_FILENAME = "Pythia_standard.cmd"
pythiafile = os.path.join(path_to_pythiafile, PYTHIA_FILENAME)
# Example of pythia configuration file to read LH event file
# pythiafile="options/Pythia_LHEinput.cmd"
pythia8gentool.pythiacard = pythiafile
pythia8gentool.doEvtGenDecays = False
pythia8gentool.printPythiaStatistics = True
pythia8gentool.pythiaExtraSettings = [""]

pythia8gen = GenAlg("Pythia8")
pythia8gen.SignalProvider = pythia8gentool
pythia8gen.VertexSmearingTool = smeartool
pythia8gen.hepmc.Path = "hepmc"
ApplicationMgr().TopAlg += [pythia8gen]

# Reads an HepMC::GenEvent from the data service and writes a collection of
# EDM Particles
hepmc_converter = HepMCToEDMConverter()
hepmc_converter.hepmc.Path = "hepmc"
hepmc_converter.hepmcStatusList = []  # convert particles with all statuses
hepmc_converter.GenParticles.Path = "GenParticles"
ApplicationMgr().TopAlg += [hepmc_converter]

# Filters events
eventfilter = GenEventFilter("EventFilter")
eventfilter.particles.Path = "GenParticles"
# eventfilter.filterRule = \
#     "bool filterRule(const edm4hep::MCParticleCollection* inColl){" \
#     "  return inColl->size() > 1000;}"
eventfilter.filterRulePath = "k4Gen/options/filterRule.hxx"
eventfilter.OutputLevel = DEBUG
ApplicationMgr().TopAlg += [eventfilter]

out = PodioOutput("out")
out.outputCommands = ["keep *"]
ApplicationMgr().TopAlg += [out]
