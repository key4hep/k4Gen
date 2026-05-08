from Gaudi.Configuration import *
from GaudiKernel import SystemOfUnits as units

from Configurables import EventDataSvc
from k4FWCore import ApplicationMgr, IOSvc
ApplicationMgr(
               EvtSel='NONE',
               EvtMax=1,
               OutputLevel=INFO,
               ExtSvc=[EventDataSvc()],
              )

from edm4hep import labels as e4_labels

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
hepmc_converter.GenParticles.Path = e4_labels.MCParticles
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

iosvc = IOSvc()
iosvc.Output = "output_particleGun.root"
iosvc.outputCommands = ["keep *"]



