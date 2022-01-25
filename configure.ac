dnl **** Initialization ****

AC_INIT([libTAS], [1.4.2], [clement.gallet@ens-lyon.org])
AM_INIT_AUTOMAKE([foreign subdir-objects])
AC_PROG_CXX
AC_CONFIG_SRCDIR([src/program/main.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
 Makefile
 src/program/Makefile
 src/library/Makefile
])
AC_CANONICAL_HOST
AM_SILENT_RULES([yes])

dnl **** Configure options ****

AC_ARG_ENABLE([hud], AS_HELP_STRING([--disable-hud], [Disable the HUD]))
AC_ARG_WITH([i386], AS_HELP_STRING([--with-i386],[Build libTAS with support for 32-bit executables]))
AC_ARG_ENABLE([i386-lib], AS_HELP_STRING([--enable-i386-lib],[Build 32-bit version of libTAS library]))

save_CXX="$CXX"

case $host in
  x86_64*|amd64*)
    if [test "x$with_i386" = "xyes"] || [test "x$enable_i386_lib" = "xyes"]
    then
       AC_LANG_PUSH([C++])
       CXX="$CXX -m32"
       AC_MSG_CHECKING(for i386 cross-compiling)
       AC_LINK_IFELSE([AC_LANG_PROGRAM()],[AC_MSG_RESULT(yes); cxx_m32=yes],[AC_MSG_RESULT(no)])
       test "x$cxx_m32" = "xyes" || AC_MSG_ERROR([Cannot build a 32-bit program, you need to install 32-bit development libraries.])
       CXX="$save_CXX"
       AC_LANG_POP([C++])
    fi
    ;;
esac

AM_CONDITIONAL([BUILD32LIB], [test "x$with_i386" = "xyes" || test "x$enable_i386_lib" = "xyes"])
AM_CONDITIONAL([BUILD32LIBONLY], [test "x$enable_i386_lib" = "xyes"])

AC_ARG_ENABLE([release-build], AS_HELP_STRING([--enable-release-build], [Build a release]))
AC_ARG_ENABLE([build-date], AS_HELP_STRING([--disable-build-date], [Do not embed build date in executable]))

dnl **** Check for libraries and headers for libTAS program ****

PROGRAM_LIBS=
LIBRARY_LIBS=
LIBRARY32_LIBS=

AS_IF([test "x$enable_i386_lib" != "xyes"], [
    PKG_CHECK_MODULES([QT5], [Qt5Core >= 5.6.0, Qt5Widgets])
    AC_SUBST([QT5_CFLAGS])
    AC_SUBST([QT5_LIBS])
    AC_CHECK_PROGS(MOC, [moc moc-qt5])

    AC_CHECK_PROG([FFMPEG], [ffmpeg])

    AC_CHECK_HEADERS([xcb/xcb.h xcb/randr.h xcb/xcb_keysyms.h xcb/xkb.h], [], AC_MSG_ERROR(Some xcb headers are missing!))
    AC_SEARCH_LIBS([XKeysymToString], [X11], [], [AC_MSG_ERROR(The x11 library is required!)])
    AC_SEARCH_LIBS([xcb_connect], [xcb], [], [AC_MSG_ERROR(The xcb library is required!)])
    AC_SEARCH_LIBS([xcb_xkb_use_extension], [xcb-xkb], [], [AC_MSG_ERROR(The xcb-xkb library is required!)])
    AC_SEARCH_LIBS([xcb_key_symbols_alloc], [xcb-keysyms], [], [AC_MSG_ERROR(The xcb-keysyms library is required!)])

    AC_SEARCH_LIBS([pthread_create], [pthread], [], [AC_MSG_ERROR(The pthread library is required!)])

    PKG_CHECK_MODULES([LIBLUA], [lua54],, [
        PKG_CHECK_MODULES([LIBLUA], [lua53])
    ])

    AC_SUBST([LIBLUA_CFLAGS])
    AC_SUBST([LIBLUA_LIBS])
    
    PROGRAM_LIBS=$LIBS
    LIBS=
])

dnl **** Check for libraries and headers for libTAS library ****

AC_SEARCH_LIBS([dlopen], [dl dld])
AC_CHECK_HEADERS([X11/Xlib.h X11/Xlib-xcb.h], [], AC_MSG_ERROR(Some X11 headers are missing!))

AC_SEARCH_LIBS([XOpenDisplay], [X11], [], [AC_MSG_ERROR(The x11 library is required!)])
AC_SEARCH_LIBS([XGetXCBConnection], [X11-xcb], [], [AC_MSG_ERROR(The x11-xcb library is required!)])

AC_CHECK_HEADERS([SDL2/SDL.h], [], AC_MSG_ERROR(The SDL2 headers are required!))
AC_CHECK_HEADERS([libudev.h], [], AC_MSG_ERROR(The udev headers are required!))

