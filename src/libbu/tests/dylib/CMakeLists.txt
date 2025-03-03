if(BUILD_TESTING)
  set_property(
    SOURCE dylib.c
    APPEND
    PROPERTY COMPILE_DEFINITIONS "DYLIB_PLUGIN_SUFFIX=\"${CMAKE_SHARED_LIBRARY_SUFFIX}\""
  )

  add_library(plugin_1 SHARED plugin_1.cpp)
  target_link_libraries(plugin_1 libbu)
  add_library(plugin_2 SHARED plugin_2.cpp)
  target_link_libraries(plugin_2 libbu)

  set(plugin_targets plugin_1 plugin_2)
  set(DIR_TYPES LIBRARY RUNTIME ARCHIVE)

  # Override the output directories to put the plugins in LIBEXEC_DIR
  foreach(target_name ${plugin_targets})
    foreach(dt ${DIR_TYPES})
      set_property(TARGET ${target_name} PROPERTY ${dt}_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${LIBEXEC_DIR}/dylib")
      if(CMAKE_CONFIGURATION_TYPES)
	foreach(ct ${CMAKE_CONFIGURATION_TYPES})
	  string(TOUPPER "${ct}" ct)
	  set_property(
	    TARGET ${target_name}
	    PROPERTY ${dt}_OUTPUT_DIRECTORY_${ct} "${CMAKE_BINARY_DIR}/${LIBEXEC_DIR}/dylib"
	  )
	endforeach(ct ${CMAKE_CONFIGURATION_TYPES})
      endif(CMAKE_CONFIGURATION_TYPES)
    endforeach(dt ${DIR_TYPES})
    set_target_properties(${target_name} PROPERTIES FOLDER "BRL-CAD Executables/Test Programs/dylib")
  endforeach(target_name${plugins})

  add_library(libdylib SHARED dylib.c)
  set_target_properties(libdylib PROPERTIES PREFIX "")
  if(HIDE_INTERNAL_SYMBOLS)
    set_property(TARGET plugin_1 APPEND PROPERTY COMPILE_DEFINITIONS BU_DYLIB_DLL_EXPORTS)
    set_property(TARGET plugin_2 APPEND PROPERTY COMPILE_DEFINITIONS BU_DYLIB_DLL_EXPORTS)
    set_property(TARGET libdylib APPEND PROPERTY COMPILE_DEFINITIONS BU_DYLIB_DLL_EXPORTS)
  endif(HIDE_INTERNAL_SYMBOLS)
  target_link_libraries(libdylib libbu)
  set_target_properties(libdylib PROPERTIES FOLDER "BRL-CAD Executables/Test Programs/dylib")

  add_executable(bu_dylib run.c)
  target_link_libraries(bu_dylib libdylib libbu)
  set_property(TARGET bu_dylib APPEND PROPERTY COMPILE_DEFINITIONS BRLCADBUILD HAVE_CONFIG_H)
  if(HIDE_INTERNAL_SYMBOLS)
    set_property(TARGET bu_dylib APPEND PROPERTY COMPILE_DEFINITIONS BU_DYLIB_DLL_IMPORTS)
  endif(HIDE_INTERNAL_SYMBOLS)
  set_target_properties(bu_dylib PROPERTIES FOLDER "BRL-CAD Executables/Test Programs/dylib")

  # As a build convenience, make bu_dylib depend on everything
  add_dependencies(bu_dylib plugin_1 plugin_2)

  brlcad_add_test(NAME bu_dylib COMMAND bu_dylib)
endif(BUILD_TESTING)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8
