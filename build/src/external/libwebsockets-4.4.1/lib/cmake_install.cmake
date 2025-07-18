# Install script for directory: D:/Repositories/Cauto/src/external/libwebsockets-4.4.1/lib

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/Cauto")
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

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "D:/Program Files/MSYS2/mingw64/bin/objdump.exe")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "core" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/Repositories/Cauto/build/src/external/libwebsockets-4.4.1/lib/libwebsockets_static.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "core" OR NOT CMAKE_INSTALL_COMPONENT)
  message("-- Installing app dependencies for websockets...")
                    execute_process(COMMAND "C:/Program Files/PowerShell/7/pwsh.exe" -noprofile -executionpolicy Bypass -file "C:/Users/Administrator/vcpkg/scripts/buildsystems/msbuild/applocal.ps1"
                        -targetBinary "${CMAKE_INSTALL_PREFIX}/bin/libwebsockets_static.a"
                        -installedDir "C:/Users/Administrator/vcpkg/installed/-static/bin"
                        -OutVariable out)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "D:/Repositories/Cauto/build/src/external/libwebsockets-4.4.1/libwebsockets.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "D:/Repositories/Cauto/build/src/external/libwebsockets-4.4.1/libwebsockets_static.pc")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("D:/Repositories/Cauto/build/src/external/libwebsockets-4.4.1/lib/plat/windows/cmake_install.cmake")
  include("D:/Repositories/Cauto/build/src/external/libwebsockets-4.4.1/lib/core/cmake_install.cmake")
  include("D:/Repositories/Cauto/build/src/external/libwebsockets-4.4.1/lib/misc/cmake_install.cmake")
  include("D:/Repositories/Cauto/build/src/external/libwebsockets-4.4.1/lib/system/cmake_install.cmake")
  include("D:/Repositories/Cauto/build/src/external/libwebsockets-4.4.1/lib/core-net/cmake_install.cmake")
  include("D:/Repositories/Cauto/build/src/external/libwebsockets-4.4.1/lib/roles/cmake_install.cmake")
  include("D:/Repositories/Cauto/build/src/external/libwebsockets-4.4.1/lib/event-libs/cmake_install.cmake")
  include("D:/Repositories/Cauto/build/src/external/libwebsockets-4.4.1/lib/secure-streams/cmake_install.cmake")
  include("D:/Repositories/Cauto/build/src/external/libwebsockets-4.4.1/lib/secure-streams/serialized/client/cmake_install.cmake")

endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "D:/Repositories/Cauto/build/src/external/libwebsockets-4.4.1/lib/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
