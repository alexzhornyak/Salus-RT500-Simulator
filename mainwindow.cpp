#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>

/* this value seems to be the most elegant imitation of button is pressed */
const int g_BTN_PRESSED_OFFSET = 2;

/*************************************************************************/
/*                              MainWindow                               */
/*************************************************************************/
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /* imitating flame animation */
    _animationFlame = new QPropertyAnimation(ui->labelFlame, "geometry");
    auto flameGeometry = ui->labelFlame->geometry();
    _animationFlame->setStartValue(flameGeometry);
    flameGeometry.setTop(flameGeometry.top() - 5);
    _animationFlame->setEndValue(flameGeometry);
    _animationFlame->setDuration(500);
    _animationFlame->setLoopCount(-1);
    _animationFlame->setEasingCurve(QEasingCurve::InOutQuad);

    /* imitating freeze mode animation */
    _animationFreeze = new QPropertyAnimation(ui->labelFreeze, "geometry");
    auto freezeGeometry = ui->labelFreeze->geometry();
    _animationFreeze->setStartValue(freezeGeometry);
    freezeGeometry.setTop(freezeGeometry.top() + 10);
    freezeGeometry.setLeft(freezeGeometry.left() + 10);
    freezeGeometry.setRight(freezeGeometry.right() - 10);
    freezeGeometry.setBottom(freezeGeometry.bottom() - 10);
    _animationFreeze->setEndValue(freezeGeometry);
    _animationFreeze->setDuration(1000);
    _animationFreeze->setLoopCount(-1);

    /* all outputs are invisible by default */
    for (auto child : ui->frameOutputs->children()) {
        QWidget *widget = qobject_cast<QWidget*>(child);
        if (widget) {
            widget->setVisible(false);
        }
    }

    /* UI Setup */
    connect(ui->BtnHighLight, &QPushButton::pressed, this, &MainWindow::onBtnInput_pressed);
    connect(ui->BtnHighLight, &QPushButton::released, this, &MainWindow::onBtnInput_released);

    connect(ui->BtnUp, &QPushButton::pressed, this, &MainWindow::onBtnInput_pressed);
    connect(ui->BtnUp, &QPushButton::released, this, &MainWindow::onBtnInput_released);

    connect(ui->BtnDown, &QPushButton::pressed, this, &MainWindow::onBtnInput_pressed);
    connect(ui->BtnDown, &QPushButton::released, this, &MainWindow::onBtnInput_released);

    connect(ui->BtnReset, &QPushButton::pressed, this, &MainWindow::onBtnInput_pressed);
    connect(ui->BtnReset, &QPushButton::released, this, &MainWindow::onBtnInput_released);

    connect(ui->BtnSet, &QPushButton::pressed, this, &MainWindow::onBtnInput_pressed);
    connect(ui->BtnSet, &QPushButton::released, this, &MainWindow::onBtnInput_released);

    connect(ui->BtnSelect, &QPushButton::pressed, this, &MainWindow::onBtnInput_pressed);
    connect(ui->BtnSelect, &QPushButton::released, this, &MainWindow::onBtnInput_released);

    /* Actions */
    this->addAction(ui->actionInputs_Panel);
    this->addAction(ui->actionLoad_Program);
    this->addAction(ui->actionSave_Program);

    /* StateMachine Setup */
    _dataModel = new SalusDataModel;
    _machine = new SalusRT500Logic(this);
    _machine->setDataModel(_dataModel);

    /* Lit All Segments during Test */
    _machine->connectToState("Test", [this](bool active){
        ui->labelDayOfWeek->setText("MTUWTHFSASU");
        ui->labelProgramNo->setText("H H H H H");
        ui->labelAmPm->setText("AM");
        ui->lcdHour->display(18);
        ui->lcdMinute->display(88);
        ui->lcdTempInt->display(88);
        ui->lcdTempFrac->display(8);

        for (auto child : ui->frameOutputs->children()) {
            QWidget *widget = qobject_cast<QWidget*>(child);
            if (widget) {
                widget->setVisible(active);
            }
        }
    });

    /* State Connections */
    _machine->connectToState("Operative", [this](bool active){            
           ui->labelAmPm->setVisible(active);
           ui->labelSemicolon->setVisible(active);

           /* Do not Allow users to save & load not in power off mode */
           ui->actionLoad_Program->setVisible(active);
           ui->actionSave_Program->setVisible(active);

           active ? _animationFlame->start() : _animationFlame->stop();
           active ? _animationFreeze->start() : _animationFreeze->stop();
    });

    _machine->connectToState("DisplayNormal", [this](bool active){
        if (active) {
            ui->lcdTempInt->display(ui->SpinTemperature->value());
            ui->lcdTempFrac->display(0);

            const auto programItem = _dataModel->currentProgramItem();
            const int iProgramNo = programItem.first;

            switch (_dataModel->temperatureMode()) {
            case tmdNormal:
                ui->labelFlame->setVisible(ui->SpinTemperature->value() < std::get<ItemType::Temperature>(programItem.second));
                break;
            case tmdTemporary:
                ui->labelFlame->setVisible(ui->SpinTemperature->value() < _dataModel->temporaryTemperature());
                break;
            case tmdFreeze:
                ui->labelFreeze->setVisible(true);
                ui->labelFlame->setVisible(ui->SpinTemperature->value() < _dataModel->freezeTemperature());
                break;
            }

            ui->labelProgramNo->setText(QString(" ").repeated(iProgramNo) + QString::number(iProgramNo+1));

        } else {
            ui->labelFlame->setVisible(false);
            ui->labelFreeze->setVisible(false);
        }
    });

    _machine->connectToState("HighLight_On", [this](bool active){
        ui->frameOutputs->setStyleSheet(
                    active ?
                    "background-color: rgba(0, 170, 255, 100%);\nborder-radius: 10px;" :
                    "background-color: rgba(135, 177, 160, 100%);\nborder-radius: 10px;");
    });

    _machine->connectToState("Temperature_On", [this](bool active){
        ui->lcdTempInt->setVisible(active);
        ui->lcdTempFrac->setVisible(active);
        ui->labelDot->setVisible(active);
        ui->labelCelcium->setVisible(active);
    });

    _machine->connectToState("TemporaryFlashOn", [this](bool active){
        if (active) {
            ui->lcdTempInt->display(_dataModel->getTemporaryTemperatureInt());
            ui->lcdTempFrac->display(_dataModel->getTemporaryTemperatureFrac());
        }
    });

    _machine->connectToState("Hour_On", [this](bool active){
        ui->lcdHour->setVisible(active);
    });

    _machine->connectToState("Minute_On", [this](bool active){
        ui->lcdMinute->setVisible(active);
    });

    _machine->connectToState("Set_On", [this](bool active){
        ui->labelSet->setVisible(active);
    });

    _machine->connectToState("Day_On", [this](bool active){
        ui->labelDayOfWeek->setVisible(active);
    });

    _machine->connectToState("ProgramNumber_On", [this](bool active){
        ui->labelProgramNo->setVisible(active);
    });

    /* Event Connections */
    _machine->connectToEvent("Update.Time", [this](const QScxmlEvent &){
        const int iHour = _dataModel->hour();
        ui->lcdHour->display(iHour > 12 ? (iHour - 12) : iHour);
        ui->lcdMinute->display(QString().sprintf("%02d",_dataModel->minute()));
        ui->labelAmPm->setText(iHour>12 ? "  PM" : "AM");
        ui->labelDayOfWeek->setText(dayOfWeekToDisplayText(_dataModel->day()));
    });

    _machine->connectToEvent("Update.Program", [this](const QScxmlEvent &){
        ui->labelDayOfWeek->setText(_dataModel->getProgramWeekType()==WorkDays ? "MTUWTHF" : "       SASU");

        const int iProgramNo = _dataModel->getProgramNumber();
        const double dTemp = std::get<ItemType::Temperature>(_dataModel->programRecordToProgramItem());
        const int iHour = std::get<ItemType::Hour>(_dataModel->programRecordToProgramItem());
        const int iMinute = std::get<ItemType::Minute>(_dataModel->programRecordToProgramItem());

        ui->lcdHour->display(iHour > 12 ? (iHour - 12) : iHour);
        ui->lcdMinute->display(QString().sprintf("%02d", iMinute));
        ui->labelAmPm->setText(iHour>12 ? "  PM" : "AM");

        ui->lcdTempInt->display(floor(dTemp));
        ui->lcdTempFrac->display(floor((dTemp - floor(dTemp))*10.0f));

        ui->labelProgramNo->setText(QString(" ").repeated(iProgramNo) + QString::number(iProgramNo+1));
    });

    /* when all setup procedures are finished starting State Machine */
    _machine->start();
}

