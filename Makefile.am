SUBDIRS = src/library

if !BUILD32LIBONLY
SUBDIRS += src/program

# The desktop files
desktopdir = $(datadir)/applications
desktop_DATA = libTAS.desktop

# The application icon
appicondir = $(datadir)/icons/hicolor/scalable/apps
appicon_DATA = libTAS.svg
endif
