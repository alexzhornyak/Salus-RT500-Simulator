#include "salusdatamodel.h"

#include <QTime>
#include <QFile>

SalusDataModel::SalusDataModel(QObject *parent/* = nullptr*/):QScxmlCppDataModel(parent) {
    resetToFactorySettings();
}

void SalusDataModel::resetToFactorySettings() {
    resetProgramRecord();

    loadFromJSON(QJsonDocument::fromJson(defaultSettings.toUtf8()));
}

void SalusDataModel::loadFromFile(const QString &sFileName)
{
    QFile loadFile(sFileName);
    if (!loadFile.open(QIODevice::ReadOnly))
        throw std::runtime_error("Can not load [" + sFileName.toStdString() + "]!");
    loadFromJSON(QJsonDocument::fromJson(loadFile.readAll()));
}

void SalusDataModel::saveToFile(const QString &sFileName)
{
    QFile saveFile(sFileName);
    if (!saveFile.open(QIODevice::WriteOnly))
        throw std::runtime_error("Can not save [" + sFileName.toStdString() + "]!");

    QJsonDocument programJson;
    saveToJSON(programJson);
    saveFile.write(programJson.toJson());
}

std::pair<int/*Number*/,ProgramItem> SalusDataModel::currentProgramItem() const
{
    const int i_SATURDAY = 5;
    const ProgramItem *program = _day < i_SATURDAY ? _program[WorkDays] : _program[WeekEnds];
    for (int k=g_MAX_PROGRAM_ITEMS_COUNT - 1; k>=0; k--) {
        const QTime programTime(std::get<ItemType::Hour>(program[k]), std::get<ItemType::Minute>(program[k]));
        const QTime currentTime(_hour, _minute);
        if (currentTime>programTime)
            return std::make_pair(k,program[k]);
        else
            continue;
    }
    return std::make_pair(g_MAX_PROGRAM_ITEMS_COUNT - 1,program[g_MAX_PROGRAM_ITEMS_COUNT - 1]);
}

void SalusDataModel::resetTemperature()
{
    const auto programItem = this->currentProgramItem();
    _temporaryTemperature = std::get<ItemType::Temperature>(programItem.second);
    _temperatureMode = tmdNormal;
}

void SalusDataModel::incrementHour()
{    
    if (_hour<23) {
        _hour++;
    }
}

void SalusDataModel::decrementHour()
{    
    if (_hour>0) {
        _hour--;
    }
}

void SalusDataModel::incrementMinute()
{    
    if (_minute<59) {
        _minute++;
    }
}

void SalusDataModel::decrementMinute()
{    
    if (_minute>0) {
        _minute--;
    }
}

void SalusDataModel::incrementDay()
{
    if (_day<6) {
        _day++;
    }
}

void SalusDataModel::decrementDay()
{
    if (_day>0) {
        _day--;
    }
}

void SalusDataModel::incrementProgramHour()
{
    int iMax = 23;
    if (_programRecord.second < g_MAX_PROGRAM_ITEMS_COUNT - 1) {
        // get Hours in the next element
        iMax = std::get<ItemType::Hour>(_program[_programRecord.first][_programRecord.second + 1]) - 1;
    }

    if (std::get<ItemType::Hour>(_program[_programRecord.first][_programRecord.second])<iMax) {
        std::get<ItemType::Hour>(_program[_programRecord.first][_programRecord.second])++;
    }
}

void SalusDataModel::decrementProgramHour()
{
    int iMin = 0;
    if (_programRecord.second > 0) {
        // get Hours in the previous element
        iMin = std::get<ItemType::Hour>(_program[_programRecord.first][_programRecord.second - 1]) + 1;
    }

    if (std::get<ItemType::Hour>(_program[_programRecord.first][_programRecord.second])>iMin) {
        std::get<ItemType::Hour>(_program[_programRecord.first][_programRecord.second])--;
    }
}

void SalusDataModel::incrementProgramMinute()
{
    if (std::get<ItemType::Minute>(_program[_programRecord.first][_programRecord.second])<59) {
        std::get<ItemType::Minute>(_program[_programRecord.first][_programRecord.second])++;
    }
}

