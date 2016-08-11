#ifndef IPCAMTHREAD_H
#define IPCAMTHREAD_H

#include <QObject>
#include <QDebug>
#include <QThread>
#include <QMutex>
#include <QImage>
#include <vlc/vlc.h>

struct IPCamVideoDataStruct
{
    int param;
};

class IPCamThread : public QThread
{
    Q_OBJECT
private:
    static IPCamThread* _self;
    IPCamThread(QObject *parent = 0);
    ~IPCamThread();

    Q_DISABLE_COPY(IPCamThread)

    void run();

    QString addrName;
    unsigned fps;
    QMutex mutex;
    bool quit;
    uint8_t *videoBuffer = 0;
    uint16_t video_width = 0;
    uint16_t video_height = 0;
    uint64_t frame_index = 0;
    uint64_t frame_old_index = 0;
    int videoBufferSize = 0;
    QImage* current_frame;

signals:
    void new_frame_ready();
public:
    static IPCamThread* Instance();
    static bool DeleteInstance();

    void listen(const QString &addrName, unsigned fps);
    void do_stop();
    void prerenderActions(int buf_size);
    void postrenderActions(int width, int height);

    uint8_t *getVideoBuffer();
    void setVideoBuffer(uint8_t *value);
    QImage *getCurrentFrame() const;
};

#endif // IPCAMTHREAD_H
