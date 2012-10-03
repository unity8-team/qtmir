#!/bin/bash -i

adb push plugins/platforms/libqhybris.so /data/ubuntu/home/ufa/dev/staging/lib/qt5/plugins/platforms
adb push examples/hybris-qmlscene /data/ubuntu/home/ufa/dev/staging/lib/qt5/examples
adb push examples/MovingLogo.qml /data/ubuntu/home/ufa/dev/staging/lib/qt5/examples
adb push examples/WarpingLogo.qml /data/ubuntu/home/ufa/dev/staging/lib/qt5/examples
adb push examples/MultiTouch.qml /data/ubuntu/home/ufa/dev/staging/lib/qt5/examples
adb push examples/logo.png /data/ubuntu/home/ufa/dev/staging/lib/qt5/examples
adb push examples/noise.png /data/ubuntu/home/ufa/dev/staging/lib/qt5/examples
