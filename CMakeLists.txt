cmake_minimum_required(VERSION 3.14.0)

option(SMTG_ENABLE_VST3_PLUGIN_EXAMPLES "Enable VST 3 Plug-in Examples" OFF)
option(SMTG_ENABLE_VST3_HOSTING_EXAMPLES "Enable VST 3 Hosting Examples" OFF)

set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13 CACHE STRING "")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)


set(vst3sdk_SOURCE_DIR "C:/workspace/externals/vst3sdk")
if(NOT vst3sdk_SOURCE_DIR)
    message(FATAL_ERROR "Path to VST3 SDK is empty!")
endif()

project(K_wire_2
    # This is your plug-in version number. Change it here only.
    # Version number symbols usable in C++ can be found in
    # source/version.h and ${PROJECT_BINARY_DIR}/projectversion.h.
    VERSION 1.0.0.0 
    DESCRIPTION "K_wire_2 VST 3 Plug-in"
)

set(SMTG_CREATE_PLUGIN_LINK 0)
set(SMTG_VSTGUI_ROOT "${vst3sdk_SOURCE_DIR}")
set(SMTG_RUN_VST_VALIDATOR OFF)

add_subdirectory(${vst3sdk_SOURCE_DIR} ${PROJECT_BINARY_DIR}/vst3sdk)
smtg_enable_vst3_sdk()

smtg_add_vst3plugin(${PROJECT_NAME}
    source/version.h
    source/Kwire2cids.h
    source/Kwire2processor.h
    source/Kwire2processor.cpp
    source/Kwire2controller.h
    source/Kwire2controller.cpp
    source/Kwire2entry.cpp
	includes/constants.h
	includes/parameters.h
	includes/LookupTable.h
	includes/CustomParameter.h
	includes/ParamPointQueue.h
	includes/TPTFilter.h
	includes/TPTSVF.h
)

# Add the includes directory to the target
target_include_directories(${PROJECT_NAME} PRIVATE includes)

#- VSTGUI Wanted ----
if(SMTG_ENABLE_VSTGUI_SUPPORT)
    target_sources(${PROJECT_NAME}
        PRIVATE
            resource/Kwire2editor.uidesc
    )
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            vstgui_support
    )
    smtg_target_add_plugin_resources(${PROJECT_NAME}
        RESOURCES
            "resource/Kwire2editor.uidesc"
    )
endif(SMTG_ENABLE_VSTGUI_SUPPORT)
# -------------------

smtg_target_add_plugin_snapshots (${PROJECT_NAME}
    RESOURCES
        resource/CE387CEA599755348F1237E15256B1C7_snapshot.png
        resource/CE387CEA599755348F1237E15256B1C7_snapshot_2.0x.png
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        sdk
)

smtg_target_configure_version_file(${PROJECT_NAME})

if(SMTG_MAC)
    smtg_target_set_bundle(${PROJECT_NAME}
        BUNDLE_IDENTIFIER com.laserbrain.kwire2
        COMPANY_NAME "Laser Brain"
    )
    smtg_target_set_debug_executable(${PROJECT_NAME}
        "/Applications/VST3PluginTestHost.app"
        "--pluginfolder;$(BUILT_PRODUCTS_DIR)"
    )
elseif(SMTG_WIN)
    target_sources(${PROJECT_NAME} PRIVATE 
        resource/win32resource.rc
    )
    if(MSVC)
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

        smtg_target_set_debug_executable(${PROJECT_NAME}
            "C:/ProgramData/Ableton/Live 12 Suite/Program/Ableton Live 12 Suite.exe"
            "--pluginfolder \"$(OutDir)/\""
        )
    endif()
endif(SMTG_MAC)
