TEMPLATE = subdirs

SUBDIRS += base ubuntucommon ubuntu ubuntumir ubuntumirserver

ubuntulegacy.depends = base
ubuntucommon.depends = base
ubuntu.depends = ubuntucommon
ubuntumir.depends = ubuntucommon
ubuntumirserver.depends = ubuntucommon
