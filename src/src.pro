TEMPLATE = subdirs

SUBDIRS += base hybrislegacy hybris

hybrislegacy.depends = base
hybris.depends = base
