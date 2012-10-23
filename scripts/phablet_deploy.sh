#!/bin/bash -i

adb push qt5_env.sh /data/ubuntu/home/ufa/qt5
adb push qt5_legacy_env.sh /data/ubuntu/home/ufa/qt5
adb push ../plugins/platforms/libqhybris.so /data/ubuntu/home/ufa/qt5/platforms
adb push ../plugins/platforms/libqhybrislegacy.so /data/ubuntu/home/ufa/qt5/platforms
adb push ../examples/hybris-qmlscene /data/ubuntu/home/ufa/qt5/examples
adb push ../examples/MovingLogo.qml /data/ubuntu/home/ufa/qt5/examples
adb push ../examples/WarpingLogo.qml /data/ubuntu/home/ufa/qt5/examples
adb push ../examples/Input.qml /data/ubuntu/home/ufa/qt5/examples
adb push ../examples/logo.png /data/ubuntu/home/ufa/qt5/examples
adb push ../examples/noise.png /data/ubuntu/home/ufa/qt5/examples
