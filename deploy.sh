#!/bin/bash -i

adb push plugins/platforms/libqhybris.so /data/ubuntu/opt/qt5/plugins/platforms
adb push plugins/platforms/libqhybrislegacy.so /data/ubuntu/opt/qt5/plugins/platforms
adb push examples/qmlscene-hybris /data/ubuntu/opt/qt5/bin
adb push examples/MovingLogo.qml /data/ubuntu/opt/qt5/examples
adb push examples/WarpingLogo.qml /data/ubuntu/opt/qt5/examples
adb push examples/Input.qml /data/ubuntu/opt/qt5/examples
adb push examples/logo.png /data/ubuntu/opt/qt5/examples
adb push examples/noise.png /data/ubuntu/opt/qt5/examples
