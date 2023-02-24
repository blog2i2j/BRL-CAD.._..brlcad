set(irmb_DESCRIPTION "
Option for enabling and disabling compilation of the Interactive and Robust
Mesh Booleans library provided with BRL-CAD's source code.  Default is AUTO,
responsive to the toplevel BRLCAD_BUNDLED_LIBS option and testing first for a
system version if BRLCAD_BUNDLED_LIBS is also AUTO.
")

# Note - this component is handled as a third party build because it doesn't
# currently function well as an "add_subdirectory" component, but there is no
# system version possible - our mods to make the library are custom.  Therefore,
# we pass the NOSYS flag.
THIRD_PARTY(irmb IRMB irmb
  irmb_DESCRIPTION
  REQUIRED_VARS BRLCAD_LEVEL2
  ALIASES ENABLE_IRMB
  RESET_VARS IRMB_LIBRARY IRMB_INCLUDE_DIR
  NOSYS UNDOCUMENTED
  )

if (BRLCAD_IRMB_BUILD)

  set(IRMB_MAJOR_VERSION 1)
  set(IRMB_MINOR_VERSION 0)
  set(IRMB_PATCH_VERSION 1)
  set(IRMB_VERSION ${IRMB_MAJOR_VERSION}.${IRMB_MINOR_VERSION}.${IRMB_PATCH_VERSION})
  set_lib_vars(IRMB irmb ${IRMB_MAJOR_VERSION} ${IRMB_MINOR_VERSION} ${IRMB_PATCH_VERSION})

  if (MSVC)
    set(IRMB_BASENAME irmb)
    set(TBB_BASENAME tbb)
  else (MSVC)
    set(IRMB_BASENAME libirmb)
    set(TBB_BASENAME libtbb)
  endif (MSVC)

  set(IRMB_INSTDIR ${CMAKE_BINARY_INSTALL_ROOT}/irmb)

  ExternalProject_Add(IRMB_BLD
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/irmb"
    BUILD_ALWAYS ${EXT_BUILD_ALWAYS} ${LOG_OPTS}
    CMAKE_ARGS
    $<$<NOT:$<BOOL:${CMAKE_CONFIGURATION_TYPES}>>:-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}>
    -DBIN_DIR=${BIN_DIR}
    -DCMAKE_INSTALL_BINDIR=${BIN_DIR}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_INSTALL_PREFIX=${IRMB_INSTDIR}
    -DCMAKE_INSTALL_RPATH=${CMAKE_BUILD_RPATH}
    -DCMAKE_INSTALL_RPATH_USE_LINK_PATH=${CMAKE_INSTALL_RPATH_USE_LINK_PATH}
    -DCMAKE_SKIP_BUILD_RPATH=${CMAKE_SKIP_BUILD_RPATH}
    -DLIB_DIR=${LIB_DIR}
    -DCMAKE_INSTALL_LIBDIR=${LIB_DIR}
    -DBUILD_EXAMPLES=OFF
    LOG_CONFIGURE ${EXT_BUILD_QUIET}
    LOG_BUILD ${EXT_BUILD_QUIET}
    LOG_INSTALL ${EXT_BUILD_QUIET}
    LOG_OUTPUT_ON_FAILURE ${EXT_BUILD_QUIET}
    )

  DISTCLEAN("${CMAKE_CURRENT_BINARY_DIR}/IRMB_BLD-prefix")

 # Tell the parent build about files and libraries
 ExternalProject_Target(SHARED irmb IRMB_BLD ${IRMB_INSTDIR}
   ${IRMB_BASENAME}${IRMB_SUFFIX}
   SYMLINKS ${IRMB_SYMLINK_1};${IRMB_SYMLINK_2}
   LINK_TARGET ${IRMB_SYMLINK_1}
   RPATH
   )

 # For now, bring along the copy of tbb build for this purpose.
 set_lib_vars(TBB tbb "12" "7" "")
 ExternalProject_Target(SHARED tbb IRMB_BLD ${IRMB_INSTDIR}
   ${TBB_BASENAME}${TBB_SUFFIX}
   SYMLINKS ${TBB_SYMLINK_1};${TBB_SYMLINK_2}
   LINK_TARGET ${TBB_SYMLINK_1}
   RPATH
   )

 ExternalProject_ByProducts(irmb IRMB_BLD ${IRMB_INSTDIR} ${INCLUDE_DIR}
   irmb/irmb.h
   )
 set(SYS_INCLUDE_PATTERNS ${SYS_INCLUDE_PATTERNS} irmb CACHE STRING "Bundled system include dirs" FORCE)

 set(IRMB_LIBRARY irmb CACHE STRING "Building bundled irmb" FORCE)
 set(IRMB_LIBRARIES irmb CACHE STRING "Building bundled irmb" FORCE)
 set(IRMB_INCLUDE_DIR "${CMAKE_BINARY_ROOT}/${INCLUDE_DIR}" CACHE STRING "Directory containing irmb headers." FORCE)
 set(IRMB_INCLUDE_DIRS "${CMAKE_BINARY_ROOT}/${INCLUDE_DIR}" CACHE STRING "Directory containing irmb headers." FORCE)

 SetTargetFolder(IRMB_BLD "Third Party Libraries")
 SetTargetFolder(irmb "Third Party Libraries")

endif (BRLCAD_IRMB_BUILD)

include("${CMAKE_CURRENT_SOURCE_DIR}/irmb.dist")

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

