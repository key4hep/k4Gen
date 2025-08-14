CMAKE_MINIMUM_REQUIRED(VERSION 3.15)

#---------------------------------------------------------------
project(k4Gen LANGUAGES CXX)

#- Dependencies ------------------------------------------------
find_package(k4FWCore 1.3 REQUIRED)
find_package(EDM4HEP REQUIRED)
find_package(Gaudi REQUIRED)
find_package(ROOT COMPONENTS RIO Tree Hist)

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
