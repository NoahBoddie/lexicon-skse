cmake_minimum_required(VERSION 3.21)
message("Using toolchain file ${CMAKE_TOOLCHAIN_FILE}.")

########################################################################################################################
## Define project
########################################################################################################################
project(
        LexiconSKSE
        VERSION 0.3.3.2
        DESCRIPTION "An SKSE implemenation of Lexicon used as a scripting engine and conditions system."
        LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

include(GNUInstallDirs)
include (cmake/build_util.cmake)


set(headers
        include/GameObjectStuff.h
        
)
set(sources
    
)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

    configure_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in
            ${CMAKE_CURRENT_BINARY_DIR}/version.rc
            @ONLY)

    set(headers
        ${headers}
        "src/MergeMapperPluginAPI.h"
        src/FunctionRegister.h
        src/ConditionFunction.h
        "src/FunctorManager.h"
        "src/ConditionTLS.h"
        "src/Hooks.hpp"
        src/ScriptFunctions.hpp
     "src/ScriptFunctions/Register_ObjectReference.h" "src/ScriptFunctions/Register_Door.h" "src/ScriptFunctions/Utility.h" "src/ScriptFunctions/Register_Condition.h" "src/CachedScript.h" "src/Hooks/Hook_GetNoRumors.h" "src/Hooks/Hook_RunConsole.h" "src/Hooks/Hook_ResolveCondition.h" "src/Hooks/Hook_LoadCondition.h" "src/Hooks/Hook_SaveConditionParams.h" "src/Hooks/Hook_SaveConditionResult.h" "src/Hooks/Hook_SaveConsoleResult.h" "src/Hooks/Hook_FilesLoaded.h" "src/ScriptFunctions/Register_Location.h" "src/ScriptFunctions/Register_Form.h")

    set(sources
            ${sources}
            src/main.cpp
            "src/MergeMapperPluginAPI.cpp"
            "src/GameObjectStuff.cpp"
            "src/MergeMapperPluginAPI.cpp"

            ${CMAKE_CURRENT_BINARY_DIR}/version.rc )
endif()



source_group(
        TREE ${CMAKE_CURRENT_SOURCE_DIR}
        FILES
        ${headers}
        ${sources})

#########################################################################################################################
### Build options
#########################################################################################################################
message("Options:")
Set(BUILD_TESTS OFF)
message("\tTests: ${BUILD_TESTS}")

########################################################################################################################
## Configure target DLL
########################################################################################################################
find_package(directxtk CONFIG REQUIRED)
find_package(CommonLibSSE CONFIG REQUIRED)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
find_package(nlohmann_json REQUIRED)
find_package(nlohmann_json_schema_validator REQUIRED)
endif()

#add_commonlibsse_plugin(${PROJECT_NAME} SOURCES ${headers} ${sources})
#add_library("${PROJECT_NAME}::${PROJECT_NAME}" ALIAS "${PROJECT_NAME}")
add_library(
		${PROJECT_NAME}
		SHARED
		${headers}
		${sources}
)
target_commonlibsse_properties(${PROJECT_NAME} ${headers} ${sources})
add_library("${PROJECT_NAME}::${PROJECT_NAME}" ALIAS "${PROJECT_NAME}")
set(TEST_BUILD OFF)



set (SETTING_PATH Data/SKSE/Plugins)



#RGL stuff
#if (NOT LexPath)
#	cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH CMAKE_REPO_DIR)
#	set(LexPath "${CMAKE_REPO_DIR}/lexicon")
#endif()

#add_subdirectory(${LexPath} Lexicon)
#add_dependencies(${PROJECT_NAME} Lexicon::Lexicon)	

add_local_dependency(Lexicon lexicon)
add_local_dependency(Lexicon lexicon)
add_local_dependency(RoguesGalleryClib rogues-gallery-clib)
message(FINISH XXXX)
	
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
        Lexicon::Lexicon
        nlohmann_json::nlohmann_json
        nlohmann_json_schema_validator::validator
)
else()
target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
        Lexicon::Lexicon
)
endif()


message(FINISH XXXX)


	target_compile_options(
		${PROJECT_NAME}
		PRIVATE			
			/W4	# Warning level
			#/WX	# Warning level (warnings are errors)
			"$<$<CONFIG:DEBUG>:>"
			"$<$<CONFIG:RELEASE>:/Zc:inline;/JMC-;/Ob3>"
	)

	target_link_options(
		${PROJECT_NAME}
		PRIVATE
			"$<$<CONFIG:DEBUG>:/INCREMENTAL;/OPT:NOREF;/OPT:NOICF>"
			"$<$<CONFIG:RELEASE>:/INCREMENTAL:NO;/OPT:REF;/OPT:ICF;/DEBUG:FULL>"
	)



target_include_directories(${PROJECT_NAME}
        PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src>
        $<INSTALL_INTERFACE:src>
        ${ARTICUNO_INCLUDE_DIRS})

target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

target_precompile_headers(${PROJECT_NAME}
        PRIVATE
        src/PCH.h)

install(TARGETS ${PROJECT_NAME}
        DESTINATION "${CMAKE_INSTALL_LIBDIR}")


########################################################################################################################
## Automatic plugin deployment
########################################################################################################################
# Automatic deployment to FOMOD directory.
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(BUILD_NAME "Debug")
    else()
        set(BUILD_NAME "Release")
    endif()
    install(DIRECTORY DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/Papyrus${BUILD_NAME}/")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/Plugin${BUILD_NAME}/")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/Plugin${BUILD_NAME}/")
    if(${CMAKE_BUILD_TYPE} STREQUAL Debug OR ${CMAKE_BUILD_TYPE} STREQUAL RelWithDebInfo)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PDB_FILE:${PROJECT_NAME}> "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/Plugin${BUILD_NAME}/")
    endif()
    file(GLOB_RECURSE OUTPUT_DLLS "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/**/*.dll")
    file(GLOB_RECURSE OUTPUT_PDBS "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/**/*.pdb")
    file(GLOB_RECURSE OUTPUT_SCRIPTS "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/**/*.pex")
    set_property(TARGET ${PROJECT_NAME}
            APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${OUTPUT_DLLS}")
    set_property(TARGET ${PROJECT_NAME}
            APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${OUTPUT_PDBS}")
    set_property(TARGET ${PROJECT_NAME}
            APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${OUTPUT_SCRIPTS}")

    # Automatic deployment to Mod Organizer 2 mod directory.
    foreach(DEPLOY_TARGET $ENV{CommonLibSSESamplePluginTargets})
        message("Adding deployment target ${DEPLOY_TARGET}.")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> "${DEPLOY_TARGET}/SKSE/Plugins/")
        if(${CMAKE_BUILD_TYPE} STREQUAL Debug OR ${CMAKE_BUILD_TYPE} STREQUAL RelWithDebInfo)
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PDB_FILE:${PROJECT_NAME}> "${DEPLOY_TARGET}/SKSE/Plugins/")
        endif()
    endforeach()
endif()
