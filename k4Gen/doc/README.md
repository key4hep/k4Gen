<!--
Copyright (c) 2020-2024 Key4hep-Project.

This file is part of Key4hep.
See https://key4hep.github.io/key4hep-doc/ for further info.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
-->

FCCSW Generation Package Documentation
--------------------------------------


The Generation package consists of framework components and utilities to provide the input to the detector simulation stage of the software.
The currently available implementations are sparse and will not cover every usecase -- an effort is underway to refactor generic parts of the software of LHC experiments, but the current code illustrates the use of the event store and other framework features. The most basic usecases -- i/o from a HepMC-file, a particle gun and an interface to the Pythia event generator -- are nonetheless available. Refer to the doxygen pages and examples for details.


