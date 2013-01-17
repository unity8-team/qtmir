load(configure)
load(qt_parts)

# FIXME(loicm) I don't get why qmake automatically detects src/ but not
#     examples/. Doing it that way makes make warn about the generated Makefile.
TEMPLATE = subdirs
SUBDIRS += examples
