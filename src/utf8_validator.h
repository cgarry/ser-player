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


#ifndef UTF8_VALIDATOR_H
#define UTF8_VALIDATOR_H

#include <QValidator>
#include <QString>


class c_utf8_validator : public QValidator
{
    Q_OBJECT

public:
    c_utf8_validator(QObject *parent = 0);

    QValidator::State validate(QString& input, int&) const;
};


#endif // UTF8_VALIDATOR_H
