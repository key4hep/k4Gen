#[[
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
]]
CMAKE_MINIMUM_REQUIRED(VERSION 3.15)

#---------------------------------------------------------------
project(k4Gen LANGUAGES CXX)

#- Dependencies ------------------------------------------------
find_package(ROOT COMPONENTS RIO Tree Hist)
find_package(Gaudi REQUIRED)
find_package(k4FWCore 1.3 REQUIRED)
find_package(EDM4HEP REQUIRED)

include(cmake/Key4hepConfig.cmake)

# use FindHEPPDT from Gaudi (used when finding heppdt later)
set(CMAKE_MODULE_PATH ${Gaudi_LIBRARY_DIR}/cmake/Gaudi/modules ${CMAKE_MODULE_PATH})

#---------------------------------------------------------------
include(GNUInstallDirs)
include(CTest)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/InstallArea/ CACHE PATH
    "Install path prefix, prepended onto install directories." FORCE )
endif()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

#---------------------------------------------------------------
add_subdirectory(k4Gen)

#---------------------------------------------------------------
install(EXPORT ${PROJECT_NAME}Targets
        NAMESPACE ${PROJECT_NAME}::
        FILE "${PROJECT_NAME}Targets.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}/"
        )

gaudi_install(CMAKE cmake/${PROJECT_NAME}Config.cmake)
