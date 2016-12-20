#include "ipcamthread.h"

IPCamThread *IPCamThread::_self = 0;

void cbVideoPrerender(void *p_video_data, uint8_t **pp_pixel_buffer, int size) {
    IPCamThread::Instance()->prerenderActions(size);
    *pp_pixel_buffer = IPCamThread::Instance()->getVideoBuffer();
}

void cbVideoPostrender(void *p_video_data, uint8_t *p_pixel_buffer, int width, int height, int pixel_pitch, int size, int64_t pts) {
    IPCamThread::Instance()->postrenderActions(width, height);
}

IPCamThread::IPCamThread(QObject *parent) : QThread(parent), quit(false)
{
    fps = 1;
    addrName = "";
    current_frame = NULL;
    setStatusText(tr("No camera action"), false);
}

IPCamThread::~IPCamThread()
{
    if (current_frame) delete current_frame;
}

void IPCamThread::listen(const QString &addrName, unsigned fps)
{
    this->addrName = addrName;
    this->fps = fps;
    quit = false;
    buffer_changed = false;
    if (!isRunning()) start();
    setStatusText(tr("Camera Initialization"), false);
}

void IPCamThread::run()
{
    qDebug() << "Playing thread started\n";

    // VLC pointers
    libvlc_instance_t *inst;
    libvlc_media_t *m;
    libvlc_media_player_t *mp;

    IPCamVideoDataStruct dataStruct;
    dataStruct.param = 0;

    // VLC options
    char smem_options[1000];

    // RV24
    sprintf(smem_options,
        "#transcode{vcodec=RV24,width=640,height=480,fps=%d,vfilter=croppadd{croptop=20,cropbottom=20}}:smem{"
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
        "--verbose=0",            // Be verbose
        "--no-sout-audio",        // no audio
        "--sout", smem_options    // Stream to memory
    };

    // We launch VLC
    inst = libvlc_new(sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args);

    /* Create a new item */
    m = libvlc_media_new_location(inst, addrName.toLocal8Bit().data());

    /* Create a media player playing environement */
    mp = libvlc_media_player_new_from_media (m);

    libvlc_state_t last_state;
    bool playing;
    unsigned nplay_counter;
    unsigned fps_delay = 1000 / fps;
    if (fps_delay < 50) fps_delay = 50;
    if (fps_delay > 1000) fps_delay = 1000;
    fps_delay = fps_delay*0.8;

    while (!quit) {

        /* play the media_player */
        playing = libvlc_media_player_play(mp)==0;
        nplay_counter = 0;

        qDebug() << "Playing start: " << (playing?1:0);

        setStatusText(tr("Camera buffering..."), false);

        while (!quit && playing && nplay_counter<100) {
            if(videoBuffer && frame_index && (frame_old_index!=frame_index))
            {
                nplay_counter = 0;
                frame_old_index = frame_index;
                buffer_changed = true;
                emit new_frame_ready();
            }
            QThread::msleep(fps_delay);

            last_state = libvlc_media_player_get_state(mp);
            playing = (last_state==libvlc_Buffering) || (last_state==libvlc_Playing) || (last_state==libvlc_Opening);
            if (last_state!=libvlc_Playing) nplay_counter++;
        }
        libvlc_media_player_stop(mp);

        setStatusText(tr("Camera error..."), true);

        if (!quit) QThread::msleep(10000);
    }

    libvlc_release(inst);

    mutex.lock();
    free(videoBuffer);
    buffer_changed = false;
    videoBuffer = NULL;
    videoBufferSize = 0;
    if (current_frame) delete current_frame;
    current_frame = NULL;
    mutex.unlock();

    setStatusText(tr("No camera action"), false);

    qDebug() << "Playing thread closed\n";
}

void IPCamThread::setStatusText(QString newtext, bool do_show)
{
    if (do_show) {
        mutex.lock();
        buffer_changed = false;
        if (current_frame) delete current_frame;
        current_frame = NULL;
    }
    statusText = newtext;
    if (do_show) {
        mutex.unlock();
    }
    emit new_frame_ready();
}

QString IPCamThread::getStatusText() const
{
    return statusText;
}

QImage *IPCamThread::getCurrentFrame()
{
    mutex.lock();
    if (buffer_changed) {
        buffer_changed = false;
        if (current_frame) delete current_frame;
        current_frame = NULL;
        if (videoBuffer && video_width && video_height && videoBufferSize>=(video_width*video_height*3)) {
            try {
                current_frame = new QImage(videoBuffer, video_width, video_height, QImage::Format_RGB888);
            } catch (int err) {
                current_frame = NULL;
            }
        }
    }
    mutex.unlock();
    return current_frame;
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
