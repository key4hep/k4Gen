from Gaudi.Configuration import *
import os

from Configurables import EventDataSvc
from k4FWCore import ApplicationMgr, IOSvc

from Configurables import MDIReader
#mdi_converter = MDIReader("Reader",MDIFilename="k4Gen/options/mdireader_testparticles.dat")
mdi_converter = MDIReader("Reader",MDIFilename=(os.path.join(os.environ["K4GEN"],'../options/mdireader_testparticles.dat')))
mdi_converter.GenParticles.Path = "allGenParticles"
mdi_converter.CrossingAngle = 0.015
mdi_converter.LongitudinalCut = 0
mdi_converter.InputType = "xtrack"
mdi_converter.BeamEnergy = 45.6



iosvc = IOSvc()
iosvc.Output = "mdireader_test_out.root"
iosvc.outputCommands = ["keep *"]

ApplicationMgr( #TopAlg = [mdi_converter, geantsim],
                TopAlg = [mdi_converter],
                EvtSel = 'NONE',
                EvtMax   = 1,
                # order is important, as GeoSvc is needed by SimG4Svc
                #ExtSvc = [EventDataSvc(), geoservice, geantservice],
                ExtSvc = [EventDataSvc()],
                OutputLevel=INFO
               )
