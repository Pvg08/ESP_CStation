#include "clientaction.h"

ClientAction::ClientAction(QObject *parent, QString action_description) : QObject(parent)
{
    action_is_ready = false;
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

ClientActionParamsList *ClientAction::getParameters() const
{
    return parameters;
}

ClientActionButtonsList *ClientAction::getButtons() const
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

QString ClientAction::getCommand(QVector<QString> *param_values)
{
    QVector<QString> *param_copy = param_values ? new QVector<QString>(*param_values) : new QVector<QString>();
    QString result = prefix + "=";
    QString pval = "";
    bool param_added = false;

    while (param_copy->size()<parameters->size()) param_copy->append("");
    if (!param_copy->isEmpty()) {
        for(int i=0; i<param_copy->size(); i++) {
            pval = param_copy->at(i);

            if (i<parameters->size()) {
                if (parameters->at(i).type=="BOOL") {
                    pval = QString::number(pval.toInt());
                    if (pval=="1" && !parameters->at(i).value.isEmpty()) {
                        pval = parameters->at(i).value;
                    }
                } else if (parameters->at(i).type=="CONST") {
                    pval = parameters->at(i).value;
                } else if (parameters->at(i).type=="INT") {
                    pval = QString::number(pval.toInt());
                } else if (parameters->at(i).type=="UINT") {
                    pval = QString::number(fmax(pval.toInt(), 0));
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
    return result;
}

bool ClientAction::parseDescription(QString description)
{
    QJsonParseError jerror;
    QJsonDocument jdoc= QJsonDocument::fromJson(description.toLatin1(),&jerror);
    if(jerror.error != QJsonParseError::NoError)
        return false;
    QJsonObject obj = jdoc.object();

    code = obj.value("CODE").toString();
    prefix = obj.value("PREFIX").toString();
    need_to_listen_answer = obj.value("LISTEN").toInt() != 0;

    QJsonArray params = obj.value("PARAM").toArray();
    parameters->clear();
    for(auto&& item: params)
    {
        const QJsonObject& param = item.toObject();
        ClientActionParam newparam;
        newparam.default_value = param.value("DEFAULT").toString();
        newparam.name = param.value("NAME").toString();
        newparam.type = param.value("TYPE").toString("CONST");
        newparam.value = param.value("VALUE").toString();
        newparam.skip = param.value("SKIP").toInt() != 0;
        if (param.value("LISTEN").toInt() != 0) {
            need_to_listen_answer = true;
        }
        if (newparam.type!="BOOL" && newparam.type!="INT" && newparam.type!="UINT" && newparam.type!="STRING" && newparam.type!="CONST") {
            newparam.type = "";
        }
        if (!newparam.type.isEmpty() && ((!newparam.value.isEmpty() && newparam.skip) || !newparam.name.isEmpty())) {
            parameters->append(newparam);
        }
    }

    QJsonArray btns = obj.value("BUTTONS").toArray();
    buttons->clear();
    for(auto&& item: btns)
    {
        const QJsonObject& btn = item.toObject();
        ClientParamButton newbtn;
        newbtn.name = btn.value("NAME").toString();
        newbtn.message_set = "";

        QJsonArray paramsets = btn.value("PARAMSET").toArray();
        for(auto&& sitem: paramsets)
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
