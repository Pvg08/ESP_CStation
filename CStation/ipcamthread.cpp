#include "ipcamthread.h"

IPCamThread *IPCamThread::_self = 0;

void cbVideoPrerender(void *p_video_data, uint8_t **pp_pixel_buffer, int size) {
    IPCamThread::Instance()->prerenderActions(size);
    *pp_pixel_buffer = IPCamThread::Instance()->getVideoBuffer();
}

void cbVideoPostrender(void *p_video_data, uint8_t *p_pixel_buffer, int width, int height, int pixel_pitch, int size, int64_t pts) {
    IPCamThread::Instance()->postrenderActions(width, height);
}

static void handleEvent(const libvlc_event_t* pEvt, void* pUserData)
{
    /*libvlc_time_t time;
    switch(pEvt->type)
    {
        case libvlc_MediaPlayerTimeChanged:
            time = libvlc_media_player_get_time(mp);
            printf("MediaPlayerTimeChanged %lld ms\n", (long long)time);
            break;
        case libvlc_MediaPlayerEndReached:
            printf ("MediaPlayerEndReached\n");
            done = 1;
            break;
        default:
            printf("%s\n", libvlc_event_type_name(pEvt->type));
    }*/
}

IPCamThread::IPCamThread(QObject *parent) : QThread(parent), quit(false)
{
    fps = 1;
    addrName = "";
}

IPCamThread::~IPCamThread()
{

}

void IPCamThread::listen(const QString &addrName, unsigned fps)
{
    this->addrName = addrName;
    this->fps = fps;
    quit = false;
    if (!isRunning()) start();
}

void IPCamThread::run()
{
    // VLC pointers
    libvlc_instance_t *inst;
    libvlc_media_t *m;
    libvlc_media_player_t *mp;
    void *pUserData = 0;

    IPCamVideoDataStruct dataStruct;
    dataStruct.param = 0;

    // VLC options
    char smem_options[1000];

    // RV24
    sprintf(smem_options,
        "#transcode{vcodec=RV24,scale=0.4,fps=%d}:smem{"
        "video-prerender-callback=%lld,"
        "video-postrender-callback=%lld,"
        "video-data=%lld,"
        "no-time-sync},"
        , fps
        , (long long int)(intptr_t)(void*)&cbVideoPrerender
        , (long long int)(intptr_t)(void*)&cbVideoPostrender
        , (long long int)(intptr_t)(void*)&dataStruct
    );

    const char * const vlc_args[] = {
        "-I", "dummy",            // Don't use any interface
        "--ignore-config",        // Don't use VLC's config
        "--verbose=1",            // Be verbose
        "--sout", smem_options    // Stream to memory
    };

    // We launch VLC
    inst = libvlc_new(sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args);

    /* Create a new item */
    m = libvlc_media_new_location(inst, addrName.toLocal8Bit().data());

    /* Create a media player playing environement */
    mp = libvlc_media_player_new_from_media (m);

    libvlc_event_manager_t* eventManager = libvlc_media_player_event_manager(mp);
    libvlc_event_attach(eventManager, libvlc_MediaPlayerTimeChanged, handleEvent, pUserData);
    libvlc_event_attach(eventManager, libvlc_MediaPlayerEndReached, handleEvent, pUserData);
    libvlc_event_attach(eventManager, libvlc_MediaPlayerPositionChanged, handleEvent, pUserData);

    /* play the media_player */
    libvlc_media_player_play(mp);

    while (!quit) {
        if(videoBuffer && frame_index && (frame_old_index!=frame_index))
        {
            mutex.lock();
            QImage tmp(videoBuffer, video_width ? video_width : 1, video_height ? video_height : 1, QImage::Format_RGB888);
            tmp.save("bf"+QString::number(frame_index)+".bmp");
            frame_old_index = frame_index;
            mutex.unlock();
        }
        QThread::msleep (120);
    }
    libvlc_release(inst);
}

uint8_t *IPCamThread::getVideoBuffer()
{
    return videoBuffer;
}

void IPCamThread::setVideoBuffer(uint8_t *value)
{
    videoBuffer = value;
}

void IPCamThread::do_stop()
{
    if (isRunning()) quit = true;
}

void IPCamThread::prerenderActions(int buf_size)
{
    mutex.lock();
    if (buf_size > videoBufferSize || !videoBuffer)
    {
        free(videoBuffer);
        videoBuffer = (uint8_t *) malloc(buf_size);
        videoBufferSize = buf_size;
    }
}

void IPCamThread::postrenderActions(int width, int height)
{
    video_width = width;
    video_height = height;
    frame_index++;
    mutex.unlock();
}

IPCamThread *IPCamThread::Instance()
{
    if(!_self)
    {
        _self = new IPCamThread(0);
    }
    return _self;
}

bool IPCamThread::DeleteInstance()
{
    if(_self)
    {
        delete _self;
        _self = 0;
        return true;
    }
    return false;
}
