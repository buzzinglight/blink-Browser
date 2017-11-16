#ifndef BROWSER_H
#define BROWSER_H

#include <QMainWindow>
#include <QSettings>
#include <QFileInfo>
#include <QTextCodec>
#include <QDir>
#include <QWebEngineUrlRequestInterceptor>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QDateTime>
#include <QStandardPaths>
#include <QTimer>
#include <QPrinterInfo>
#include "interfacehttp.h"
#include "udp.h"
#include "global.h"

namespace Ui {
class Browser;
}

class EventEater : public QObject {
    Q_OBJECT

public:
    QDateTime lastAction;
    qint32 timeout;

public:
    explicit EventEater(qint32 _timeout);
protected:
    bool eventFilter(QObject *obj, QEvent *event);
};

class WebUrlRequestInterceptor : public QWebEngineUrlRequestInterceptor {
    Q_OBJECT

public:
    QStringList allow, deny;

public:
    WebUrlRequestInterceptor(QObject *p = Q_NULLPTR);
    void interceptRequest(QWebEngineUrlRequestInfo &info);
};

class Browser : public QMainWindow, public MainWindowInterface {
    Q_OBJECT

public:
    explicit Browser(QWidget *parent = 0);
    ~Browser();

public:
    QString home;
    EventEater *eventEater;

private:
    qint16 ticketWidth;
    bool doPrint;


protected:
    void timerEvent(QTimerEvent *event);

public slots:
    void setUrl(QString url = "");
    void action();
    void urlChanged();
    void urlChanged(const QUrl &url);
    void loadStarted();
    void loadProgress(int);
    void loadFinished(bool);
    void print(qint8 action = 1);
    void incomingMessage(const QString &log);
    void incomingData(const QString &ip, quint16 port, const QString &destination, const QList<QVariant> &valeurs);

private:
    Ui::Browser *ui;
};

#endif // BROWSER_H
