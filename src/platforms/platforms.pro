TEMPLATE = subdirs

SUBDIRS += base ubuntucommon

!mirclient:!mirserver {
CONFIG += hybris
}

hybris {
SUBDIRS += ubuntu
}
mirclient {
SUBDIRS += ubuntumirclient
}
mirserver {
SUBDIRS += ubuntumirserver
}

ubuntulegacy.depends = base
ubuntucommon.depends = base
ubuntu.depends = ubuntucommon
ubuntumirclient.depends = ubuntucommon
ubuntumirserver.depends = ubuntucommon
