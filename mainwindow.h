#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPropertyAnimation>

#include "SalusRT500Logic.h"
#include "salusdatamodel.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void onBtnInput_pressed();

    void onBtnInput_released();

    void on_actionInputs_Panel_triggered(bool checked);

    void on_CheckPower_clicked(bool checked);

    void on_SpinTemperature_valueChanged(int arg1);

    void on_actionLoad_Program_triggered();

    void on_actionSave_Program_triggered();

private:
    Ui::MainWindow *ui;    

    SalusRT500Logic *_machine = nullptr;
    SalusDataModel *_dataModel = nullptr;

    QPropertyAnimation *_animationFlame = nullptr;
    QPropertyAnimation *_animationFreeze = nullptr;

    static QString dayOfWeekToDisplayText(const int day);
};

#endif // MAINWINDOW_H
