cmake_minimum_required(VERSION 3.14.0)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13 CACHE STRING "")

set(vst3sdk_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../externals/vst3sdk")

if(NOT vst3sdk_SOURCE_DIR)
    message(FATAL_ERROR "Path to VST3 SDK is empty!")
endif()

project(K_wire_2
    VERSION 1.0.0.0 
    DESCRIPTION "K-wire 2 VST 3 Plug-in"
)

set(SMTG_CREATE_PLUGIN_LINK 0)

set(SMTG_VSTGUI_ROOT "${vst3sdk_SOURCE_DIR}")

include_directories(".")

add_subdirectory(${vst3sdk_SOURCE_DIR} ${PROJECT_BINARY_DIR}/vst3sdk)
smtg_enable_vst3_sdk()

smtg_add_vst3plugin(${PROJECT_NAME}
    source/version.h
    source/myplugincids.h
    source/mypluginprocessor.h
    source/mypluginprocessor.cpp
    source/myplugincontroller.h
    source/myplugincontroller.cpp
    source/mypluginentry.cpp
	includes/constants.h
	includes/parameters.h
	includes/CustomParameter.h
)

#- VSTGUI Wanted ----
if(SMTG_ENABLE_VSTGUI_SUPPORT)
    target_sources(${PROJECT_NAME}
        PRIVATE
            resource/myplugineditor.uidesc
    )
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            vstgui_support
    )
    smtg_target_add_plugin_resources(${PROJECT_NAME}
        RESOURCES
            "resource/myplugineditor.uidesc"
    )
endif(SMTG_ENABLE_VSTGUI_SUPPORT)
# -------------------

smtg_target_add_plugin_snapshots (${PROJECT_NAME}
    RESOURCES
        resource/A718A160C6415CB1B5798506651C3CB1_snapshot.png
        resource/A718A160C6415CB1B5798506651C3CB1_snapshot_2.0x.png
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
            "$(ProgramW6432)/Steinberg/VST3PluginTestHost/VST3PluginTestHost.exe"
            "--pluginfolder \"$(OutDir)/\""
        )
    endif()
endif(SMTG_MAC)
