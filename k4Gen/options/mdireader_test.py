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
from Gaudi.Configuration import *
import os

# Data service
from Configurables import FCCDataSvc
podioevent = FCCDataSvc("EventDataSvc")

from Configurables import MDIReader
#mdi_converter = MDIReader("Reader",MDIFilename="k4Gen/options/mdireader_testparticles.dat")
mdi_converter = MDIReader("Reader",MDIFilename=(os.path.join(os.environ["K4GEN"],'../options/mdireader_testparticles.dat')))
mdi_converter.GenParticles.Path = "allGenParticles"
mdi_converter.CrossingAngle = 0.015
mdi_converter.LongitudinalCut = 0
mdi_converter.InputType = "xtrack"
mdi_converter.BeamEnergy = 45.6



# PODIO algorithm
from Configurables import PodioOutput
out = PodioOutput("out",
                  filename="mdireader_test_out.root",
                  OutputLevel=INFO)
out.outputCommands = ["keep *"]

# ApplicationMgr
from Configurables import ApplicationMgr
ApplicationMgr( #TopAlg = [mdi_converter, geantsim, out],
                TopAlg = [mdi_converter, out],
                EvtSel = 'NONE',
                EvtMax   = 1,
                # order is important, as GeoSvc is needed by SimG4Svc
                #ExtSvc = [podioevent, geoservice, geantservice],
                ExtSvc = [podioevent],
                OutputLevel=INFO
               )
