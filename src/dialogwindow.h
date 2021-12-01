#ifndef DIALOGWINDOW_H
#define DIALOGWINDOW_H

#include <QDialog>
#include <QLabel>
#include <QGridLayout>
#include <QDesktopWidget>
#include <QApplication>

class DialogWindow: public QDialog
{
    Q_OBJECT

public:
    DialogWindow(QString text = nullptr, QWidget *parent = nullptr);
    ~DialogWindow();
};

#endif // DIALOGWINDOW_H
