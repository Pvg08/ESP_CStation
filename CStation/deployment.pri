
CONFIG( release, debug|release ) {
    isEmpty(TARGET_EXT) {
        win32 {
            TARGET_CUSTOM_EXT = .exe
        }
    } else {
        TARGET_CUSTOM_EXT = $${TARGET_EXT}
    }
    win32 {
        DEPLOY_COMMAND = windeployqt
    }
    DEPLOY_TARGET = $$shell_quote($$shell_path($${OUT_PWD}/release/$${TARGET}$${TARGET_CUSTOM_EXT}))
    warning($${DEPLOY_COMMAND} $${DEPLOY_TARGET})
    QMAKE_POST_LINK = $${DEPLOY_COMMAND} $${DEPLOY_TARGET}
    DTYPE = release
} else:CONFIG(debug, debug|release) {
    DTYPE = debug
}

win32 {
    PR_LIB_PATH = $$PWD/vlc/bin/win32

    RR_LIB_FILE1.commands = $(COPY_FILE) $$shell_quote($$shell_path($${PR_LIB_PATH}/libvlc.dll)) $$shell_quote($$shell_path($$OUT_PWD/$${DTYPE}))
    RR_LIB_FILE1.target = vlcfile1
    RR_LIB_FILE2.commands = $(COPY_FILE) $$shell_quote($$shell_path($${PR_LIB_PATH}/libvlccore.dll)) $$shell_quote($$shell_path($$OUT_PWD/$${DTYPE}))
    RR_LIB_FILE2.target = vlcfile2
    RR_LIB_DIR.commands = $(COPY_DIR) $$shell_quote($$shell_path($${PR_LIB_PATH}/plugins)) $$shell_quote($$shell_path($$OUT_PWD/$${DTYPE}/plugins))
    RR_LIB_DIR.target = folder_plugins

    QMAKE_EXTRA_TARGETS += RR_LIB_FILE1 RR_LIB_FILE2 RR_LIB_DIR
    PRE_TARGETDEPS += vlcfile1 vlcfile2 folder_plugins
}
