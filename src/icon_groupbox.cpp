#include <QDebug>
#include "icon_groupbox.h"


c_icon_groupbox::c_icon_groupbox(QWidget *parent) :
    QGroupBox(parent),
    mp_group_icon(nullptr)
{
    mp_grid_Layout = new QGridLayout;
    mp_grid_Layout->setMargin(0);
    mp_grid_Layout->setSpacing(0);

    mp_icon_Label = new QLabel();
    QHBoxLayout *icon_hlayout = new QHBoxLayout;
    icon_hlayout->setMargin(0);
    icon_hlayout->setSpacing(0);
    icon_hlayout->addSpacing(15);
    icon_hlayout->addWidget(mp_icon_Label, 0, Qt::AlignRight | Qt::AlignTop);
    icon_hlayout->addSpacing(5);

    mp_grid_Layout->addLayout(icon_hlayout, 0, 1, Qt::AlignRight | Qt::AlignTop);
    QGroupBox::setLayout(mp_grid_Layout);
}


c_icon_groupbox::~c_icon_groupbox()
{
    if (mp_group_icon != nullptr)
    {
        delete mp_group_icon;
    }
}


void c_icon_groupbox::set_icon(const char *filename)
{
    mp_group_icon = new QPixmap(filename);
    mp_icon_Label->setPixmap(*mp_group_icon);
}


void c_icon_groupbox::setLayout(QLayout *layout) {
    mp_grid_Layout->addLayout(layout, 0, 0);
}
