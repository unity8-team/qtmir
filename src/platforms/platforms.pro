TEMPLATE = subdirs

SUBDIRS += base ubuntulegacy ubuntu

ubuntulegacy.depends = base
ubuntu.depends = base
