dnl Process this file with autoconf to produce a configure script.

AC_INIT([dreamland], [0.1])
AC_CONFIG_SRCDIR(Makefile.am)
AC_CONFIG_AUX_DIR(admin)
AC_CANONICAL_SYSTEM
AM_CONFIG_HEADER(config.h)
AC_PREFIX_DEFAULT(`pwd`)
AM_INIT_AUTOMAKE
   
dnl Assign global compiler and linker options (currently for unix/linux only)
CPPFLAGS="$CPPFLAGS -Wall -D_NIX_ -Wno-deprecated"
CFLAGS="$CFLAGS -Wall -D_NIX_ -Wno-deprecated"
LDFLAGS="$LDFLAGS -Wl,-no-undefined -lstdc++"

dnl Check for additional compiler/linker options
AC_CHECK_BITNESS
AC_CHECK_OPTIMIZATION


AM_DISABLE_STATIC
AC_LIBTOOL_WIN32_DLL
AM_PROG_LIBTOOL

dnl Checks for programs.

AC_EXEEXT
AC_PROG_CC
AC_PROG_CXX

AC_LANG_CPLUSPLUS

AM_PROG_LEX
AC_PROG_YACC
AC_PROG_AWK
AC_PROG_RANLIB

dnl Checks for header files.
AC_HEADER_STDC
AC_HEADER_DIRENT
AC_HEADER_TIME
AC_CHECK_HEADERS( unistd.h )
AC_CHECK_HEADERS( netinet/in.h arpa/inet.h sys/socket.h semaphore.h )
AC_CHECK_HEADERS( dlfcn.h sys/types.h )


dnl Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_DLSIZE_T
AC_TYPE_SIGNAL
AC_TYPE_SIZE_T

AC_CHECK_TIME_T

dnl Checks for library functions.
AC_CHECK_BDB

AC_CHECK_MINGW

AC_FUNC_CLOSEDIR_VOID
AC_FAKE_CLOSEDIR_VOID

AC_CHECK_FUNCS( socket )
AC_CHECK_FUNCS( accept )

dnl Checks for libraries.
AC_CHECK_LIB(m, main)
AC_CHECK_LIB(dl, dlopen)

AC_CHECK_THREADS
AC_CHECK_MCCP

dnl Creating installation tree
AC_CREATE_DIR(bin)
AC_CREATE_DIR(etc)
AC_CREATE_FILE(etc/planescape.xml)
AC_CREATE_DIR(lib)
AC_CREATE_DIR(libexec)
AC_CREATE_DIR(libexec/plugins)
AC_COPY_FILE(etc/plugin.xml.template, libexec/plugin.xml)
AC_CREATE_DIR(var/db)
AC_CREATE_DIR(var/db/admin)
AC_CREATE_DIR(var/db/boards)
AC_CREATE_DIR(var/db/bugs)
AC_CREATE_DIR(var/db/feniadb)
AC_CREATE_DIR_HASH(var/db/plrs)
AC_CREATE_DIR_HASH(var/db/plralias)
AC_CREATE_DIR_HASH(var/db/plrobjs)
AC_CREATE_DIR_HASH(var/db/plrvars)
AC_CREATE_DIR(var/log)
AC_CREATE_DIR(var/mail)
AC_CREATE_DIR(var/mail/spool)
AC_CREATE_DIR(var/run)


dnl Configuring and building meta object compiler
AC_CONFIG_COMMANDS([libdreamland/moc/moc], [
    AC_MSG_NOTICE([Configuring meta object compiler...])
    ac_moc_dir_pop=`pwd`
    cd $ac_dir
    ac_moc_cmd="/bin/sh $ac_srcdir/configure --build=$ac_moc_build"
    AC_MSG_NOTICE([Running command $ac_moc_cmd]);
    eval $ac_moc_cmd || exit 1
    AC_MSG_NOTICE([Building meta object compiller...])
    make || exit 1
    cd $ac_moc_dir_pop
], [ac_moc_build=$build])

AC_OUTPUT( \
Makefile \
libdreamland/Makefile \
libdreamland/fenia/Makefile \
libdreamland/io/Makefile \
libdreamland/lang/Makefile \
libdreamland/moc/Makefile \
libdreamland/plugin/Makefile \
libdreamland/scheduler/Makefile \
libdreamland/util/Makefile \
libdreamland/regex/Makefile \
libdreamland/xml/Makefile \
libdreamland/l10n/Makefile \
libdreamland/alloc/Makefile \
libdreamland/flags/Makefile \
libdreamland/gref/Makefile \
libdreamland/process/Makefile \
src/Makefile \
src/core/Makefile \
src/fenia/Makefile \
src/io/Makefile \
src/xparser/Makefile \
src/xformat/Makefile \
plug-ins/Makefile \
plug-ins/feniaroot/Makefile \
plug-ins/runtime/Makefile \
)

echo ""
echo ""

