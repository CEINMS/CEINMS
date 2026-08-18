
if (WIN32)

find_file(xerces_dll
    NAMES
        "xerces-c_3_1.dll"
        "xerces-c_3_1_vc100.dll"
    PATHS
        "C:/xerces-c-3.1.3_32r/bin"
        "C:/Program Files (x86)/CodeSynthesis XSD 4.0/bin64"
    PATH)
install(PROGRAMS ${xerces_dll} DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT ThirdParty)

# Package the OpenSim runtime DLLs next to the CEINMS executables.
#
# This matters for more than convenience. OpenSim ships identically named DLLs
# (osimSimulation.dll, ...) in every release, with no version resource on them,
# so Windows cannot tell 4.4 from 4.5 and simply loads whichever appears first
# on PATH. A CEINMS binary compiled against one OpenSim and run against another
# dies at load time with STATUS_ENTRYPOINT_NOT_FOUND (0xC0000139) before main()
# runs, so there is no error message and no chance to check the version from
# inside CEINMS. Because Windows searches the executable's own directory before
# PATH, shipping the matching DLLs here makes that mismatch impossible.
#
# file(GET_RUNTIME_DEPENDENCIES) resolves the actual transitive closure of the
# CEINMS executables, so we copy only what is really needed rather than the
# whole OpenSim bin directory (which also carries Moco's CasADi/tropter stack).
if(NOT DEFINED OpenSim_BIN_DIR)
    message(WARNING
        "OpenSim_BIN_DIR is not defined -- OpenSim runtime libraries will NOT be "
        "packaged. Ensure find_package(OpenSim) runs before include(CeinmsPack).")
else()
    message(STATUS "Packaging OpenSim runtime libraries from ${OpenSim_BIN_DIR}")
    install(CODE "
        # CMake >= 4.0 normalizes paths to forward slashes before matching the
        # exclude regexes below; without this it warns once per system DLL.
        if(POLICY CMP0207)
            cmake_policy(SET CMP0207 NEW)
        endif()
        # Scan the executables in the build tree rather than the install tree.
        # CMake emits the add_subdirectory() install scripts *after* the
        # top-level ones, so the installed copies do not exist yet at this
        # point. The build-tree copies have the same dependencies and are
        # already present once the build has run. Multi-config generators
        # (Visual Studio) put them under a per-config subdirectory; single-config
        # ones (Ninja, Makefiles) do not, so check both.
        set(_ceinms_exes)
        foreach(_exe CEINMS CEINMScalibrate CEINMSoptimise)
            foreach(_candidate
                    \"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/\${CMAKE_INSTALL_CONFIG_NAME}/\${_exe}${CMAKE_EXECUTABLE_SUFFIX}\"
                    \"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/\${_exe}${CMAKE_EXECUTABLE_SUFFIX}\")
                if(EXISTS \"\${_candidate}\")
                    list(APPEND _ceinms_exes \"\${_candidate}\")
                    break()
                endif()
            endforeach()
        endforeach()
        if(NOT _ceinms_exes)
            message(WARNING
                \"CEINMS: no built executables found under ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}; \"
                \"OpenSim runtime libraries were not packaged. Build CEINMS before installing.\")
            return()
        endif()
        file(GET_RUNTIME_DEPENDENCIES
            EXECUTABLES \${_ceinms_exes}
            RESOLVED_DEPENDENCIES_VAR _ceinms_resolved
            UNRESOLVED_DEPENDENCIES_VAR _ceinms_unresolved
            DIRECTORIES \"${OpenSim_BIN_DIR}\"
            PRE_EXCLUDE_REGEXES \"^api-ms-\" \"^ext-ms-\"
            POST_EXCLUDE_REGEXES \".*/[Ss][Yy][Ss][Tt][Ee][Mm]32/.*\" \".*/[Ww][Ii][Nn][Dd][Oo][Ww][Ss]/.*\")
        foreach(_dep IN LISTS _ceinms_resolved)
            file(INSTALL
                DESTINATION \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}\"
                TYPE SHARED_LIBRARY
                FOLLOW_SYMLINK_CHAIN
                FILES \"\${_dep}\")
        endforeach()
        list(LENGTH _ceinms_resolved _n_resolved)
        message(STATUS \"CEINMS: packaged \${_n_resolved} runtime libraries\")
        if(_ceinms_unresolved)
            message(STATUS \"CEINMS: unresolved (expected for system DLLs): \${_ceinms_unresolved}\")
        endif()
    " COMPONENT ThirdParty)
endif()

include(InstallRequiredSystemLibraries)
endif()
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Calibrated EMG-Informed NeuroMusculoSkeletal modeling")
set(CPACK_PACKAGE_VENDOR "The CEINMS team")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/NOTICE.txt")
set(CPACK_RESOURCE_FILE_WELCOME "${CMAKE_SOURCE_DIR}/CHANGELOG.txt")
set(CPACK_PACKAGE_VERSION_MAJOR ${CEINMS_MAJOR_VERSION})
set(CPACK_PACKAGE_VERSION_MINOR ${CEINMS_MINOR_VERSION})
set(CPACK_PACKAGE_VERSION_PATCH ${CEINMS_PATCH_VERSION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "CEINMS ${CEINMS_MAJOR_VERSION}.${CEINMS_MINOR_VERSION}")

set(CPACK_COMPONENTS_ALL Applications ThirdParty Unspecified)
set(CPACK_COMPONENT_UNSPECIFIED_DISPLAY_NAME "Additional files")
set(CPACK_COMPONENT_UNSPECIFIED_GROUP "Application Bundle")
set(CPACK_COMPONENT_THIRDPARTY_DISPLAY_NAME "Third parties")
set(CPACK_COMPONENT_THIRDPARTY_GROUP "Application Bundle")
set(CPACK_COMPONENT_THIRDPARTY_DEPENDS Unspecified)
set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "CEINMS Application")
set(CPACK_COMPONENT_APPLICATIONS_GROUP "Application Bundle")
set(CPACK_COMPONENT_APPLICATIONS_DEPENDS ThirdParty)


set(CPACK_PACKAGE_CONTACT "Monica Reggiani <monica.reggiani@gmail.com>")
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_RPM_COMPONENT_INSTALL ON)

# On Windows replace slashes in file name with escaped backslashes, as NSIS is picky about Unix paths
# ans using file(TO_NATIVE_PATH) is not enough, as \s are not escaped
if(WIN32)
    string(REPLACE "/" "\\\\" CPACK_PACKAGE_ICON "${CPACK_PACKAGE_ICON}")
    # Produce an installer for 64bit windows
    if("${CMAKE_GENERATOR}" MATCHES "Win64")
        set(CMAKE_CL_64 TRUE)
        set(CPACK_WIX_SIZEOF_VOID_P 8)
    endif()
endif()

if(CMAKE_CL_64)
    set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
    set(CPACK_NSIS_PACKAGE_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} (Win64)")
    set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${CPACK_PACKAGE_NAME}
    ${CPACK_PACKAGE_VERSION} (Win64)")
else()
    set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
    set(CPACK_NSIS_PACKAGE_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY}")
    set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${CPACK_PACKAGE_NAME}
            ${CPACK_PACKAGE_VERSION}")
endif()

set(CPACK_NSIS_MODIFY_PATH ON)
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)

include(CPack)
