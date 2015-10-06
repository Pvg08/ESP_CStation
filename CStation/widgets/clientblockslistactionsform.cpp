#include "clientblockslistactionsform.h"
#include "ui_clientblockslistactionsform.h"

ClientBlocksListActionsForm::ClientBlocksListActionsForm(QWidget *parent, Server* srv) :
    QWidget(parent),
    ui(new Ui::ClientBlocksListActionsForm)
{
    ui->setupUi(this);
    server = srv;
    parsed_block_id = 0;
    block_forms = new QMap<quint16, ClientBlockActionListForm*>();
    connect(server, SIGNAL(new_block_ready(quint16)), this, SLOT(new_block_ready(quint16)));
}

ClientBlocksListActionsForm::~ClientBlocksListActionsForm()
{
    delete ui;
    delete block_forms;
}

void ClientBlocksListActionsForm::setIPString(QString ip_addr)
{
    QString sitem = ip_addr;
    sitem.replace(QRegExp("^([^\\(\\)]*)\\(DS([0-9]*)\\).*$"), "\\2");
    parsed_block_id = sitem.toInt();
    if (!parsed_block_id) {
        ClientBlock *cblock = server->getClientBlock(QHostAddress(ip_addr).toIPv4Address());
        if (cblock) {
            parsed_block_id = cblock->getblockId();
        }
    }

    QMap<quint16, ClientBlockActionListForm*>::const_iterator i = block_forms->constBegin();
    while (i != block_forms->constEnd()) {
        i.value()->setEnabled(i.key()==parsed_block_id);
        i.value()->setVisible(i.key()==parsed_block_id);
        ++i;
    }
}

void ClientBlocksListActionsForm::new_block_ready(quint16 block_id)
{
    updateclientBlockItemForm(block_id);
}

void ClientBlocksListActionsForm::updateclientBlockItemForm(quint16 block_id)
{
    ClientBlock *cblock = server->getClientBlockByID(block_id);
    if (cblock) {
        bool is_v = true;
        ClientBlockActionListForm *cform = block_forms->value(block_id, NULL);
        if (cform) {
            is_v = cform->isVisible();
            delete cform;
            block_forms->remove(block_id);
        } else {
            is_v = parsed_block_id==block_id;
        }

        cform = new ClientBlockActionListForm(this, cblock);
        cform->setVisible(is_v);
        ui->verticalLayout_controls->addWidget(cform);
        block_forms->insert(block_id, cform);
    }
}
