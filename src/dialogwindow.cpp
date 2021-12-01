#include "dialogwindow.h"

DialogWindow::DialogWindow(QString text, QWidget *parent)
    : QDialog(parent)
{
    QLabel *information = new QLabel();
    information->setFont(QFont("Times", 24));
    information->setAlignment(Qt::AlignLeft);
    information->setAlignment(Qt::AlignVCenter);
    information->setWordWrap(true);
    information->setText(text);
    QGridLayout *work_place = new QGridLayout(this);
    work_place->addWidget(information);
}

DialogWindow::~DialogWindow()
{

}
