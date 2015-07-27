#ifndef SENSORSDISPLAYFORM_H
#define SENSORSDISPLAYFORM_H

#include <QDialog>
#include "./sensorblock.h"
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

private slots:
    void update_blocks_list();
    void update_sensors_values(quint16 block_id);

private:
    Ui::SensorsDisplayForm *ui;
    Server* server;
    quint16 display_block_id;

    void showNextSensorsPage();
};

#endif // SENSORSDISPLAYFORM_H
