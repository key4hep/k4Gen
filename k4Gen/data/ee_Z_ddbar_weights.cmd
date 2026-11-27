! ee -> Z -> ddbar with two shower variation weights in addition to the nominal one
Random:setSeed = on
Random:seed = 42
Main:numberOfEvents = 10           ! number of events to generate
Main:timesAllowErrors = 5          ! how many aborts before run stops
Init:showChangedSettings = on      ! list changed settings
Init:showChangedParticleData = off ! list changed particle data
Next:numberCount = 100             ! print message every n events
Next:numberShowInfo = 1            ! print event information n times
Next:numberShowProcess = 1         ! print process record n times
Next:numberShowEvent = 0           ! print event record n times

Beams:idA = 11                     ! first beam, e- = 11
Beams:idB = -11                    ! second beam, e+ = -11
Beams:eCM = 91                     ! CM energy of collision

WeakSingleBoson:ffbar2ffbar(s:gmZ) = on
WeakZ0:gmZmode = 2
23:onMode = off
23:onIfMatch = 1 1

! Shower variations, each adds a weight
UncertaintyBands:doVariations = on
UncertaintyBands:List = { fsrRhi fsr:muRfac=2.0, fsrRlo fsr:muRfac=0.5 }
