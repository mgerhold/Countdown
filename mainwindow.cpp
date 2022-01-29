#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <chrono>
#include <QString>

static const QString filenameSettingKey = QStringLiteral("DestinationFilename");
static const QString hoursSettingKey = QStringLiteral("Hours");
static const QString minutesSettingKey = QStringLiteral("Minutes");
static const QString secondsSettingKey = QStringLiteral("Seconds");
static const QString finishMessageEnabledSettingKey = QStringLiteral("UseFinishMessage");
static const QString finishMessageSettingKey = QStringLiteral("FinishMessage");
static const QString formatStringSettingKey = QStringLiteral("FormatString");

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      mSettings{ "coder2k", "coder2countdown" } {
    ui->setupUi(this);
    setFixedSize(sizeHint());
    /* connect(addressOfSender,
     *         signalOfSender as pointer to member function,
     *         address of receiver,
     *         slot as pointer to member function) */
    connect(&mTimer, &QTimer::timeout, this, &MainWindow::on_timer_timed_out);
    readSetting(filenameSettingKey, *ui->filename_edit);
    refreshStartButton();
    refreshPauseButton();
    refreshDestinationWidgets();
    refreshResetButton();

    readSetting(hoursSettingKey, *ui->hours_spinbox);
    readSetting(minutesSettingKey, *ui->minutes_spinbox);
    readSetting(secondsSettingKey, *ui->seconds_spinbox);

    readSetting(finishMessageSettingKey, *ui->finish_message_edit);
    readSetting(finishMessageEnabledSettingKey, *ui->finish_message_checkbox);

    readSetting(formatStringSettingKey, *ui->format_string_edit);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_destination_button_clicked() {
    auto filename = QFileDialog::getSaveFileName(this, "Choose destination", "", "Text files (*.txt);;All Files (*)");
    if (filename.isEmpty()) {
        return;
    }
    mSettings.setValue(filenameSettingKey, filename);
    ui->filename_edit->setText(filename);
    refreshStartButton();
}

void MainWindow::on_start_button_clicked() {
    using namespace std::chrono_literals;
    Q_ASSERT(not getFilename().isEmpty());
    mTotalDuration = TimeSpan{ ui->hours_spinbox->value(), ui->minutes_spinbox->value(), ui->seconds_spinbox->value() };
    mTimer.setInterval(250ms);
    mTimer.start();
    mSettings.setValue(hoursSettingKey, mTotalDuration.hours);
    mSettings.setValue(minutesSettingKey, mTotalDuration.minutes);
    mSettings.setValue(secondsSettingKey, mTotalDuration.seconds);
    setSpinboxesEnabled(false);
    refreshStartButton();
    writeToFile(getFilename(), mTotalDuration);
    mStartTime = QTime::currentTime();
    saveFinishMessageSettings();
    mSettings.setValue(formatStringSettingKey, ui->format_string_edit->text());
    mCountdownWasStarted = true;
    refreshPauseButton();
    refreshDestinationWidgets();
    refreshResetButton();
}

void MainWindow::on_timer_timed_out() {
    const auto remainingDuration = calculateRemainingDuration();
    displayTimeSpan(remainingDuration);
    if (remainingDuration <= TimeSpan::zero()) {
        mTimer.stop();
        displayTimeSpan(mTotalDuration);
        setSpinboxesEnabled(true);
        refreshStartButton();
        mCountdownWasStarted = false;
        refreshPauseButton();
        refreshDestinationWidgets();
        refreshResetButton();
        if (ui->finish_message_checkbox->isChecked()) {
            writeToFile(getFilename(), ui->finish_message_edit->text());
            return;
        }
    }
    writeToFile(getFilename(), remainingDuration);
}

void MainWindow::on_spinbox_valueChanged(int) {
    refreshStartButton();
}

void MainWindow::on_finish_message_checkbox_toggled(bool checked) {
    saveFinishMessageSettings();
    ui->finish_message_edit->setEnabled(checked);
}

void MainWindow::on_format_string_changed(QString formatString) {
    mSettings.setValue(formatStringSettingKey, formatString);
}

void MainWindow::setSpinboxesEnabled(bool enabled) const {
    ui->hours_spinbox->setEnabled(enabled);
    ui->minutes_spinbox->setEnabled(enabled);
    ui->seconds_spinbox->setEnabled(enabled);
}

void MainWindow::refreshStartButton() {
    const auto isValidDuration =
            ui->hours_spinbox->value() > 0 || ui->minutes_spinbox->value() > 0 || ui->seconds_spinbox->value() > 0;
    const auto wasFilenameSelected = not getFilename().isEmpty();
    const auto isTimerRunning = mTimer.isActive();
    ui->start_button->setEnabled(isValidDuration && wasFilenameSelected && not isTimerRunning);
}

void MainWindow::refreshPauseButton() {
    ui->pause_button->setEnabled(mCountdownWasStarted);
    ui->pause_button->setText(mTimer.isActive() || not mCountdownWasStarted ? "Pause" : "Resume");
}

void MainWindow::refreshResetButton() {
    ui->reset_button->setEnabled(mCountdownWasStarted);
}

void MainWindow::refreshDestinationWidgets() {
    ui->destination_button->setEnabled(not mCountdownWasStarted);
    ui->filename_edit->setEnabled(not mCountdownWasStarted);
}

void MainWindow::displayTimeSpan(const TimeSpan& timeSpan) {
    ui->hours_spinbox->setValue(timeSpan.hours);
    ui->minutes_spinbox->setValue(timeSpan.minutes);
    ui->seconds_spinbox->setValue(timeSpan.seconds);
}

void MainWindow::readSetting(const QString& settingKey, QSpinBox& spinBox) {
    const auto settingVariant = mSettings.value(settingKey);
    if (not settingVariant.isNull()) {
        spinBox.setValue(settingVariant.toInt());
    }
}

void MainWindow::readSetting(const QString& settingKey, QLineEdit& lineEdit) {
    const auto filenameSettingVariant = mSettings.value(settingKey);
    if (not filenameSettingVariant.isNull()) {
        lineEdit.setText(filenameSettingVariant.toString());
    }
}

void MainWindow::readSetting(const QString& settingKey, QCheckBox& checkBox) {
    const auto filenameSettingVariant = mSettings.value(settingKey);
    if (not filenameSettingVariant.isNull()) {
        checkBox.setChecked(filenameSettingVariant.toBool());
    }
}

void MainWindow::saveFinishMessageSettings() {
    mSettings.setValue(finishMessageEnabledSettingKey, ui->finish_message_checkbox->isChecked());
    mSettings.setValue(finishMessageSettingKey, ui->finish_message_edit->text());
}

TimeSpan MainWindow::calculateRemainingDuration() const {
    const auto currentTime = QTime::currentTime();
    const auto endTime =
            mStartTime.addSecs(mTotalDuration.hours * 60 * 60 + mTotalDuration.minutes * 60 + mTotalDuration.seconds);
    auto remainingSeconds = currentTime.secsTo(endTime);

    auto result = TimeSpan{};
    result.hours = remainingSeconds / 60 / 60;
    remainingSeconds -= result.hours * 60 * 60;
    result.minutes = remainingSeconds / 60;
    remainingSeconds -= result.minutes * 60;
    result.seconds = remainingSeconds;
    return result;
}

void MainWindow::writeToFile(const QString& filename, const TimeSpan& timeSpan) {
    auto formatString = ui->format_string_edit->text();
    const auto hoursString = QString{ "%1" }.arg(QString::number(timeSpan.hours), 2, QLatin1Char{ '0' });
    const auto minutesString = QString{ "%1" }.arg(QString::number(timeSpan.minutes), 2, QLatin1Char{ '0' });
    const auto secondsString = QString{ "%1" }.arg(QString::number(timeSpan.seconds), 2, QLatin1Char{ '0' });
    formatString.replace("{hh}", hoursString);
    formatString.replace("{mm}", minutesString);
    formatString.replace("{ss}", secondsString);
    writeToFile(filename, formatString.toLatin1());
}

void MainWindow::writeToFile(const QString& filename, const QString& contents) {
    auto file = QFile{ filename };
    if (file.open(QFile::WriteOnly)) {
        file.write(contents.toLatin1());
        file.close();
    } else {
        qDebug() << "unable to open file!";
    }
}

QString MainWindow::getFilename() const {
    return ui->filename_edit->text();
}

void MainWindow::on_pause_button_clicked() {
    if (mTimer.isActive()) {
        mStartOfPause = QTime::currentTime();
        mTimer.stop();
    } else {
        // resuming
        const auto durationOfPause = mStartOfPause.msecsTo(QTime::currentTime());
        mStartTime = mStartTime.addMSecs(durationOfPause);
        mTimer.start();
        writeToFile(getFilename(), calculateRemainingDuration());
    }
    refreshPauseButton();
}

void MainWindow::on_reset_button_clicked() {
    Q_ASSERT(mCountdownWasStarted);
    if (mTimer.isActive()) {
        mTimer.stop();
    }
    mCountdownWasStarted = false;
    displayTimeSpan(mTotalDuration);
    writeToFile(getFilename(), mTotalDuration);
    setSpinboxesEnabled(true);
    refreshStartButton();
    refreshPauseButton();
    refreshResetButton();
    refreshDestinationWidgets();
}
