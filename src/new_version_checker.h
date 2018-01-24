// ---------------------------------------------------------------------
// Copyright (C) 2015 Chris Garry
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>
// ---------------------------------------------------------------------


#ifndef NEW_VERSION_CHECKER_H
#define NEW_VERSION_CHECKER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QList>


class c_new_version_checker : public QObject
{
    Q_OBJECT
public:
    explicit c_new_version_checker(QObject *parent = 0, QString version = "1.0.0");
    void check();
    static bool compare_version_strings(QString current_ver, QString new_ver);

signals:
    void new_version_available_signal(QString new_version);

public slots:
    void finished(QNetworkReply * reply);

private:
    QString m_version;
    QNetworkAccessManager *net_access_manager;
    QString rstrip(const QString& str);
    static QList<int> get_version_from_string(const QString &ver_string);

};

#endif // NEW_VERSION_CHECKER_H
