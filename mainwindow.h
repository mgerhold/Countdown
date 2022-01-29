#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QTime>
#include <QSettings>
#include "TimeSpan.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QSpinBox;
class QLineEdit;
class QCheckBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_destination_button_clicked();

    void on_start_button_clicked();

    void on_timer_timed_out();

    void on_spinbox_valueChanged(int);

    void on_finish_message_checkbox_toggled(bool);

    void on_format_string_changed(QString);

    void on_pause_button_clicked();

    void on_reset_button_clicked();

private:
    void setSpinboxesEnabled(bool enabled) const;
    void refreshStartButton();
    void refreshPauseButton();
    void refreshResetButton();
    void refreshDestinationWidgets();
    void displayTimeSpan(const TimeSpan& timeSpan);
    void readSetting(const QString& settingKey, QSpinBox& spinBox);
    void readSetting(const QString& settingKey, QLineEdit& lineEdit);
    void readSetting(const QString& settingKey, QCheckBox& checkBox);
    void saveFinishMessageSettings();
    TimeSpan calculateRemainingDuration() const;
    void writeToFile(const QString& filename, const TimeSpan& timeSpan);
    void writeToFile(const QString& filename, const QString& contents);
    [[nodiscard]] QString getFilename() const;

private:
    Ui::MainWindow *ui;
    TimeSpan mTotalDuration;
    QTime mStartTime;
    QTime mStartOfPause;
    QTimer mTimer;
    QSettings mSettings;
    bool mCountdownWasStarted{ false };
};
#endif // MAINWINDOW_H