MainWindow::~MainWindow()
{
    if (_dataModel) {
        delete _dataModel;
        _dataModel = nullptr;
    }

    if (_machine) {
        delete _machine;
        _machine = nullptr;
    }

    if (_animationFlame) {
        delete _animationFlame;
        _animationFlame = nullptr;
    }

    if (_animationFreeze) {
        delete _animationFreeze;
        _animationFreeze = nullptr;
    }

    delete ui;
}

void MainWindow::on_CheckPower_clicked(bool checked)
{
    _machine->submitEvent("CheckPower",checked);
}

void MainWindow::onBtnInput_pressed()
{
    /* when pressed, set a flag, later we will use it for handle CTRL case */
    auto btn = qobject_cast<QPushButton*>(sender());
    if (btn) {
        if (!btn->property("Pressed").toBool()) {
            const auto geometry = btn->geometry();
            btn->setGeometry(geometry.left() + g_BTN_PRESSED_OFFSET, geometry.top() + g_BTN_PRESSED_OFFSET, geometry.width(), geometry.height());
            btn->setProperty("Pressed", true);
            _dataModel->inputs["Inp." + btn->objectName()] = true;
            _machine->submitEvent("Inp." + btn->objectName(),true);
        }
    }
}

void MainWindow::onBtnInput_released()
{
    /* when released with CTRL button, then do not send message to State Machine */
    auto btn = qobject_cast<QPushButton*>(sender());
    if (btn) {
        const bool bCtrlPressed = QApplication::keyboardModifiers() & Qt::ControlModifier;
        if (btn->property("Pressed").toBool() && !bCtrlPressed) {
            const auto geometry = btn->geometry();
            btn->setGeometry(geometry.left() - g_BTN_PRESSED_OFFSET, geometry.top() - g_BTN_PRESSED_OFFSET, geometry.width(), geometry.height());
            btn->setProperty("Pressed", false);
            _dataModel->inputs["Inp." + btn->objectName()] = false;
            _machine->submitEvent("Inp." + btn->objectName(),false);
        }
    }
}

