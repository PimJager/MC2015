# Install script for directory: /Users/pimjager/Library/Mobile Documents/com~apple~CloudDocs/Radboud/S2 Model Checking/sylvan-master/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/pimjager/Library/Mobile Documents/com~apple~CloudDocs/Radboud/S2 Model Checking/sylvan-master/src/libsylvan.a")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsylvan.a" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsylvan.a")
    execute_process(COMMAND "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ranlib" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsylvan.a")
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "/Users/pimjager/Library/Mobile Documents/com~apple~CloudDocs/Radboud/S2 Model Checking/sylvan-master/src/lace.h"
    "/Users/pimjager/Library/Mobile Documents/com~apple~CloudDocs/Radboud/S2 Model Checking/sylvan-master/src/llmsset.h"
    "/Users/pimjager/Library/Mobile Documents/com~apple~CloudDocs/Radboud/S2 Model Checking/sylvan-master/src/sylvan.h"
    "/Users/pimjager/Library/Mobile Documents/com~apple~CloudDocs/Radboud/S2 Model Checking/sylvan-master/src/sylvan_cache.h"
    "/Users/pimjager/Library/Mobile Documents/com~apple~CloudDocs/Radboud/S2 Model Checking/sylvan-master/src/sylvan_common.h"
    "/Users/pimjager/Library/Mobile Documents/com~apple~CloudDocs/Radboud/S2 Model Checking/sylvan-master/src/sylvan_config.h"
    "/Users/pimjager/Library/Mobile Documents/com~apple~CloudDocs/Radboud/S2 Model Checking/sylvan-master/src/sylvan_bdd.h"
    "/Users/pimjager/Library/Mobile Documents/com~apple~CloudDocs/Radboud/S2 Model Checking/sylvan-master/src/sylvan_ldd.h"
    "/Users/pimjager/Library/Mobile Documents/com~apple~CloudDocs/Radboud/S2 Model Checking/sylvan-master/src/sylvan_mtbdd.h"
    "/Users/pimjager/Library/Mobile Documents/com~apple~CloudDocs/Radboud/S2 Model Checking/sylvan-master/src/sylvan_obj.hpp"
    "/Users/pimjager/Library/Mobile Documents/com~apple~CloudDocs/Radboud/S2 Model Checking/sylvan-master/src/tls.h"
    )
endif()

