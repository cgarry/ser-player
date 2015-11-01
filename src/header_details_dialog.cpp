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

#include <Qt>
#include <QDebug>
#include <QHBoxLayout>
#include <QTextEdit>

#include "pipp_ser.h"  // colour IDs
#include "header_details_dialog.h"


c_header_details_dialog::c_header_details_dialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("SER File Details"));
    QDialog::setWindowFlags(QDialog::windowFlags() & ~Qt::WindowContextHelpButtonHint);

    mp_header_details_Tedit = new QTextEdit;
    mp_header_details_Tedit->setReadOnly(true);
    QPalette p = mp_header_details_Tedit->palette();
    p.setColor(QPalette::Base, QColor(255, 248, 220));
    mp_header_details_Tedit->setPalette(p);
    mp_header_details_Tedit->setWordWrapMode(QTextOption::NoWrap);
    mp_header_details_Tedit->append("No file");

    QHBoxLayout *header_details_Hlayout = new QHBoxLayout;
    header_details_Hlayout->setMargin(0);
    header_details_Hlayout->setSpacing(0);
    header_details_Hlayout->addWidget(mp_header_details_Tedit);
    
    setLayout(header_details_Hlayout);
    layout()->setSizeConstraint(QLayout::SetFixedSize);
}


void c_header_details_dialog::set_details(
        QString filename,
        int64_t filesize,
        QString file_id,
        int lu_id,
        int colour_id,
        int little_endian,
        int image_width,
        int image_height,
        int pixel_depth,
        int frame_count,
        QString observer,
        QString instrument,
        QString telescope,
        uint64_t date_time,
        uint64_t date_time_utc,
        QString timestamp_info)
{
    QString colour_id_string;
    switch (colour_id) {
    case COLOURID_MONO:
        colour_id_string = "MONO";
        break;
    case COLOURID_BAYER_RGGB:
        colour_id_string = "RGGB";
        break;
    case COLOURID_BAYER_GRBG:
        colour_id_string = "GRBG";
        break;
    case COLOURID_BAYER_GBRG:
        colour_id_string = "GBRG";
        break;
    case COLOURID_BAYER_BGGR:
        colour_id_string = "BGGR";
        break;
    case COLOURID_BAYER_CYYM:
        colour_id_string = "CYYM";
        break;
    case COLOURID_BAYER_YCMY:
        colour_id_string = "YCMY";
        break;
    case COLOURID_BAYER_YMCY:
        colour_id_string = "YMCY";
        break;
    case COLOURID_BAYER_MYYC:
        colour_id_string = "MYYC";
        break;
    case COLOURID_RGB:
        colour_id_string = "RGB";
        break;
    case COLOURID_BGR:
        colour_id_string = "BGR";
        break;
    }

    mp_header_details_Tedit->clear();
    mp_header_details_Tedit->append(tr("Filename: %1").arg(filename));
    mp_header_details_Tedit->append(tr("Filesize: %1 bytes").arg(filesize));
    mp_header_details_Tedit->append("");
    mp_header_details_Tedit->append(tr("Header Details:"));
    mp_header_details_Tedit->append(tr(" * FileId: %1").arg(file_id));
    mp_header_details_Tedit->append(tr(" * LuID: %1").arg(lu_id));
    mp_header_details_Tedit->append(tr(" * ColorID: %1 (%2)").arg(colour_id).arg(colour_id_string));
    mp_header_details_Tedit->append(tr(" * LittleEndian: %1").arg(little_endian));
    mp_header_details_Tedit->append(tr(" * ImageWidth: %1").arg(image_width));
    mp_header_details_Tedit->append(tr(" * ImageHeight: %1").arg(image_height));
    mp_header_details_Tedit->append(tr(" * PixelDepth: %1").arg(pixel_depth));
    mp_header_details_Tedit->append(tr(" * FrameCount: %1").arg(frame_count));
    mp_header_details_Tedit->append(tr(" * Observer: %1").arg(observer));
    mp_header_details_Tedit->append(tr(" * Instrument: %1").arg(instrument));
    mp_header_details_Tedit->append(tr(" * Telescope: %1").arg(telescope));
    mp_header_details_Tedit->append(tr(" * DateTime: 0x%1").arg(date_time, 1, 16));
    mp_header_details_Tedit->append(tr(" * DateTime_UTC: 0x%1").arg(date_time_utc, 1, 16));
    mp_header_details_Tedit->append("");
    mp_header_details_Tedit->append(tr("Timestamps:"));
    mp_header_details_Tedit->append(timestamp_info);

    // Resize to fit text
    mp_header_details_Tedit->document()->adjustSize();
    mp_header_details_Tedit->setFixedSize(mp_header_details_Tedit->document()->size().toSize().width() + 20,
                                          mp_header_details_Tedit->document()->size().toSize().height() + 20);

}
