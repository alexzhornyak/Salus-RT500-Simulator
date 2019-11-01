#ifndef SALUSDATAMODEL_H
#define SALUSDATAMODEL_H

#include <tuple>

#include <QScxmlCppDataModel>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

static const int g_MAX_PROGRAM_ITEMS_COUNT = 5;

typedef std::tuple<int/*Hour*/,int/*Minute*/,double/*Temperature*/> ProgramItem;
typedef enum {Hour,Minute,Temperature} ItemType;

typedef enum {WorkDays, WeekEnds} WeekType;
static const int g_MAX_WEEK_TYPE_COUNT = 2;

typedef enum {tmdNormal, tmdTemporary, tmdFreeze} TemperatureMode;

class SalusDataModel : public QScxmlCppDataModel
{
    Q_OBJECT
    Q_SCXML_DATAMODEL

public:

    SalusDataModel(QObject *parent = nullptr);

    /*
     * we will remember the current state of inputs here
     * later we will use it, when we check a couple of buttons pressed
    */
    QVariantMap inputs;

    /* Program Record depending on current day and time settings */
    std::pair<int/*Number*/,ProgramItem> currentProgramItem() const;

    /* Temporary Temperature Setup */
    inline int getTemporaryTemperatureInt() { return floor(_temporaryTemperature); }
    inline int getTemporaryTemperatureFrac() { return (_temporaryTemperature - getTemporaryTemperatureInt())*10.0f; }

    inline void incrementTemporaryTemperature() { if (_temporaryTemperature < 30.0) _temporaryTemperature+= 0.5; }
    inline void decrementTemporaryTemperature() { if (_temporaryTemperature < 30.0) _temporaryTemperature-= 0.5; }

    inline const int &hour() const { return _hour; }
    inline const int &minute() const { return _minute; }
    inline const int &day() const { return _day; }

    inline const TemperatureMode temperatureMode() const { return _temperatureMode; }
    inline const double temporaryTemperature() const { return _temporaryTemperature; }
    inline const double freezeTemperature() const { return _freezeTemperature; }
    inline const bool isFreezeMode() const { return _temperatureMode==tmdFreeze; }

    void resetTemperature();
    inline void setTemporaryTemperatureMode() { _temperatureMode=tmdTemporary; }
    inline void flipFreezeTemperatureMode() { _temperatureMode=_temperatureMode==tmdFreeze ? tmdNormal : tmdFreeze; }

    /* Day and Time Setup */
    void incrementHour();
    void decrementHour();

    void incrementMinute();
    void decrementMinute();

    void incrementDay();
    void decrementDay();

    /* Program Setup */

    // there can be 5/2 mode or 7 mode
    // in this program we are simulating 5/2 mode, so just simply invert
    inline void incrementProgramDay() { _programRecord.first = _programRecord.first == WorkDays ? WeekEnds : WorkDays; }
    inline void decrementProgramDay() { incrementProgramDay(); }

    void incrementProgramHour();
    void decrementProgramHour();

    void incrementProgramMinute();
    void decrementProgramMinute();

    void incrementProgramTemperature();
    void decrementProgramTemperature();

    void incrementProgramNumber();
    void decrementProgramNumber();

    inline void resetProgramRecord() { _programRecord = std::make_pair(WorkDays,0); }
    inline const WeekType getProgramWeekType() const { return _programRecord.first; }
    inline const int getProgramNumber() const { return _programRecord.second; }
    inline const ProgramItem &programRecordToProgramItem() const {
        return _program[_programRecord.first][_programRecord.second]; }

    /* HARD RESET */
    void resetToFactorySettings();

    void loadFromFile(const QString &sFileName);
    void saveToFile(const QString &sFileName);

private:

    ProgramItem _program[g_MAX_WEEK_TYPE_COUNT][g_MAX_PROGRAM_ITEMS_COUNT];

    int _hour = 12; // 0...23
    int _minute = 0; // 0...59
    int _day = 0; // Day of week: 0 ... 6

    TemperatureMode _temperatureMode = tmdFreeze;
    double _temporaryTemperature = 15.0f;
    const double _freezeTemperature = 5.0f;

    std::pair<WeekType,int/*ProgramNumber*/> _programRecord;

    /* Serialization */
    void loadFromJSON(const QJsonDocument &program);

    void saveToJSON(QJsonDocument &program);

    /* Default */
    const QString defaultSettings =  "{\n"
                                     "  \"Hour\" : 12,\n"
                                     "  \"Minute\" : 0,\n"
                                     "  \"Day\": 0,\n"
                                     "  \"WorkDays\": [\n"
                                     "    [ 6, 0, 22 ],\n"
                                     "    [ 12, 0, 22 ],\n"
                                     "    [ 16, 0, 22 ],\n"
                                     "    [ 18, 0, 22 ],\n"
                                     "    [ 22, 0, 22 ]\n"
                                     "  ],\n"
                                     "  \"WeekEnds\": [\n"
                                     "    [ 6, 0, 22 ],\n"
                                     "    [ 12, 0, 22 ],\n"
                                     "    [ 16, 0, 22 ],\n"
                                     "    [ 18, 0, 22 ],\n"
                                     "    [ 22, 0, 22 ]\n"
                                     "  ]\n"
                                     "}";
};

#endif // SALUSDATAMODEL_H
