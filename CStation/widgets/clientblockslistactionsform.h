#ifndef CLIENTBLOCKSLISTACTIONSFORM_H
#define CLIENTBLOCKSLISTACTIONSFORM_H

#include <QWidget>
#include "../server.h"
#include "./clientblockactionlistform.h"

namespace Ui {
class ClientBlocksListActionsForm;
}

class ClientBlocksListActionsForm : public QWidget
{
    Q_OBJECT

public:
    explicit ClientBlocksListActionsForm(QWidget *parent, Server* srv);
    ~ClientBlocksListActionsForm();

    void setIPString(QString ip_addr);

private slots:
    void new_block_ready(quint16 block_id);

private:
    Ui::ClientBlocksListActionsForm *ui;
    QMap<quint16, ClientBlockActionListForm*> *block_forms;
    Server *server;
    quint16 parsed_block_id;

    void updateclientBlockItemForm(quint16 block_id);
};

#endif // CLIENTBLOCKSLISTACTIONSFORM_H
