#!/bin/bash

buildRoot=""

if [[ $# != 1 ]] ; then
    echo "Usage: $0 <path_to_build_directory>";
    exit 1;
else
    buildRoot="$1";
fi

export QT_QPA_PLATFORM="mirserver"
export DESKTOP_SESSION="unity8-mir"
export QT_QPA_PLATFORM_PLUGIN_PATH=$buildRoot/src/platforms/mirserver
export QML2_IMPORT_PATH=$buildRoot/src/modules

# check required files
if [[ ! -f $QML2_IMPORT_PATH/Unity/Application/libunityapplicationplugin.so ]] ; then
    echo "Error, $QML2_IMPORT_PATH/Unity/Application/libunityapplicationplugin.so not found, check build directory"
    exit 2;
fi
if [[ ! -f $QT_QPA_PLATFORM_PLUGIN_PATH/libqpa-mirserver.so ]] ; then
    echo "Error, $QT_QPA_PLATFORM_PLUGIN_PATH/libqpa-mirserver.so not found, check build directory"
    exit 2;
fi

qmlscene qml-demo-shell.qml
