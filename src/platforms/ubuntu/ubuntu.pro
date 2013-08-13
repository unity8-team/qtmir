TEMPLATE = subdirs

!mirclient:!mirserver {
CONFIG += hybris
}

hybris {
SUBDIRS += ubuntu
}

SUBDIRS += ubuntucommon ubuntumir

ubuntu.depends = ubuntucommon
ubuntumir.depends = ubuntucommon