AC_CHECK_HEADER([xcb/randr.h], [AC_DEFINE([LIBTAS_HAS_XCB_RANDR], [1], [Extension xcb randr is present])])

AC_CHECK_HEADERS([pthread.h], [], [AC_MSG_ERROR(The pthread header is required!)])
AC_SEARCH_LIBS([pthread_join], [pthread], [], [AC_MSG_ERROR(The pthread library is required!)])

AC_CHECK_HEADERS([alsa/asoundlib.h], [], [AC_MSG_ERROR(The alsa headers are required!)])
AC_SEARCH_LIBS([snd_pcm_open], [asound], [], [AC_MSG_ERROR(The asound library is required!)])

AC_SUBST(have_libswresample, no)
PKG_CHECK_MODULES(LIBSWRESAMPLE, [libswresample], AC_SUBST(have_libswresample, yes), AC_MSG_WARN(Cannot find libswresample using pkg-config))

AS_IF([test "x$have_libswresample" != "xyes"], [
    AC_CHECK_HEADERS([libswresample/swresample.h], [], [AC_MSG_ERROR(The swresample header is required!)])
    AC_SEARCH_LIBS([swr_init], [swresample], [], [AC_MSG_ERROR(The swresmaple library is required!)])
    AC_SUBST(LIBSWRESAMPLE_CFLAGS, "")
])

AS_IF([test "x$have_libswresample" = "xyes"], [
    AC_SUBST(LIBSWRESAMPLE_CFLAGS)
])

AS_IF([test "x$enable_hud" != "xno"], [
    CPPFLAGS='-I/usr/include/freetype2'
    AC_CHECK_HEADERS([fontconfig/fontconfig.h ft2build.h], [], [enable_hud=no])
    AC_SEARCH_LIBS([FcInitLoadConfigAndFonts], [fontconfig], [], [enable_hud=no])
    AC_SEARCH_LIBS([FT_Open_Face], [freetype], [], [enable_hud=no])
])

LIBRARY_LIBS=$LIBS
LIBS=

dnl **** Check for 32-bit libraries for libTAS library ****

save_CXX="$CXX"

case $host in
  x86_64*|amd64*)
    if [test "x$with_i386" = "xyes"] || [test "x$enable_i386_lib" = "xyes"]
    then

       AC_LANG_PUSH([C++])
       CXX="$CXX -m32"

       dnl Look at different functions to avoid caching
       AC_SEARCH_LIBS([dlclose], [dl dld])
       AC_SEARCH_LIBS([XCloseDisplay], [X11], [], [AC_MSG_ERROR(The x11 library is required!)])
       unset ac_cv_search_XGetXCBConnection
       AC_SEARCH_LIBS([XGetXCBConnection], [X11-xcb], [], [AC_MSG_ERROR(The x11-xcb library is required!)])
       AC_SEARCH_LIBS([pthread_exit], [pthread], [], [AC_MSG_ERROR(The pthread library is required!)])
       AC_SEARCH_LIBS([snd_pcm_close], [asound], [], [AC_MSG_ERROR(The asound library is required!)])

       AS_IF([test "x$enable_hud" != "xno"], [
       	  AC_SEARCH_LIBS([FcConfigAppFontAddFile], [fontconfig], [], [enable_hud=no])
       	  AC_SEARCH_LIBS([FT_Bitmap_Convert], [freetype], [], [enable_hud=no])
       ])

       LIBRARY32_LIBS=$LIBS
       LIBS=

       CXX="$save_CXX"
       AC_LANG_POP([C++])
    fi
    ;;
esac

AS_IF([test "x$enable_hud" != "xno"], [
   AC_DEFINE([LIBTAS_ENABLE_HUD], [1], [HUD is enabled])
   AC_MSG_NOTICE([HUD is enabled])
])

dnl **** Export date and commit ****

AS_IF([test "x$enable_release_build" != "xyes"], [
    dnl Get the latest abbreviated commit hash of the working branch
    git_commit_hash=`git log -1 --format=%h`
    AC_DEFINE_UNQUOTED([LIBTAS_INTERIM_COMMIT], ["$git_commit_hash"], [Git commit hash])
    AC_MSG_NOTICE([git commit hash is $git_commit_hash])

    AS_IF([test "x$enable_build_date" != "xno"], [
      dnl Add commit date as macro
      git_commit_date=`git log -1 --format=%as`
      AC_DEFINE_UNQUOTED([LIBTAS_INTERIM_DATE], ["$git_commit_date"], [Git commit date])
      AC_MSG_NOTICE([git commit date is $git_commit_date])
    ])
])

AC_SUBST([PROGRAM_LIBS])
AC_SUBST([LIBRARY_LIBS])
AC_SUBST([LIBRARY32_LIBS])

AC_OUTPUT
