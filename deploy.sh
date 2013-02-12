#!/bin/bash -i

adb push src/platforms/ubuntu/libqubuntu.so /data/ubuntu/usr/lib/arm-linux-gnueabihf/qt5/plugins/platforms
adb push src/platforms/ubuntulegacy/libqubuntulegacy.so /data/ubuntu/usr/lib/arm-linux-gnueabihf/qt5/plugins/platforms
adb push src/modules/application/libubuntuapplicationplugin.so /data/ubuntu/usr/lib/arm-linux-gnueabihf/qt5/imports/Ubuntu/Application
adb push src/modules/application/qmldir /data/ubuntu/usr/lib/arm-linux-gnueabihf/qt5/imports/Ubuntu/Application
adb push tests/qmlscene_ubuntu/qmlscene-ubuntu /data/ubuntu/usr/bin
adb push tests/clipboard/clipboard /data/ubuntu/usr/lib/arm-linux-gnueabihf/qt5/tests
adb push tests/Logo.qml /data/ubuntu/usr/lib/arm-linux-gnueabihf/qt5/tests
adb push tests/MovingLogo.qml /data/ubuntu/usr/lib/arm-linux-gnueabihf/qt5/tests
adb push tests/WarpingLogo.qml /data/ubuntu/usr/lib/arm-linux-gnueabihf/qt5/tests
adb push tests/Input.qml /data/ubuntu/usr/lib/arm-linux-gnueabihf/qt5/tests
adb push tests/Application.qml /data/ubuntu/usr/lib/arm-linux-gnueabihf/qt5/tests
adb push tests/Fullscreen.qml /data/ubuntu/usr/lib/arm-linux-gnueabihf/qt5/tests
adb push tests/logo.png /data/ubuntu/usr/lib/arm-linux-gnueabihf/qt5/tests
adb push tests/noise.png /data/ubuntu/usr/lib/arm-linux-gnueabihf/qt5/tests
