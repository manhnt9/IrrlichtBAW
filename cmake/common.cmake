
# Macro creating project for an executable
# Project and target get its name from directory when this macro gets executed (truncating number in the beginning of the name and making all lower case)
# Created because of common cmake code for examples and tools
macro(irr_create_executable_project _EXTRA_SOURCES _EXTRA_OPTIONS _EXTRA_INCLUDES _EXTRA_LIBS)
	get_filename_component(EXECUTABLE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
	string(REGEX REPLACE "^[0-9]+\." "" EXECUTABLE_NAME ${EXECUTABLE_NAME})
	string(TOLOWER ${EXECUTABLE_NAME} EXECUTABLE_NAME)

	project(${EXECUTABLE_NAME})

	add_executable(${EXECUTABLE_NAME} main.cpp ${_EXTRA_SOURCES}) 
	# EXTRA_SOURCES is var containing non-common names of sources (if any such sources, then EXTRA_SOURCES must be set before including this cmake code)
	add_dependencies(${EXECUTABLE_NAME} Irrlicht)

	target_include_directories(${EXECUTABLE_NAME}
		PUBLIC ../../include
		PRIVATE ${_EXTRA_INCLUDES}
	)
	target_link_libraries(${EXECUTABLE_NAME} Irrlicht ${_EXTRA_LIBS})
	add_compile_options(${_EXTRA_OPTIONS})
	
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		add_compile_options(
			"$<$<CONFIG:DEBUG>:-fstack-protector-all>"
		)
	
		set(COMMON_LINKER_OPTIONS "-msse4.2 -mfpmath=sse -fuse-ld=gold")
		set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${COMMON_LINKER_OPTIONS}")
		set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${COMMON_LINKER_OPTIONS} -fstack-protector-strong -fsanitize=address")
	endif()

	irr_adjust_flags() # macro defined in root CMakeLists
	irr_adjust_definitions() # macro defined in root CMakeLists

	set_target_properties(${EXECUTABLE_NAME} PROPERTIES DEBUG_POSTFIX _d)
	set_target_properties(${EXECUTABLE_NAME}
		PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
	)
	if(MSVC)
		set_target_properties(${EXECUTABLE_NAME}
			PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}/bin"
			RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}/bin"
			VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/bin" # seems like has no effect
		)
	endif()
endmacro()

macro(irr_create_ext_library_project EXT_NAME LIB_HEADERS LIB_SOURCES LIB_INCLUDES LIB_OPTIONS)
	set(LIB_NAME "IrrExt${EXT_NAME}")
	project(${LIB_NAME})

	add_library(${LIB_NAME} ${LIB_SOURCES})
	# EXTRA_SOURCES is var containing non-common names of sources (if any such sources, then EXTRA_SOURCES must be set before including this cmake code)
	add_dependencies(${LIB_NAME} Irrlicht)

	target_include_directories(${LIB_NAME}
		PUBLIC ${CMAKE_BINARY_DIR}/include/irr/config/debug
		PUBLIC ${CMAKE_BINARY_DIR}/include/irr/config/release
		PUBLIC ${CMAKE_SOURCE_DIR}/include
		PUBLIC ${CMAKE_SOURCE_DIR}/src
		PUBLIC ${CMAKE_SOURCE_DIR}/source/Irrlicht
		PRIVATE ${LIB_INCLUDES}
	)
	add_dependencies(${LIB_NAME} Irrlicht)
	target_compile_options(${LIB_NAME} PUBLIC ${LIB_OPTIONS})

	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		add_compile_options(
			"$<$<CONFIG:DEBUG>:-fstack-protector-all>"
		)

		set(COMMON_LINKER_OPTIONS "-msse4.2 -mfpmath=sse -fuse-ld=gold")
		set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${COMMON_LINKER_OPTIONS}")
		set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${COMMON_LINKER_OPTIONS} -fstack-protector-strong -fsanitize=address")
	endif()

	irr_adjust_flags() # macro defined in root CMakeLists
	irr_adjust_definitions() # macro defined in root CMakeLists

	set_target_properties(${LIB_NAME} PROPERTIES DEBUG_POSTFIX _d)
	set_target_properties(${LIB_NAME}
		PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
	)
	if(MSVC)
		set_target_properties(${LIB_NAME}
			PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}/bin"
			RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}/bin"
			VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/bin" # seems like has no effect
		)
	endif()

	install(
		FILES ${LIB_HEADERS}
		DESTINATION ./include/irr/ext/${EXT_NAME}
		CONFIGURATIONS Release
	)
	install(
		FILES ${LIB_HEADERS}
		DESTINATION ./debug/include/irr/ext/${EXT_NAME}
		CONFIGURATIONS Debug
	)
	install(
		TARGETS ${LIB_NAME}
		DESTINATION ./lib/irr/ext/${EXT_NAME}
		CONFIGURATIONS Release
	)
	install(
		TARGETS ${LIB_NAME}
		DESTINATION ./debug/lib/irr/ext/${EXT_NAME}
		CONFIGURATIONS Debug
	)

	set("IRR_EXT_${EXT_NAME}_INCLUDE_DIRS"
		"${CMAKE_SOURCE_DIR}/include/"
		"${CMAKE_SOURCE_DIR}/src"
		"${CMAKE_SOURCE_DIR}/source/Irrlicht"
		"${CMAKE_SOURCE_DIR}/ext/${EXT_NAME}"
		"${LIB_INCLUDES}"
		PARENT_SCOPE
	)
	set("IRR_EXT_${EXT_NAME}_LIB"
		"${LIB_NAME}"
		PARENT_SCOPE
	)
endmacro()


function(irr_get_conf_dir _OUTVAR _CONFIG)
	string(TOLOWER ${_CONFIG} CONFIG)
	set(${_OUTVAR} "${CMAKE_BINARY_DIR}/include/irr/config/${CONFIG}" PARENT_SCOPE)
endfunction()


# function for installing header files preserving directory structure
# _DEST_DIR is directory relative to CMAKE_INSTALL_PREFIX
function(irr_install_headers _HEADERS _BASE_HEADERS_DIR)
	foreach (file ${_HEADERS})
		file(RELATIVE_PATH dir ${_BASE_HEADERS_DIR} ${file})
		get_filename_component(dir ${dir} DIRECTORY)
		install(FILES ${file} DESTINATION include/${dir} CONFIGURATIONS Release)
		install(FILES ${file} DESTINATION debug/include/${dir} CONFIGURATIONS Debug)
	endforeach()
endfunction()

function(irr_install_config_header _CONF_HDR_NAME)
	irr_get_conf_dir(dir_deb Debug)
	irr_get_conf_dir(dir_rel Release)
	set(file_deb "${dir_deb}/${_CONF_HDR_NAME}")
	set(file_rel "${dir_rel}/${_CONF_HDR_NAME}")
	install(FILES ${file_rel} DESTINATION include CONFIGURATIONS Release)
	install(FILES ${file_deb} DESTINATION debug/include CONFIGURATIONS Debug)
endfunction()
