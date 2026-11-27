"""
Pythia8 with event weights: the weights end up in the EventHeader, their names and
the generator in the metadata.
"""

import os

from Gaudi.Configuration import INFO
from Configurables import EventDataSvc, GenAlg, HepMCToEDMConverter, PythiaInterface
from edm4hep import labels as e4_labels
from k4FWCore import ApplicationMgr, IOSvc

pythia8gentool = PythiaInterface()
pythia8gentool.pythiacard = os.path.join(os.environ.get("K4GEN", ""), "ee_Z_ddbar_weights.cmd")

pythia8gen = GenAlg("Pythia8")
pythia8gen.SignalProvider = pythia8gentool
pythia8gen.hepmc.Path = "hepmc"

hepmc_converter = HepMCToEDMConverter()
hepmc_converter.hepmc.Path = "hepmc"
hepmc_converter.GenParticles.Path = e4_labels.MCParticles
hepmc_converter.EventHeader.Path = e4_labels.EventHeader
hepmc_converter.runNumber = 42

iosvc = IOSvc()
iosvc.Output = "output_pythia_weights.root"
iosvc.outputCommands = ["keep *"]

ApplicationMgr(
    TopAlg=[pythia8gen, hepmc_converter],
    EvtSel="NONE",
    EvtMax=10,
    ExtSvc=[EventDataSvc("EventDataSvc")],
    OutputLevel=INFO,
)
