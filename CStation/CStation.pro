#-------------------------------------------------
#
# Project created by QtCreator 2015-07-10T17:22:35
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CStation
TEMPLATE = app

win32 {
    LIBS += $$PWD/vlc/lib/libvlc.lib
}
include(deployment.pri)

SOURCES += main.cpp\
    mainwindow.cpp \
    abstractserver.cpp \
    server.cpp \
    clientblock.cpp \
    sensor.cpp \
    clientaction.cpp \
    widgets/sensorblock.cpp \
    widgets/sensorsdisplayform.cpp \
    widgets/msensordrawsurface.cpp \
    widgets/clientblockactionform.cpp \
    widgets/clientblockactionlistform.cpp \
    widgets/clientblockslistactionsform.cpp \
    widgets/ipcamviewer.cpp \
    ipcamthread.cpp

HEADERS  += mainwindow.h \
    abstractserver.h \
    server.h \
    clientblock.h \
    sensor.h \
    clientaction.h \
    widgets/sensorblock.h \
    widgets/sensorsdisplayform.h \
    widgets/msensordrawsurface.h \
    widgets/clientblockactionform.h \
    widgets/clientblockactionlistform.h \
    widgets/clientblockslistactionsform.h \
    vlc/plugins/vlc_about.h \
    vlc/plugins/vlc_access.h \
    vlc/plugins/vlc_addons.h \
    vlc/plugins/vlc_aout.h \
    vlc/plugins/vlc_aout_volume.h \
    vlc/plugins/vlc_arrays.h \
    vlc/plugins/vlc_atomic.h \
    vlc/plugins/vlc_avcodec.h \
    vlc/plugins/vlc_bits.h \
    vlc/plugins/vlc_block.h \
    vlc/plugins/vlc_block_helper.h \
    vlc/plugins/vlc_charset.h \
    vlc/plugins/vlc_codec.h \
    vlc/plugins/vlc_common.h \
    vlc/plugins/vlc_config.h \
    vlc/plugins/vlc_config_cat.h \
    vlc/plugins/vlc_configuration.h \
    vlc/plugins/vlc_cpu.h \
    vlc/plugins/vlc_demux.h \
    vlc/plugins/vlc_dialog.h \
    vlc/plugins/vlc_epg.h \
    vlc/plugins/vlc_es.h \
    vlc/plugins/vlc_es_out.h \
    vlc/plugins/vlc_events.h \
    vlc/plugins/vlc_filter.h \
    vlc/plugins/vlc_fingerprinter.h \
    vlc/plugins/vlc_fourcc.h \
    vlc/plugins/vlc_fs.h \
    vlc/plugins/vlc_gcrypt.h \
    vlc/plugins/vlc_http.h \
    vlc/plugins/vlc_httpd.h \
    vlc/plugins/vlc_image.h \
    vlc/plugins/vlc_inhibit.h \
    vlc/plugins/vlc_input.h \
    vlc/plugins/vlc_input_item.h \
    vlc/plugins/vlc_keys.h \
    vlc/plugins/vlc_main.h \
    vlc/plugins/vlc_md5.h \
    vlc/plugins/vlc_media_library.h \
    vlc/plugins/vlc_messages.h \
    vlc/plugins/vlc_meta.h \
    vlc/plugins/vlc_meta_fetcher.h \
    vlc/plugins/vlc_mime.h \
    vlc/plugins/vlc_modules.h \
    vlc/plugins/vlc_mouse.h \
    vlc/plugins/vlc_mtime.h \
    vlc/plugins/vlc_network.h \
    vlc/plugins/vlc_objects.h \
    vlc/plugins/vlc_opengl.h \
    vlc/plugins/vlc_picture.h \
    vlc/plugins/vlc_picture_fifo.h \
    vlc/plugins/vlc_picture_pool.h \
    vlc/plugins/vlc_playlist.h \
    vlc/plugins/vlc_plugin.h \
    vlc/plugins/vlc_probe.h \
    vlc/plugins/vlc_rand.h \
    vlc/plugins/vlc_services_discovery.h \
    vlc/plugins/vlc_sout.h \
    vlc/plugins/vlc_spu.h \
    vlc/plugins/vlc_stream.h \
    vlc/plugins/vlc_strings.h \
    vlc/plugins/vlc_subpicture.h \
    vlc/plugins/vlc_text_style.h \
    vlc/plugins/vlc_threads.h \
    vlc/plugins/vlc_tls.h \
    vlc/plugins/vlc_url.h \
    vlc/plugins/vlc_variables.h \
    vlc/plugins/vlc_video_splitter.h \
    vlc/plugins/vlc_vlm.h \
    vlc/plugins/vlc_vout.h \
    vlc/plugins/vlc_vout_display.h \
    vlc/plugins/vlc_vout_osd.h \
    vlc/plugins/vlc_vout_window.h \
    vlc/plugins/vlc_xlib.h \
    vlc/plugins/vlc_xml.h \
    vlc/deprecated.h \
    vlc/libvlc.h \
    vlc/libvlc_events.h \
    vlc/libvlc_media.h \
    vlc/libvlc_media_discoverer.h \
    vlc/libvlc_media_library.h \
    vlc/libvlc_media_list.h \
    vlc/libvlc_media_list_player.h \
    vlc/libvlc_media_player.h \
    vlc/libvlc_structures.h \
    vlc/libvlc_version.h \
    vlc/libvlc_vlm.h \
    vlc/vlc.h \
    widgets/ipcamviewer.h \
    ipcamthread.h

FORMS    += mainwindow.ui \
    widgets/sensorblock.ui \
    widgets/sensorsdisplayform.ui \
    widgets/clientblockactionform.ui \
    widgets/clientblockactionlistform.ui \
    widgets/clientblockslistactionsform.ui \
    widgets/ipcamviewer.ui

TRANSLATIONS += translations/CStation_ru_RU.ts

DISTFILES += \
    deployment.pri
