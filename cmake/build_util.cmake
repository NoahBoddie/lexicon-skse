
function(add_local_dependency ARG_PROJ ARG_REPO)
	
	if(ARGN STREQUAL "")
        set(ARG_PACKNAME "${ARG_PROJ}::${ARG_PROJ}")
	else()
		set(ARG_PACKNAME ${ARGN})
    endif()

	find_package(ARG_PROJ QUIET)

	if(NOT ${ARG_PROJ}_FOUND)
		
		cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH CMAKE_REPO_DIR)
		set(PROJ_PATH "${CMAKE_REPO_DIR}/${ARG_REPO}")

		if (EXISTS "${PROJ_PATH}/CMakeLists.txt")
			message("it it it ${${ARG_PROJ}_LOCAL_LOADED}")
			message("yit yit yit ${commonlibsse_plugin_file}")
		
			if (DEFINED ${ARG_PROJ}_LOCAL_LOADED)	
				message("${ARG_REPO} already a dependency")	
				
			else()
				message(${ARG_PROJ}_LOCAL_LOADED)
				add_subdirectory(${PROJ_PATH} ${ARG_PROJ})
				add_dependencies(${PROJECT_NAME} ${ARG_PACKNAME})	
			endif()

			set (${ARG_PROJ}_LOCAL_LOADED true PARENT_SCOPE)

		else()
			message(WARN "${ARG_REPO} not found at \"${CMAKE_REPO_DIR}\"")
        endif()


		unset (PROJ_PATH)
		unset (ARG_PACKNAME)
		unset (CMAKE_REPO_DIR)
	else()
		message("${ARG_PROJ} was already included.")

	endif()

	#${ARGN}#use this to add whatever else needs to be used on findpackage

endfunction()


function(replace_add_local_dependency)
#using something like this
#NOT TARGET RoguesGallery
#additionally, if it's already there you should use target link library on it, so it links to the shared one.
endfunction()