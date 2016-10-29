#include "clientaction.h"

ClientAction::ClientAction(AbstractServer *parent, QString action_description) : QObject(parent)
{
    action_is_ready = false;
    block_id = 0;
    parameters = new ClientActionParamsList();
    buttons = new ClientActionButtonsList();
    parseDescription(action_description);
}

ClientAction::~ClientAction()
{
    delete parameters;
    delete buttons;
}

bool ClientAction::actionIsReady() const
{
    return action_is_ready;
}

QString ClientAction::getCode() const
{
    return code;
}

QString ClientAction::getPrefix() const
{
    return prefix;
}

bool ClientAction::isNeedToListenAnswer() const
{
    return need_to_listen_answer;
}

ClientActionParamsList *ClientAction::getParameters()
{
    return parameters;
}

ClientActionButtonsList *ClientAction::getButtons()
{
    return buttons;
}

quint16 ClientAction::getBlockID() const
{
    return block_id;
}

void ClientAction::setBlockID(const quint16 &value)
{
    block_id = value;
}

void ClientAction::sendCommand(QVector<QString> *param_values)
{
    AbstractServer *server = dynamic_cast<AbstractServer*>(this->parent());
    if (server && block_id) {
        server->SendData(block_id, getCommand(param_values, NULL));
    }
    //need_to_listen_answer
}

void ClientAction::sendCommand(QString param_values)
{
    AbstractServer *server = dynamic_cast<AbstractServer*>(this->parent());
    if (server && block_id) {
        server->SendData(block_id, prefix + "=" + param_values);
    }
}

void ClientAction::sendCommandButton(QVector<QString> *param_values, ClientParamButton *clicked_btn)
{
    AbstractServer *server = dynamic_cast<AbstractServer*>(this->parent());
    if (server && block_id) {
        server->SendData(block_id, getCommand(param_values, clicked_btn));
    }
    //need_to_listen_answer
}

QString ClientAction::getActionName() const
{
    return actionName;
}

QString ClientAction::getCommand(QVector<QString> *param_values, ClientParamButton* clicked_btn)
{
    QString result = prefix + "=";

    if (clicked_btn) {
        result += clicked_btn->message_set;
    } else {
        QVector<QString> *param_copy = param_values ? new QVector<QString>(*param_values) : new QVector<QString>();
        QString pval = "";
        bool param_added = false;

        while (param_copy->size()<parameters->size()) param_copy->append("");
        if (!param_copy->isEmpty()) {
            for(int i=0; i<param_copy->size(); i++) {
                pval = param_copy->at(i);

                if (i<parameters->size()) {
                    if (parameters->at(i).type=="BOOL") {
                        pval = QString::number(pval.toInt());
                        if (pval=="1" && !parameters->at(i).value.isEmpty() && parameters->at(i).value!="0") {
                            pval = parameters->at(i).value;
                        }
                    } else if (parameters->at(i).type=="CONST") {
                        pval = parameters->at(i).value;
                    } else if (parameters->at(i).type=="INT") {
                        pval = QString::number(pval.toInt());
                    } else if (parameters->at(i).type=="UINT") {
                        pval = QString::number(fmax(pval.toInt(), 0));
                    } else if (parameters->at(i).type=="DOUBLE") {
                        pval = QString::number(pval.toDouble(), 'f');
                    } else if (parameters->at(i).type=="TIMESTAMP") {
                        pval = QString::number(pval.toDouble(), 'f', 0);
                    }

                    if (parameters->at(i).type!="CONST" && (pval=="0" || pval.isEmpty()) && parameters->at(i).skip) {
                        continue;
                    }
                }

                if (!pval.isEmpty()) {
                    result = result + (param_added ? "," : "") + pval;
                    param_added = true;
                }
            }
        }
        delete param_copy;
    }

    return result;
}

bool ClientAction::parseDescription(QString description)
{
    description.replace('\'', '"');
    QJsonParseError jerror;
    QJsonDocument jdoc= QJsonDocument::fromJson(description.toLocal8Bit(),&jerror);

    if(jerror.error != QJsonParseError::NoError) return false;
    QJsonObject obj = jdoc.object();

    code = obj.value("CODE").toString();
    actionName = code.trimmed();
    if (!actionName.isEmpty()) {
        actionName[0]=actionName[0].toUpper();
    }
    prefix = obj.value("PREFIX").toString();
    need_to_listen_answer = obj.value("LISTEN").toInt() != 0;

    QJsonArray params = obj.value("PARAM").toArray();
    parameters->clear();
    foreach (const QJsonValue & item, params)
    {
        const QJsonObject& param = item.toObject();
        ClientActionParam newparam;
        newparam.name = param.value("NAME").toString();
        newparam.type = param.value("TYPE").toString("CONST");
        newparam.value = param.value("VALUE").isString() ? param.value("VALUE").toString() : QString::number(param.value("VALUE").toInt());
        newparam.skip = param.value("SKIP").toInt() != 0;
        if (param.value("DEFAULT").isString() || newparam.type=="STRING") {
            newparam.default_value = param.value("DEFAULT").toString();
        } else {
            newparam.default_value = QString::number(param.value("DEFAULT").toInt());
        }

        if (param.value("LISTEN").toInt() != 0) {
            need_to_listen_answer = true;
        }
        if (newparam.type!="BOOL" && newparam.type!="INT" && newparam.type!="UINT" && newparam.type!="DOUBLE" && newparam.type!="TIMESTAMP" && newparam.type!="STRING" && newparam.type!="CONST") {
            newparam.type = "";
        }
        if (!newparam.type.isEmpty() && ((!newparam.value.isEmpty() && newparam.skip) || !newparam.name.isEmpty())) {
            parameters->append(newparam);
        }
    }

    QJsonArray btns = obj.value("BUTTONS").toArray();
    buttons->clear();

    foreach (const QJsonValue & item, btns) {

        const QJsonObject& btn = item.toObject();
        ClientParamButton newbtn;
        newbtn.name = btn.value("NAME").toString();
        newbtn.message_set = "";

        QJsonArray paramsets = btn.value("PARAMSET").toArray();

        foreach (const QJsonValue & sitem, paramsets)
        {
            newbtn.message_set += (!newbtn.message_set.isEmpty() ? "," : "") + sitem.toString();
        }

        if (!newbtn.name.isEmpty()) {
            buttons->append(newbtn);
        }
    }

    action_is_ready = !code.isEmpty() && !prefix.isEmpty() && !parameters->isEmpty();
    return action_is_ready;
}
