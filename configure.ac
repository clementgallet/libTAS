dnl **** Initialization ****

AC_INIT([libTAS], [1.3.4], [clement.gallet@ens-lyon.org])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_PROG_CXX
AC_CONFIG_SRCDIR([src/program/main.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
 Makefile
 src/program/Makefile
 src/library/Makefile
])
AC_CANONICAL_HOST

dnl **** Configure options ****

AC_ARG_ENABLE([hud], AS_HELP_STRING([--disable-hud], [Disable the HUD]))
AC_ARG_ENABLE([i386], AS_HELP_STRING([--enable-i386],[Build libTAS for 32-bit executables]))

case $host in
  x86_64*|amd64*)
    if test "x$enable_i386" == "xyes"
    then
       AC_LANG_PUSH([C++])
       CXX="$CXX -m32"
       AC_MSG_CHECKING(for i386 cross-compiling)
       AC_LINK_IFELSE([AC_LANG_PROGRAM()],[AC_MSG_RESULT(yes); cxx_m32=yes],[AC_MSG_RESULT(no)])
       test "x$cxx_m32" == "xyes" || AC_MSG_ERROR([Cannot build a 32-bit program, you need to install 32-bit development libraries.])
       AC_LANG_POP([C++])
       AC_SUBST(LIBSUFFIX, "32")
    else
       AC_SUBST(LIBSUFFIX, "")
    fi
    ;;
esac
AM_CONDITIONAL([BUILD32LIB], [test "x$enable_i386" == "xyes"])

AC_ARG_ENABLE([release-build], AS_HELP_STRING([--enable-release-build], [Build a release]))

dnl **** Check for libraries and headers for libTAS program ****

AS_IF([test "x$enable_i386" != "xyes"], [
    PKG_CHECK_MODULES([QT5], [Qt5Core >= 5.6.0, Qt5Widgets])
    AC_SUBST([QT5_CFLAGS])
    AC_SUBST([QT5_LIBS])
    AC_CHECK_PROGS(MOC, [moc])
    AC_CHECK_HEADERS([xcb/xcb.h xcb/randr.h xcb/xcb_cursor.h xcb/xcb_keysyms.h xcb/xkb.h], [], AC_MSG_ERROR(Some xcb headers are missing!))
    AC_CHECK_LIB(xcb, xcb_connect, [], AC_MSG_ERROR(The xcb library is required!))
    AC_CHECK_LIB(xcb-xkb, xcb_xkb_use_extension, [], AC_MSG_ERROR(The xcb-xkb library is required!))
    AC_CHECK_LIB(xcb-cursor, xcb_cursor_context_new, [], AC_MSG_ERROR(The xcb-cursor library is required!))
    AC_CHECK_LIB(xcb-keysyms, xcb_key_symbols_alloc, [], AC_MSG_ERROR(The xcb-keysyms library is required!))
    AC_CHECK_PROG([FFMPEG], [ffmpeg])
])

dnl **** Check for libraries and headers for libTAS library ****

AC_SEARCH_LIBS([dlopen], [dl dld])
AC_CHECK_HEADERS([X11/Xlib.h X11/Xlib-xcb.h], [], AC_MSG_ERROR(Some X11 headers are missing!))

AC_CHECK_LIB(X11, XOpenDisplay, [], AC_MSG_ERROR(The x11 library is required!))
AC_CHECK_LIB(X11-xcb, XGetXCBConnection, [], AC_MSG_ERROR(The x11-xcb library is required!))

AC_CHECK_HEADERS([SDL2/SDL.h], [], AC_MSG_ERROR(The SDL2 headers are required!))
AC_CHECK_HEADERS([libudev.h], [], AC_MSG_ERROR(The udev headers are required!))

AC_CHECK_HEADER([X11/extensions/xf86vmode.h], [AC_DEFINE([LIBTAS_HAS_XF86VMODE], [1], [Extension xf86vmode is present])], [], [#include <X11/Xlib.h>])
AC_CHECK_HEADER([X11/extensions/Xrandr.h], [AC_DEFINE([LIBTAS_HAS_XRANDR], [1], [Extension Xrandr is present])])
AC_CHECK_HEADER([X11/extensions/XInput2.h], [AC_DEFINE([LIBTAS_HAS_XINPUT], [1], [Extension XInput2 is present])])

AC_CHECK_HEADERS([pthread.h], [], AC_MSG_ERROR(The pthread header is required!))
AC_CHECK_LIB(pthread, pthread_create, [], AC_MSG_ERROR(The pthread library is required!))

AC_CHECK_HEADERS([alsa/asoundlib.h], [], AC_MSG_ERROR(The alsa headers are required!))
AC_CHECK_LIB(asound, snd_pcm_open, [], AC_MSG_ERROR(The asound library is required!))

AC_CHECK_HEADERS([libswresample/swresample.h libavutil/avutil.h], [], AC_MSG_ERROR(The swresample headers are required!))
AC_CHECK_LIB(swresample, swr_init, [], AC_MSG_ERROR(The swresmaple library is required!))
AC_CHECK_LIB(avutil, av_opt_set_sample_fmt, [], AC_MSG_ERROR(The avutil library is required!))

AS_IF([test "x$enable_hud" != "xno"], [
	CPPFLAGS='-I/usr/include/freetype2'
	AC_CHECK_HEADERS([fontconfig/fontconfig.h ft2build.h], [], [enable_hud=no])
	AC_CHECK_LIB(fontconfig, FcInitLoadConfigAndFonts, [], [enable_hud=no])
	AC_CHECK_LIB(freetype, FT_Open_Face, [], [enable_hud=no])
])

AS_IF([test "x$enable_hud" == "xno"], [
	AC_MSG_WARN(HUD is disabled)
])

AS_IF([test "x$enable_release_build" != "xyes"], [
    dnl Get the latest abbreviated commit hash of the working branch
    git_commit_hash=`git log -1 --format=%h`
    AC_DEFINE_UNQUOTED([LIBTAS_INTERIM_COMMIT], ["$git_commit_hash"], [Git commit])
    AC_MSG_NOTICE([git commit is $git_commit_hash])
    
    dnl Add build date as macro
    today=`date +%Y-%m-%d`
    AC_DEFINE_UNQUOTED([LIBTAS_INTERIM_DATE], ["$today"], [Git commit])
])

AC_OUTPUT
