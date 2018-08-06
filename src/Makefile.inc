include $(top_srcdir)/libdreamland/Makefile.inc
INCLUDES_SRC = \
$(INCLUDES_DREAMLAND) \
-I$(srcdir) \
-I$(top_builddir)/src \
-I$(top_builddir) \
-I$(top_srcdir)/src \
-I$(top_srcdir)/src/core \
-I$(top_srcdir)/src/fenia \
-I$(top_srcdir)/src/io \
-I$(top_srcdir)/src/xparser \
-I$(top_srcdir)/src/xformat
