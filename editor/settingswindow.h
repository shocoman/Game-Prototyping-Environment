#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QDialog>
#include <QSettings>
#include <QDebug>

namespace Ui {
class SettingsWindow;
}

class SettingsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsWindow(QWidget *parent = nullptr, QSettings *settings_ = nullptr);
    ~SettingsWindow();

private slots:
    void on_pathOpen_clicked();

public:
    Ui::SettingsWindow *ui;
    QSettings *settings;

    void fillUI();
    void accept() override;
};

#endif // SETTINGSWINDOW_H
