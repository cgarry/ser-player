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


#include <QDateTime>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMessageBox>
#include <QDebug>
#include "new_version_checker.h"
#include "persistent_data.h"

c_new_version_checker::c_new_version_checker(QObject *parent, QString version) :
    QObject(parent),
    m_version(version)
{
    net_access_manager = new QNetworkAccessManager(this);
    connect(net_access_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)));
}


void c_new_version_checker::check()
{
    const uint secs_between_update_checks = 12 * 60 * 60;  // 12 hours minimum between update checks
    uint current_time = QDateTime::currentDateTime().toTime_t();
    if (current_time > c_persistent_data::m_last_ver_check_time + secs_between_update_checks) {
        net_access_manager->get(QNetworkRequest(QUrl("https://raw.githubusercontent.com/cgarry/ser-player/master/latest_version.txt")));
    }
}


void c_new_version_checker::finished(QNetworkReply *reply)
{
    int http_status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (http_status == 301 || http_status == 302  || http_status == 302)
    {
        // Handle redirect
        net_access_manager->get(QNetworkRequest(QUrl(reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString())));
        return;
    }
    else if (http_status == 200)
    {
        // Grab time of update check
        c_persistent_data::m_last_ver_check_time = QDateTime::currentDateTime().toTime_t();

        // Get the received file in a string
        QString rxd_file(reply->readAll());
        rxd_file.remove("\r");

        // Spilt the file into lines
        QStringList rxd_lines = rxd_file.split("\n");

        // Check if new version is newer than current version
        bool new_version_available = compare_version_strings(m_version, rxd_lines[0]);

        if (new_version_available) {
            new_version_available_signal(rxd_lines[0]);
        }
    }

    this->deleteLater();
}


bool c_new_version_checker::compare_version_strings(QString current_version, QString new_version)
{
    // Extract new version number
    bool new_dev_build;
    QList<int> new_ver = get_version_from_string(new_version, &new_dev_build);
    if (new_ver.size() < 3) {
        // Not a valid version file - give up
        return false;
    }

    // Extract current version number
    bool cur_dev_build;
    QList<int> cur_ver = get_version_from_string(current_version, &cur_dev_build);
    if (cur_ver.size() < 3) {
        // Not a valid version file - give up
        return false;
    }

    // Check if new version is newer than current version
    bool new_version_is_newer = false;
    if (new_ver[0] > cur_ver[0]) {
        new_version_is_newer = true;
    }

    if (new_ver[0] == cur_ver[0] && new_ver[1] > cur_ver[1]) {
        new_version_is_newer = true;
    }

    if (new_ver[0] == cur_ver[0] && new_ver[1] == cur_ver[1] && new_ver[2] > cur_ver[2]) {
        new_version_is_newer = true;
    }

    if (new_ver[0] == cur_ver[0] && new_ver[1] == cur_ver[1] && new_ver[2] == cur_ver[2] && cur_dev_build) {
        // Current version is a dev build of released version
        new_version_is_newer = true;
    }

    return new_version_is_newer;
}


QString c_new_version_checker::rstrip(const QString& str)
{
    int n = str.size() - 1;
    for (; n >= 0; --n) {
        if (!str.at(n).isSpace()) {
            return str.left(n + 1);
        }
    }

    return "";
}

QList<int> c_new_version_checker::get_version_from_string(const QString &ver_string, bool* dev_build) {
    QList<int> ret;  // Empty list
    *dev_build = false;

    // Extract new version number
    QStringList version = ver_string.split(".");
    if (!version[0].startsWith("v") || version.size() < 3) {
        // Not a valid version file
        return ret;
    }

    version[0].remove("v");
    int new_ver[3];
    bool okay;
    new_ver[0] = version[0].toInt(&okay);
    if (!okay) {
        return ret;
    }

    new_ver[1] = version[1].toInt(&okay);
    if (!okay) {
        return ret;
    }

    new_ver[2] = version[2].toInt(&okay);
    if (!okay) {
        return ret;
    }

    ret.append(new_ver[0]);
    ret.append(new_ver[1]);
    ret.append(new_ver[2]);
    if (version.size()> 3) {
        *dev_build = true;
    }

    return ret;
}
