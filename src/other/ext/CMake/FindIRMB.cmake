# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindIRMB
-----------

Find the Interactive and Robust Mesh Booleans library

IMPORTED targets
^^^^^^^^^^^^^^^^

This module defines the following :prop_tgt:`IMPORTED` target:

``IRMB::IRMB``

Result variables
^^^^^^^^^^^^^^^^

This module will set the following variables if found:

``IRMB_INCLUDE_DIRS``
where to find irmb.h, etc.
``IRMB_LIBRARIES``
the libraries to link against to use IRMB.
``IRMB_FOUND``
TRUE if found

#]=======================================================================]

# Look for the necessary header
find_path(IRMB_INCLUDE_DIR NAMES irmb.h)
mark_as_advanced(IRMB_INCLUDE_DIR)

# Look for the necessary library
find_library(IRMB_LIBRARY NAMES irmb sqlite)
mark_as_advanced(IRMB_LIBRARY)

# Look for the necessary exec
find_program(IRMB_EXECNAME NAMES irmb sqlite)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(IRMB
  REQUIRED_VARS IRMB_INCLUDE_DIR IRMB_LIBRARY
  )

# Create the imported target
if(IRMB_FOUND)
  set(IRMB_INCLUDE_DIRS ${IRMB_INCLUDE_DIR})
  set(IRMB_LIBRARIES ${IRMB_LIBRARY})
  if(NOT TARGET IRMB::IRMB)
    add_library(IRMB::IRMB UNKNOWN IMPORTED)
    set_target_properties(IRMB::IRMB PROPERTIES
      IMPORTED_LOCATION             "${IRMB_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${IRMB_INCLUDE_DIR}")
  endif()
endif()

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8
