TEMPLATE = subdirs

mirclient|mirserver{
SUBDIRS += ubuntumircommon
}

mirclient {
SUBDIRS += ubuntumirclient
}
mirserver {
SUBDIRS += ubuntumirserver
}

ubuntumirclient.depends = ubuntumircommon
ubuntumirserver.depends = ubuntumircommon

