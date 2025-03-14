#[[ 
SPDX-FileCopyrightText: 2025 Marius Dege <marius.dege@basyskom.com>
# SPDX-FileCopyrightText: 2024 Marius Dege <marius.dege@basyskom.com>
# SPDX-FileCopyrightText: 2024 basysKom GmbH

# SPDX-License-Identifier: LGPL-3.0-or-later
]]

cmake_minimum_required(VERSION 3.16)

# Project definition
set(PROJECT_NAME open62541devicedriver)
project(${PROJECT_NAME} VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_QML_GENERATE_QMLLS_INI ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(TARGET_NAME ${PROJECT_NAME}_bin)
# Enable the model tester if needed
option(ENABLE_MODEL_TESTER "Enable QAbstractItemModelTester for debugging" OFF)

if (ENABLE_MODEL_TESTER)
    message(STATUS "Model Tester enabled")
    add_definitions(-DENABLE_MODEL_TESTER)
    set(QT_COMPONENTS Quick Core Test)
else()
    set(QT_COMPONENTS Quick Core)
endif()

add_definitions(-DQT_NO_CAST_FROM_ASCII)

find_package(Qt6 6.7 REQUIRED COMPONENTS ${QT_COMPONENTS})

qt_standard_project_setup(REQUIRES 6.7)

qt_add_executable(${TARGET_NAME}
    main.cpp
)

# Enable Address Sanitizer if ENABLE_ASAN is ON
option(ENABLE_ASAN "Enable Address Sanitizer" OFF)

if (ENABLE_ASAN)
    message(STATUS "Address Sanitizer enabled")
    target_compile_options(${TARGET_NAME} PUBLIC -fsanitize=address)
    target_link_libraries(${TARGET_NAME} PRIVATE asan)
endif()

# Source files
set(SOURCES
    treeitem.h treeitem.cpp
    treemodel.h treemodel.cpp
    uanodeset.h uanodeset.cpp
    uanode.h uanode.cpp
    uanodesetparser.h uanodesetparser.cpp
    devicedrivercore.h devicedrivercore.cpp
    childitemfiltermodel.h childitemfiltermodel.cpp
    rootnodefiltermodel.h rootnodefiltermodel.cpp
    Util/Utils.h Util/Utils.cpp
)

# QML files
set(QML_FILES
    Main.qml
    Components/LabeledDropdown.qml
    Components/LabeledInput.qml
    Components/MethodDelegate.qml
    Components/NodeDelegateBase.qml
    Components/NodeSetupDialog.qml
    Components/ObjectDelegate.qml
    Components/RootObjectDelegate.qml
    Components/RootObjectListDelegate.qml
    Components/VariableDelegate.qml
    Screens/NodeSelectionGridScreen.qml
    Components/IconButton.qml
    Components/DeviceNodesTreeDeletage.qml
    Components/ChildNodesTreeDelegate.qml
    Components/LabeledCheckbox.qml
    Components/LoadingSpinner.qml
)

# Resources
set(RESOURCES
    Icons/setup.svg
    Icons/arrow.svg
    Icons/help.svg
    Icons/method.svg
    Icons/object.svg
    Icons/root.svg
    Icons/variable.svg
)

qt_add_qml_module(${TARGET_NAME}
    URI open62541devicedriver
    VERSION 1.0
    QML_FILES ${QML_FILES}
    SOURCES ${SOURCES}
    RESOURCES ${RESOURCES}
)

# Add subdirectory for mstch
add_subdirectory(mstch)
include_directories(${mstch_INCLUDE_DIR})

# Set macOS and Windows bundle properties
set_target_properties(${TARGET_NAME} PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(${TARGET_NAME}
    PRIVATE Qt6::Quick
    mstch
)

# Link Qt6::Test only if ENABLE_MODEL_TESTER is ON
if (ENABLE_MODEL_TESTER)
    target_link_libraries(${TARGET_NAME} PRIVATE Qt6::Test)
endif()

include(GNUInstallDirs)
install(TARGETS ${TARGET_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
