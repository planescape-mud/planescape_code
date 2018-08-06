lib_LTLIBRARIES = libplanescape_impl.la

bin_PROGRAMS = planescape

CXXLD = $(CXX)

SUBDIRS = \
core \
fenia \
io \
xparser \
xformat

include $(top_srcdir)/src/Makefile.inc
INCLUDES = $(INCLUDES_SRC) 


#####################################################
# planescape implementation library
#
libplanescape_impl_la_SOURCES = \
impl.cpp \
memutils.cpp \
structs.cpp 

libplanescape_impl_la_LIBADD = \
$(top_builddir)/libdreamland/libdreamland.la \
core/libcore.la \
fenia/libfenia.la \
io/libio.la \
xparser/libxparser.la \
xformat/libxformat.la \
-lcrypt

#####################################################
# planescape binary
#
planescape_SOURCES = main.cpp

planescape_LDADD = \
libplanescape_impl.la \
$(top_builddir)/libdreamland/libdreamland.la 


