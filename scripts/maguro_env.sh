#!/bin/bash -i

NAME='Maguro'

export PATH=/opt/qt5/bin${PATH:+:$PATH}
export LD_LIBRARY_PATH=/opt/qt5/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
export PKG_CONFIG_PATH=/opt/qt5/lib/pkgconfig${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}
export CMAKE_MODULE_PATH=/opt/qt5/lib/cmake${CMAKE_MODULE_PATH:+:$CMAKE_MODULE_PATH}
export QML_IMPORT_PATH=/opt/qt5/imports
export QT_PLUGIN_PATH=/opt/qt5/plugins
export QT_FONT_PATH=/opt/qt5/lib/fonts
export QT_QPA_PLATFORM_PLUGIN_PATH=/home/ufa/qt5/platforms
export QT_QPA_PLATFORM=hybris

TMP=`mktemp -t bashrc.XXXXXXXX`
echo PS1=\'[$NAME] $PS1\' >> $TMP
echo "alias ll=\"ls -l\"" >> $TMP
SHELL_OPTIONS="--init-file $TMP"

echo Entering $NAME shell.
/bin/bash $SHELL_OPTIONS
echo Leaving $NAME shell.

if test ! -z "$TMP"
then
  rm $TMP
fi
