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
from GaudiKernel import SystemOfUnits as units

from Configurables import ApplicationMgr
ApplicationMgr(
               EvtSel='NONE',
               EvtMax=1,
               OutputLevel=INFO,
              )

from Configurables import k4DataSvc
podioevent = k4DataSvc("EventDataSvc")
ApplicationMgr().ExtSvc += [podioevent]



from Configurables import ConstPtParticleGun
guntool1 = ConstPtParticleGun("SignalProvider", PdgCodes=[-211], PtMin=50, PtMax=50)
guntool2 = ConstPtParticleGun("PileUpProvider", PdgCodes=[11], writeParticleGunBranches=False)
from Configurables import FlatSmearVertex
smeartool = FlatSmearVertex()
smeartool.xVertexMin = -10*units.mm
smeartool.xVertexMax = 10*units.mm
smeartool.yVertexMin = -10*units.mm
smeartool.yVertexMax = 10*units.mm
smeartool.zVertexMin = -30*units.mm
smeartool.zVertexMax = 30*units.mm
from Configurables import GenAlg
gun = GenAlg()
gun.SignalProvider = guntool1
gun.hepmc.Path = "hepmc"
ApplicationMgr().TopAlg += [gun]

from Configurables import HepMCDumper
dumper = HepMCDumper()
dumper.hepmc.Path="hepmc"
ApplicationMgr().TopAlg += [dumper]

from Configurables import HepMCFileWriter
writer = HepMCFileWriter()
writer.hepmc.Path="hepmc"
ApplicationMgr().TopAlg += [writer]

from Configurables import HepMCToEDMConverter
hepmc_converter = HepMCToEDMConverter()
hepmc_converter.hepmc.Path="hepmc"
hepmc_converter.GenParticles.Path = "GenParticles"
ApplicationMgr().TopAlg += [hepmc_converter]

from Configurables import HepMCHistograms
histo = HepMCHistograms("GenHistograms")
histo.hepmc.Path="hepmc"
ApplicationMgr().TopAlg += [histo]

from Configurables import THistSvc
THistSvc().Output = ["rec DATAFILE='output_particleGun_GenHistograms.root' TYP='ROOT' OPT='RECREATE'"]
THistSvc().PrintAll=True
THistSvc().AutoSave=True
THistSvc().AutoFlush=True
THistSvc().OutputLevel=INFO

from Configurables import PodioOutput
out = PodioOutput("out", filename = "output_particleGun.root")
out.outputCommands = ["keep *"]
ApplicationMgr().TopAlg += [out]



