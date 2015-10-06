#ifndef CLIENTACTION_H
#define CLIENTACTION_H

#include <QObject>
#include <QVector>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>
#include <math.h>

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
    explicit ClientAction(QObject *parent, QString action_description);
    ~ClientAction();
    bool actionIsReady() const;
    QString getCode() const;
    QString getPrefix() const;
    bool isNeedToListenAnswer() const;
    ClientActionParamsList *getParameters() const;
    ClientActionButtonsList *getButtons() const;
    quint16 getBlockID() const;
    void setBlockID(const quint16 &value);

    QString getCommand(QVector<QString>* param_values);
private:
    ClientActionParamsList *parameters;
    ClientActionButtonsList *buttons;
    QString code;
    QString prefix;
    bool need_to_listen_answer;
    bool action_is_ready;
    quint16 block_id;

    bool parseDescription(QString description);
private slots:

signals:

public slots:
};

#endif // CLIENTACTION_H
