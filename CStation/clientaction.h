#ifndef CLIENTACTION_H
#define CLIENTACTION_H

#include <QObject>
#include <QVector>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>
#include <math.h>
#include "abstractserver.h"

typedef struct
{
    QString name, type, value, default_value;
    bool skip;
} ClientActionParam;

typedef struct
{
    QString name, message_set;
} ClientParamButton;

typedef QVector<ClientActionParam> ClientActionParamsList;
typedef QVector<ClientParamButton> ClientActionButtonsList;

class ClientAction : public QObject
{
    Q_OBJECT
public:
    explicit ClientAction(AbstractServer *parent, QString action_description);
    ~ClientAction();
    bool actionIsReady() const;
    QString getCode() const;
    QString getPrefix() const;
    bool isNeedToListenAnswer() const;
    ClientActionParamsList *getParameters();
    ClientActionButtonsList *getButtons();
    quint16 getBlockID() const;
    QString getActionName() const;
    void setBlockID(const quint16 &value);

    void sendCommand(QVector<QString>* param_values);
    void sendCommand(QString param_values);
    void sendCommandButton(QVector<QString>* param_values, ClientParamButton* clicked_btn);

private:
    ClientActionParamsList *parameters;
    ClientActionButtonsList *buttons;
    QString code;
    QString actionName;
    QString prefix;
    bool need_to_listen_answer;
    bool action_is_ready;
    quint16 block_id;

    bool parseDescription(QString description);
    QString getCommand(QVector<QString>* param_values, ClientParamButton* clicked_btn);
private slots:

signals:
    void InitiateSending(QString message, bool listen_answer);
public slots:
};

#endif // CLIENTACTION_H