void SalusDataModel::decrementProgramMinute()
{
    if (std::get<ItemType::Minute>(_program[_programRecord.first][_programRecord.second])>0) {
        std::get<ItemType::Minute>(_program[_programRecord.first][_programRecord.second])--;
    }
}

void SalusDataModel::incrementProgramTemperature()
{
    if (std::get<ItemType::Temperature>(_program[_programRecord.first][_programRecord.second])<30) {
        std::get<ItemType::Temperature>(_program[_programRecord.first][_programRecord.second])+=0.5;
    }
}

void SalusDataModel::decrementProgramTemperature()
{
    if (std::get<ItemType::Temperature>(_program[_programRecord.first][_programRecord.second])>0) {
        std::get<ItemType::Temperature>(_program[_programRecord.first][_programRecord.second])-=0.5;
    }
}

void SalusDataModel::incrementProgramNumber()
{
    if (_programRecord.second<g_MAX_PROGRAM_ITEMS_COUNT-1) {
        _programRecord.second++;
    }
}

void SalusDataModel::decrementProgramNumber()
{
    if (_programRecord.second>0) {
        _programRecord.second--;
    }
}

void SalusDataModel::loadFromJSON(const QJsonDocument &program) {

    const auto root = program.object();

    _hour = root["Hour"].toInt();
    _minute = root["Minute"].toInt();
    _day = root["Day"].toInt();

    const auto &workDaysJson = root["WorkDays"].toArray();

    for (int k=0; k<g_MAX_PROGRAM_ITEMS_COUNT; k++) {
        std::get<ItemType::Hour>(_program[WorkDays][k]) = workDaysJson.at(k).toArray().at(ItemType::Hour).toInt();
        std::get<ItemType::Minute>(_program[WorkDays][k]) = workDaysJson.at(k).toArray().at(ItemType::Minute).toInt();
        std::get<ItemType::Temperature>(_program[WorkDays][k]) = workDaysJson.at(k).toArray().at(ItemType::Temperature).toDouble();
    }

    const auto &weekEndsJson = root["WeekEnds"].toArray();

    for (int k=0; k<g_MAX_PROGRAM_ITEMS_COUNT; k++) {
        std::get<ItemType::Hour>(_program[WeekEnds][k]) = weekEndsJson.at(k).toArray().at(ItemType::Hour).toInt();
        std::get<ItemType::Minute>(_program[WeekEnds][k]) = weekEndsJson.at(k).toArray().at(ItemType::Minute).toInt();
        std::get<ItemType::Temperature>(_program[WeekEnds][k]) = weekEndsJson.at(k).toArray().at(ItemType::Temperature).toDouble();
    }
}

void SalusDataModel::saveToJSON(QJsonDocument &program) {
    QJsonObject root;

    QJsonArray workDaysJson;
    for (int k=0; k<g_MAX_PROGRAM_ITEMS_COUNT; k++) {
        QJsonArray item;
        item.append(std::get<ItemType::Hour>(_program[WorkDays][k]));
        item.append(std::get<ItemType::Minute>(_program[WorkDays][k]));
        item.append(std::get<ItemType::Temperature>(_program[WorkDays][k]));
        workDaysJson.append(item);
    }

    root["WorkDays"] = workDaysJson;

    QJsonArray weekEndsJson;
    for (int k=0; k<g_MAX_PROGRAM_ITEMS_COUNT; k++) {
        QJsonArray item;
        item.append(std::get<ItemType::Hour>(_program[WeekEnds][k]));
        item.append(std::get<ItemType::Minute>(_program[WeekEnds][k]));
        item.append(std::get<ItemType::Temperature>(_program[WeekEnds][k]));
        weekEndsJson.append(item);
    }

    root["WeekEnds"] = weekEndsJson;

    root["Hour"] = _hour;
    root["Minute"] = _minute;
    root["Day"] = _day;

    program.setObject(root);
}
