#ifndef ICON_GROUPBOX_H
#define ICON_GROUPBOX_H

#include <QGroupBox>
#include <QLabel>
#include <QHBoxLayout>

class c_icon_groupbox : public QGroupBox
{
    Q_OBJECT
public:
    explicit c_icon_groupbox(QWidget *parent = 0);
    ~c_icon_groupbox();
    void set_icon(const char *filename);
    void setLayout(QLayout *layout);

private:
    QPixmap *mp_group_icon;
    QLabel *mp_icon_Label;
    QGridLayout *mp_grid_Layout;

signals:

public slots:

};

#endif // ICON_GROUPBOX_H
