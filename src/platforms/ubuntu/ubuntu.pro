TEMPLATE = subdirs

SUBDIRS += ubuntucommon ubuntumir

!mirclient:!mirserver {
CONFIG += hybris
}

hybris {
SUBDIRS += ubuntu
}

ubuntu.depends = ubuntucommon
ubuntumir.depends = ubuntucommon
