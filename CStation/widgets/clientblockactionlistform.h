#ifndef CLIENTBLOCKACTIONLISTFORM_H
#define CLIENTBLOCKACTIONLISTFORM_H

#include <QWidget>
#include "../clientblock.h"
#include "./clientblockactionform.h"

namespace Ui {
class ClientBlockActionListForm;
}

class ClientBlockActionListForm : public QWidget
{
    Q_OBJECT

public:
    explicit ClientBlockActionListForm(QWidget *parent, ClientBlock* cblock);
    ~ClientBlockActionListForm();
    ClientBlock *getClientBlock() const;
private:
    Ui::ClientBlockActionListForm *ui;
    ClientBlock* client_block;
    QVector<ClientBlockActionForm*> *action_controls;

    void createControls();
};

#endif // CLIENTBLOCKACTIONLISTFORM_H
