
function(port_local ARG_PATH ARG_PROJ)

add_subdirectory(${ARG_PATH} ARG_PROJ)
add_dependencies(${PROJECT_NAME} ARG_PROJ::ARG_PROJ)	

endfunction()



function(handle_port ARG_USER ARG_PROJ ARG_REF ARG_SHA ARG_HEAD)
    
    message(HELPMEHELP  "$ENV{REPO_PATH}")
    if (DEFINED ENV{REPO_PATH})
        set(REPO_PATH $ENV{REPO_PATH})
        message(HELPMEHELP REPO_PATH)

        if (EXISTS "${REPO_PATH}/${PORT}/CMakeLists.txt")
            port_local(${REPO_PATH} ${ARG_PROJ})
            return()
        endif()
    endif()
    
    vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO ${ARG_USER}/${PORT}
        REF  ${ARG_REF}
        SHA512  ${ARG_SHA}
        HEAD_REF ${ARG_HEAD}
    )

endfunction()