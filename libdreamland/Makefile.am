DUMMY_LIBS =
LD_DLLGLUE =
CC_DLLGLUE =
MY_CXXLD = $(CXX)

lib_LTLIBRARIES = libdreamland.la 

AM_LDFLAGS = $(LD_DLLGLUE)
AM_CXXFLAGS = $(CC_DLLGLUE)
CXXLD = $(MY_CXXLD)


SUBDIRS =  \
regex      \
lang       \
alloc      \
util       \
l10n       \
xml        \
flags      \
gref       \
process    \
plugin     \
scheduler  \
io         \
fenia      

include $(top_srcdir)/libdreamland/Makefile.inc
INCLUDES = $(INCLUDES_DREAMLAND)

#####################################################
# dreamland shared library
#
libdreamland_la_SOURCES =

libdreamland_la_LIBADD = \
fenia/libfenia.la \
xml/libxml.la \
lang/liblang.la \
l10n/libl10n.la \
io/libio.la \
plugin/libplugin.la \
util/libutil.la \
scheduler/libscheduler.la \
regex/libregex.la \
alloc/liballoc.la \
gref/libgref.la \
flags/libflags.la \
process/libprocess.la \
$(DUMMY_LIBS) @BDB_LIBS@ @MCCP_LIBS@ 

CLEANFILES = $(DUMMY_LIBS)
