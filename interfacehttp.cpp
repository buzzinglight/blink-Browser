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

#include "interfacehttp.h"

InterfaceHttp::InterfaceHttp(QObject *parent, quint16 port) :
    QObject(parent) {
    //HTTP server
    httpServer = new InterfaceHttpServer(this, port);
    connect(httpServer, SIGNAL(parseRequest(QNetworkReply*)), SLOT(parseRequest(QNetworkReply*)));
    connect(httpServer, SIGNAL(parseSocket(QTcpSocket*)),     SLOT(parseSocket(QTcpSocket*)));
}

InterfaceHttpServer::InterfaceHttpServer(QObject *parent, quint16 port) :
    QTcpServer(parent) {
    http = new QNetworkAccessManager(this);
    connect(http, SIGNAL(finished(QNetworkReply*)), SLOT(parse(QNetworkReply*)));
    setPort(port);
}
void InterfaceHttpServer::setPort(quint16 port) {
    if(isListening())
        close();
    listen(QHostAddress::Any, port);
}
QString InterfaceHttpServer::send(const QString &ip, quint16 port, const QString &destination, const QList<QVariant> &) {
    QUrl url;
    url.setScheme("http");
    url.setHost(ip);
    url.setPort(port);
    url.setPath(destination);
    /*
    QUrlQuery query;
    for(quint16 i = 0 ; i < valeurs.count() ; i++)
        query.addQueryItem(QString("value%1").arg(i), valeurs.at(i).toString());
    url.setQuery(query);
    */
    http->get(QNetworkRequest(url));
    return QString("%2: HTTP sent on %1").arg(url.toString()).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"));
}

void InterfaceHttpServer::parse(QNetworkReply *reply) {
    emit(parseRequest(reply));
}
void InterfaceHttp::parseRequest(QNetworkReply *reply) {
    //QString(reply->readAll());
    QString replyStr = reply->readAll();
}


//HTTP reception
void InterfaceHttpServer::incomingConnection(qintptr socketDescriptor) {
    QTcpSocket *socket = new QTcpSocket(this);
    connect(socket, SIGNAL(readyRead()),    this, SLOT(readClient()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(discardClient()));
    socket->setSocketDescriptor(socketDescriptor);
}
void InterfaceHttpServer::readClient() {
    QTcpSocket *socket = (QTcpSocket*)sender();
    if(socket->canReadLine())
        emit(parseSocket(socket));
}

void InterfaceHttp::parseSocket(QTcpSocket *socket) {
    QStringList tokens = QString(socket->readLine()).split(QRegExp("[ \r\n][ \r\n]*"));
    if((tokens.count() > 1) && (tokens.at(0) == "GET")) {
#ifdef QT4
        QUrl url(tokens.at(1));
#else
        QUrlQuery url(QUrl(tokens.at(1)));
#endif

        QTextStream os(socket);

        os.setAutoDetectUnicode(true);
        os << "HTTP/1.0 200 Ok\r\n"
              "Content-Type: text/plain; charset=\"utf-8\"\r\n"
              "Access-Control-Allow-Origin: *\r\n"
              "Access-Control-Allow-Credentials: true\r\n"
              "Access-Control-Allow-Headers: origin, content-type, accept\r\n"
              "\r\n";

        QString ip = "127.0.0.1", destination = "/http";
        quint16 port = 57120;
        QList<QVariant> valeurs = QList<QVariant>() << socket->peerAddress().toString() << socket->peerPort();
        for(quint16 index = 0 ; index < url.queryItems().count() ; index++) {
            QString param = url.queryItems().at(index).first.toLower();
            QString value = url.queryItems().at(index).second.toLower();
            if     (param == "ip")           ip = value;
            else if(param == "destination")  destination = value;
            else if(param == "port")         port = value.toDouble();
            else {
                bool ok = false;
                qreal valueReal = value.toDouble(&ok);
                if(ok)  valeurs.append(valueReal);
                else    valeurs.append(value);
            }
        }
        QString log = MainWindowInterface::main->udp->send(ip, port, destination, valeurs);
        emit(outgoingMessage(log));
        emit(outgoingData(ip, port, destination, valeurs));
        os << log;

        os.flush();
        socket->close();
        if(socket->state() == QTcpSocket::UnconnectedState)
            delete socket;
    }
}
void InterfaceHttpServer::discardClient() {
    QTcpSocket* socket = (QTcpSocket*)sender();
    socket->deleteLater();
}

InterfaceHttp::~InterfaceHttp() {
}
