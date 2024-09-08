set(CMAKE_C_FLAGS "-Ofast -m32" CACHE STRING "C compiler flags" FORCE)
set(CMAKE_CXX_FLAGS "-Ofast -m32" CACHE STRING "C++ compiler flags" FORCE)

set(LIB32 /usr/lib) # Fedora

if (EXISTS /usr/lib32)
    set(LIB32 /usr/lib32) # Arch, Solus
endif()

set(CMAKE_SYSTEM_LIBRARY_PATH ${LIB32} CACHE STRING "system library search path" FORCE)
set(CMAKE_LIBRARY_PATH ${LIB32} CACHE STRING "library search path" FORCE)

# this is probably unlikely to be needed, but just in case
set(CMAKE_EXE_LINKER_FLAGS "-m32 -L${LIB32}" CACHE STRING "executable linker flags" FORCE)
set(CMAKE_SHARED_LINKER_FLAGS "-m32 -L${LIB32}" CACHE STRING "shared library linker flags" FORCE)
set(CMAKE_MODULE_LINKER_FLAGS "-m32 -L${LIB32}" CACHE STRING "module linker flags" FORCE)

# find Wx config script on Fedora for the highest version of 32 bit Wx installed
if (EXISTS ${LIB32}/wx/config)
    file(GLOB WX_INSTALLS ${LIB32}/wx/config/*)

    set(MAX_WX_VERSION 0.0)
    foreach (WX_INSTALL ${WX_INSTALLS})
        string(REGEX MATCH "[0-9]+(\\.[0-9]+)+" WX_VERSION ${WX_INSTALL})

        if (WX_VERSION VERSION_GREATER MAX_WX_VERSION)
            set(MAX_WX_VERSION ${WX_VERSION})
        endif()
    endforeach()

    file(GLOB WX_INSTALL_CONFIGS "${LIB32}/wx/config/*${MAX_WX_VERSION}*")
    list(GET WX_INSTALL_CONFIGS 0 WX_INSTALL_CONFIG)

    set(WX_CONFIG_TRANSFORM_SCRIPT_LINES
        ""
    )
    file(WRITE ${CMAKE_BINARY_DIR}/wx-config-wrapper
"#!/bin/sh
${WX_INSTALL_CONFIG} \"\$@\" | sed 's!/emul32/!/usr/!g'
")
    execute_process(COMMAND chmod +x ${CMAKE_BINARY_DIR}/wx-config-wrapper)

    set(wxWidgets_CONFIG_EXECUTABLE ${CMAKE_BINARY_DIR}/wx-config-wrapper)
endif()

# on Fedora and Arch and similar, point pkgconfig at 32 bit .pc files. We have
# to include the regular system .pc files as well (at the end), because some
# are not always present in the 32 bit directory
if (EXISTS ${LIB32}/pkgconfig)
    set(ENV{PKG_CONFIG_LIBDIR} ${LIB32}/pkgconfig:/usr/share/pkgconfig:/usr/lib/pkgconfig:/usr/lib64/pkgconfig)
endif()
