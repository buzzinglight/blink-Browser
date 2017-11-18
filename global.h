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

#ifndef GLOBAL_H
#define GLOBAL_H

#include <QVariant>

class SenderInterface {
public:
    virtual QString send(const QString &ip, quint16 port, const QString &destination, const QList<QVariant> &valeurs) = 0;
    virtual void setPort(quint16 port) = 0;
};
class MainWindowInterface {
public:
    static MainWindowInterface *main;
    SenderInterface *udp;
    SenderInterface *http;
    virtual void setUrl(QString url = "") = 0;
};

#endif // GLOBAL_H
