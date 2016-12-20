#ifndef SENSORSDISPLAYFORM_H
#define SENSORSDISPLAYFORM_H

#include <QDialog>
#include "./sensorblock.h"
#include "./ipcamviewer.h"
#include "../server.h"

namespace Ui {
class SensorsDisplayForm;
}

class SensorsDisplayForm : public QDialog
{
    Q_OBJECT

public:
    explicit SensorsDisplayForm(Server* c_server, QWidget *parent = 0);
    ~SensorsDisplayForm();

    quint16 getNextPageTimeout() const;
    void setNextPageTimeout(const quint16 &value);
    QString getSensorCodes() const;
    void setSensorCodes(const QString &value);

    QColor getBg_color() const;
    void setBg_color(const QColor &value);

    QColor getLabel_color() const;
    void setLabel_color(const QColor &value);

    QColor getValue_color() const;
    void setValue_color(const QColor &value);

    QColor getGraphics_color() const;
    void setGraphics_color(const QColor &value);

    quint64 getSensorGraphicsLogInterval() const;
    void setSensorGraphicsLogInterval(const quint64 &value);

    void setIPCamVisibility(bool isvisible);
private slots:
    void update_blocks_list();
    void new_sensor(Sensor* new_sensor);
    void showNextSensorsPage();
    void sensorBlockClicked();

private:
    Ui::SensorsDisplayForm *ui;
    Server* server;
    QString sensor_codes;
    QColor bg_color, label_color, value_color, graphics_color;
    quint64 sensor_graphics_log_interval;

    bool fullscreen_block;

    quint16 display_block_id;
    quint16 next_page_timeout;
    QMap<unsigned, unsigned> sensor_counters;

    IPCamViewer *camviewer;

    virtual void showEvent(QShowEvent * event);
};

#endif // SENSORSDISPLAYFORM_H
