lib_LTLIBRARIES = libfeniaroot.la 

libfeniaroot_la_SOURCES = \
ceval.cpp \
cfindref.cpp \
characterwrapper.cpp \
objectwrapper.cpp \
roomwrapper.cpp \
impl.cpp \
root.cpp \
schedulerwrapperext.cpp \
validatetask.cpp \
pluginnativeimpl.cpp \
wrappermanager.cpp \
wrappersplugin.cpp \
idcontainerext.cpp \
regcontainerext.cpp \
feniastringext.cpp \
reglistext.cpp \
wrap_utils.cpp

libfeniaroot_la_MOC = \
characterwrapper.h \
objectwrapper.h \
roomwrapper.h \
root.h \
wrappersplugin.h 

libfeniaroot_la_LIBADD = \
../runtime/libruntime.la			 

include $(top_srcdir)/src/Makefile.inc
include $(top_srcdir)/plug-ins/Makefile.inc
INCLUDES = \
$(INCLUDES_SRC) \
-I$(top_srcdir)/plug-ins/runtime
