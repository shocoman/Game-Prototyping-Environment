#include "settingswindow.h"
#include "ui_settingswindow.h"

#include <QFileDialog>
#include <QMessageBox>

SettingsWindow::SettingsWindow(QWidget *parent, QSettings *settings_) :
    QDialog(parent),
    ui(new Ui::SettingsWindow)
{
    ui->setupUi(this);
    settings = settings_;

    if (settings)
        fillUI();


    resize(sizeHint());
    resize(frameSize().width()*2, frameSize().height());
    setWindowTitle("Options Window");
}

SettingsWindow::~SettingsWindow()
{
    delete ui;
}

void SettingsWindow::fillUI()
{
    QString filePath = settings->value("engine_path", "").toString();
    ui->pathEdit->setText(filePath);
}


void SettingsWindow::accept(){
    auto newEnginePath = ui->pathEdit->text();
    settings->setValue("engine_path", newEnginePath);
    settings->setValue("theme", ui->themeBox->currentText());

    QDialog::accept();
}


void SettingsWindow::on_pathOpen_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open text file");
    if (filePath.length() == 0) return;
    ui->pathEdit->setText(filePath);
}
