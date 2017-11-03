/*
    This file is part of HTTP2OSC
    Copyright (C) 2014-2015 — Buzzing Light

    Development:     Guillaume Jacquemin (http://www.buzzinglight.com)

    This file was written by Guillaume Jacquemin.

    HTTP2OSC is a free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef INTERFACEHTTP_H
#define INTERFACEHTTP_H

#include <QWidget>
#include <QtGlobal>
#include <QDesktopServices>
#include <QDir>
#include <QBuffer>
#include <QDateTime>
#include <QApplication>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTcpServer>
#include <QTcpSocket>
#include "global.h"
#ifndef QT4
    #include <QUrlQuery>
#endif


class InterfaceHttpServer : public QTcpServer, public SenderInterface {
    Q_OBJECT

public:
    InterfaceHttpServer(QObject *parent, quint16 port);

private:
    QNetworkAccessManager *http;

public:
    void setPort(quint16 port);
    QString send(const QString &ip, quint16 port, const QString &destination, const QList<QVariant> &valeurs);

protected:
    void incomingConnection(qintptr socketDescriptor);
private slots:
    void readClient();
    void discardClient();
    void parse(QNetworkReply*);
signals:
    void parseRequest(QNetworkReply*);
    void parseSocket(QTcpSocket*);
};



class InterfaceHttp : public QObject, public SenderInterface {
    Q_OBJECT
    
public:
    explicit InterfaceHttp(QObject *parent = 0, quint16 port = 15556);
    ~InterfaceHttp();

private:
    InterfaceHttpServer *httpServer;
public:
    void setPort(quint16 port) {
        httpServer->setPort(port);
    }
    QString send(const QString &ip, quint16 port, const QString &destination, const QList<QVariant> &valeurs) {
        return httpServer->send(ip, port, destination, valeurs);
    }

private slots:
    void parseRequest(QNetworkReply*);
    void parseSocket(QTcpSocket*);

signals:
    void outgoingMessage(const QString &log);
    void outgoingData(const QString &ip, quint16 port, const QString &destination, const QList<QVariant> &valeurs);
};

#endif // INTERFACEHTTP_H
