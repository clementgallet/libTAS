bin_PROGRAMS = libTAS

libTAS_QTSOURCES = \
    GameEvents.h \
    GameLoop.h \
    ramsearch/MemScanner.h \
    ui/AnnotationsWindow.h \
    ui/AutoSaveWindow.h \
    ui/ControllerAxisWidget.h \
    ui/ControllerTabWindow.h \
    ui/ControllerWidget.h \
    ui/EncodeWindow.h \
    ui/ExecutableWindow.h \
    ui/GameInfoWindow.h \
    ui/GameSpecificWindow.h \
    ui/InputEditorModel.h \
    ui/InputEditorView.h \
    ui/InputEditorWindow.h \
    ui/InputWindow.h \
    ui/KeyPressedDialog.h \
    ui/MainWindow.h \
    ui/OsdWindow.h \
    ui/PointerScanModel.h \
    ui/PointerScanWindow.h \
    ui/RamSearchModel.h \
    ui/RamSearchWindow.h \
    ui/RamWatchEditWindow.h \
    ui/RamWatchModel.h \
    ui/RamWatchWindow.h \
    ui/TimeTraceModel.h \
    ui/TimeTraceWindow.h

libTAS_MOCSOURCES = $(libTAS_QTSOURCES:.h=_moc.cpp)

libTAS_SOURCES = \
    AutoSave.cpp \
    Config.cpp \
    GameEvents.cpp \
    GameEventsXcb.cpp \
    GameLoop.cpp \
    GameThread.cpp \
    KeyMapping.cpp \
    KeyMappingXcb.cpp \
    main.cpp \
    SaveState.cpp \
    SaveStateList.cpp \
    utils.cpp \
    lua/Gui.cpp \
    lua/Input.cpp \
    lua/Main.cpp \
    lua/Memory.cpp \
    lua/Movie.cpp \
    movie/MovieFile.cpp \
    movie/MovieFileAnnotations.cpp \
    movie/MovieFileEditor.cpp \
    movie/MovieFileHeader.cpp \
    movie/MovieFileInputs.cpp \
    ui/AnnotationsWindow.cpp \
    ui/AutoSaveWindow.cpp \
    ui/ControllerAxisWidget.cpp \
    ui/ControllerTabWindow.cpp \
    ui/ControllerWidget.cpp \
    ui/EncodeWindow.cpp \
    ui/ErrorChecking.cpp \
    ui/ExecutableWindow.cpp \
    ui/GameInfoWindow.cpp \
    ui/GameSpecificWindow.cpp \
    ui/InputEditorModel.cpp \
    ui/InputEditorView.cpp \
    ui/InputEditorWindow.cpp \
    ui/InputWindow.cpp \
    ui/KeyPressedDialog.cpp \
    ui/MainWindow.cpp \
    ui/OsdWindow.cpp \
    ui/PointerScanModel.cpp \
    ui/PointerScanWindow.cpp \
    ui/RamSearchModel.cpp \
    ui/RamSearchWindow.cpp \
    ui/RamWatchEditWindow.cpp \
    ui/RamWatchModel.cpp \
    ui/RamWatchWindow.cpp \
    ui/TimeTraceModel.cpp \
    ui/TimeTraceWindow.cpp \
    ui/qtutils.cpp \
    ramsearch/BaseAddresses.cpp \
    ramsearch/CompareOperations.cpp \
    ramsearch/IRamWatchDetailed.cpp \
    ramsearch/MemAccess.cpp \
    ramsearch/MemLayout.cpp \
    ramsearch/MemScanner.cpp \
    ramsearch/MemScannerThread.cpp \
    ramsearch/MemSection.cpp \
    ../shared/AllInputs.cpp \
    ../shared/SingleInput.cpp \
    ../shared/sockethelpers.cpp \
    $(libTAS_MOCSOURCES)

libTAS_CXXFLAGS = $(QT5_CFLAGS) $(LIBLUA_CFLAGS) -fno-stack-protector -Wno-float-equal -fPIC
libTAS_LDADD = $(QT5_LIBS) $(LIBLUA_LIBS) $(PROGRAM_LIBS)

.h_moc.cpp:
	@MOC@ -o $@ $(CPPFLAGS) $<

SUFFIXES = .h _moc.cpp

CLEANFILES = $(libTAS_MOCSOURCES)
