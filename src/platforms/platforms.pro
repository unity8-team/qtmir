TEMPLATE = subdirs

SUBDIRS += base ubuntucommon

!mirclient:!mirserver {
CONFIG += hybris
}

hybris {
SUBDIRS += ubuntu
}
mirclient {
SUBDIRS += ubuntumir
}
mirserver {
SUBDIRS += ubuntumirserver
}

ubuntulegacy.depends = base
ubuntucommon.depends = base
ubuntu.depends = ubuntucommon
ubuntumir.depends = ubuntucommon
ubuntumirserver.depends = ubuntucommon