void MainWindow::on_actionInputs_Panel_triggered(bool checked)
{
    ui->frameInputs->setVisible(checked);
}

void MainWindow::on_SpinTemperature_valueChanged(int arg1)
{
    // this handler can be triggered before state machine is running, so require checks before
    if (_machine && _machine->isRunning()) {
        // not necessary to transmit Spin Value, so doing it just for future possible needs
        _machine->submitEvent("SpinTemperature",arg1);
    }
}


void MainWindow::on_actionLoad_Program_triggered()
{
    try {
        const QString fileName = QFileDialog::getOpenFileName(this,
                                                              "Load Program", "",
                                                              "Program Files (*.json);;All Files (*)");
        if (!fileName.isEmpty())
            _dataModel->loadFromFile(fileName);

        _machine->submitEvent("Program.Reload");

    } catch (std::exception &e) {
        qWarning(e.what());
    }
}

void MainWindow::on_actionSave_Program_triggered()
{
    try {
        const QString fileName = QFileDialog::getSaveFileName(this,
                                                              "Save Program", "",
                                                              "Program Files (*.json);;All Files (*)");
        if (!fileName.isEmpty())
            _dataModel->saveToFile(fileName);

    } catch (std::exception &e) {
        qWarning(e.what());
    }
}

QString MainWindow::dayOfWeekToDisplayText(const int day)
{
    // MTUWTHFSASU
    switch (day) {
    case 0: return "M";
    case 1: return " TU";
    case 2: return "   W";
    case 3: return "    TH";
    case 4: return "      F";
    case 5: return "       SA";
    case 6: return "         SU";
    }
    throw std::exception("Day is out of range!");
}
