#!/bin/bash -i

adb push src/platforms/hybris/libqhybris.so /data/ubuntu/usr/lib/arm-linux-gnueabihf/qt5/plugins/platforms
adb push src/platforms/hybrislegacy/libqhybrislegacy.so /data/ubuntu/usr/lib/arm-linux-gnueabihf/qt5/plugins/platforms
# FIXME(loicm) Not sure here to install that for now?
#adb push src/modules/application/libubuntuapplicationplugin.so /data/ubuntu/usr/???/Ubuntu/Application
#adb push src/modules/application/qmldir /data/ubuntu/usr/???/Ubuntu/Application
adb push examples/qmlscene-hybris /data/ubuntu/usr/bin
adb push examples/Logo.qml /data/ubuntu/usr/lib/arm-linux-gnueabihf/qt5/examples
adb push examples/MovingLogo.qml /data/ubuntu/usr/lib/arm-linux-gnueabihf/qt5/examples
adb push examples/WarpingLogo.qml /data/ubuntu/usr/lib/arm-linux-gnueabihf/qt5/examples
adb push examples/Input.qml /data/ubuntu/usr/lib/arm-linux-gnueabihf/qt5/examples
adb push examples/Application.qml /data/ubuntu/usr/lib/arm-linux-gnueabihf/qt5/examples
adb push examples/logo.png /data/ubuntu/usr/lib/arm-linux-gnueabihf/qt5/examples
adb push examples/noise.png /data/ubuntu/usr/lib/arm-linux-gnueabihf/qt5/examples
