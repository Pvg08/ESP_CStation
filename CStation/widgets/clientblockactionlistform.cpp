#include "clientblockactionlistform.h"
#include "ui_clientblockactionlistform.h"

ClientBlockActionListForm::ClientBlockActionListForm(QWidget *parent, ClientBlock* cblock) :
    QWidget(parent),
    ui(new Ui::ClientBlockActionListForm)
{
    ui->setupUi(this);
    client_block = cblock;
    action_controls = new QVector<ClientBlockActionForm*>();
    createControls();
}

ClientBlockActionListForm::~ClientBlockActionListForm()
{
    delete ui;
    delete action_controls;
}

ClientBlock *ClientBlockActionListForm::getClientBlock() const
{
    return client_block;
}

void ClientBlockActionListForm::createControls()
{
    if (!client_block) return;

    ui->label->setText(tr("Actions of block %1:").arg("DS"+QString::number(client_block->getblockId())));

    ClientActions *actlist = client_block->getClientActions();

    if (actlist) {
        ClientBlockActionForm *tform;
        ClientActions::const_iterator i = actlist->constBegin();
        while (i != actlist->constEnd()) {
            tform = new ClientBlockActionForm(ui->scrollAreaWidgetContents, i.value());
            ui->scrollAreaWidgetContents->layout()->addWidget(tform);
            action_controls->append(tform);
            ++i;
        }

        for(int i=0; i<action_controls->size(); i++) {
            if (action_controls->at(i)->isJustButton()) {
                int windex = ui->scrollAreaWidgetContents->layout()->indexOf(action_controls->at(i));
                QLayoutItem *item = ui->scrollAreaWidgetContents->layout()->takeAt(windex);
                if (item) {
                    ui->scrollAreaWidgetContents->layout()->addItem(item);
                }
            }
        }

        QWidget *n_widget = new QWidget(ui->scrollAreaWidgetContents);
        n_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        ui->scrollAreaWidgetContents->layout()->addWidget(n_widget);
    }
}
