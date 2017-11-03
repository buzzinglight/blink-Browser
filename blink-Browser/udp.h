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

#ifndef UDP_H
#define UDP_H

#include <QWidget>
#include <QByteArray>
#include <QUdpSocket>
#include <QHash>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QDateTime>
#include "global.h"

class UdpDatagram;

class Udp : public QObject, public SenderInterface {
    Q_OBJECT

private:
    bool allowLog;
    QUdpSocket *socket;
    QNetworkAccessManager httpRequest;
    quint8  parsingBufferI[4096];
    quint16 parsingBufferISize;
    char    parsingAddressBuffer[255];
    quint8  parsingArgumentsBuffer[255];
    quint16 parsingIndexAddressBuffer, parsingIndexArgumentsBuffer;

public:
    explicit Udp(QObject *parent = 0, quint16 port = 15555);

public:
    void setPort(quint16 port);

public:
    void open();
    QString send(const QString &ip, quint16 port, const QString &destination, const QList<QVariant> &valeurs = QList<QVariant>());
    void send(UdpDatagram *datagram, quint16 counter = 0);

signals:
    void readyUdp();
    void outgoingMessage(const QString &log);
    void outgoingData(const QString &ip, quint16 port, const QString &destination, const QList<QVariant> &valeurs);

public slots:
    void incomingMessage(const QString &, quint16, const QString &, const QList<QVariant> &) {}
private slots:
    void parseOSC();
};


class UdpDatagram : public QObject {
    Q_OBJECT

public:
    explicit UdpDatagram(Udp *_udp, const QString &_ip, quint16 _port, const QString &_destination, const QList<QVariant> &_valeurs, bool _secure);

private:
    Udp *udp;

public:
    QByteArray arguments, typetag, address, buffer;
    QString verboseMessage;
    QHostAddress host;
    quint16 counter;

public:
    bool secure;
    QString ip, destination;
    quint16 port;
    QList<QVariant> valeurs;

private:
    inline void pad(QByteArray & b) const {
        while (b.size() % 4 != 0)
            b += (char)0;
    }
};

#endif // UDP_H
