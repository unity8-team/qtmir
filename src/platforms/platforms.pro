TEMPLATE = subdirs

SUBDIRS += base ubuntu

ubuntulegacy.depends = base
ubuntu.depends = base
