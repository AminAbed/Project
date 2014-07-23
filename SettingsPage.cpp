#include "SettingsPage.h"
#include "ui_SettingsPage.h"
#include "MainWindow.h"
#include <QDebug>

SettingsPage::SettingsPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsPage)
{
    ui->setupUi(this);

}

SettingsPage::~SettingsPage()
{
    delete ui;
}

void SettingsPage::on_removeAllPushButton_clicked()
{
    emit removeAll();
}

void SettingsPage::on_plotAllPushButton_clicked()
{

}

void SettingsPage::on_resetScalePushButton_clicked()
{

}

void SettingsPage::on_reCheckBox_toggled(bool checked)
{
    QString arg = ui->reCheckBox->text();
    if(checked) plot(arg, checked);
    else remove(arg, checked);
}

void SettingsPage::on_ereCheckBox_toggled(bool checked)
{
    QString arg = ui->ereCheckBox->text();
    if(checked) plot(arg, checked);
    else remove(arg, checked);
}

void SettingsPage::on_o2CheckBox_toggled(bool checked)
{
    QString arg = ui->o2CheckBox->text();
    if(checked) plot(arg, checked);
    else remove(arg, checked);
}

void SettingsPage::on_minTempCheckBox_toggled(bool checked)
{
    QString arg = ui->minTempCheckBox->text();
    if(checked) plot(arg, checked);
    else remove(arg, checked);
}

void SettingsPage::on_maxTempCheckBox_toggled(bool checked)
{
    QString arg = ui->maxTempCheckBox->text();
    if(checked) plot(arg, checked);
    else remove(arg, checked);
}

void SettingsPage::on_minRhCheckBox_toggled(bool checked)
{
    QString arg = ui->minRhCheckBox->text();
    if(checked) plot(arg, checked);
    else remove(arg, checked);
}

void SettingsPage::on_maxRhCheckBox_toggled(bool checked)
{
    QString arg = ui->maxRhCheckBox->text();
    if(checked) plot(arg, checked);
    else remove(arg, checked);
}

void SettingsPage::updateCheckBoxes(QString name, bool checked)
{
         if (name == "RE")       ui->reCheckBox->setChecked(checked);
    else if (name == "ERE")      ui->ereCheckBox->setChecked(checked);
    else if (name == "O2")       ui->o2CheckBox->setChecked(checked);
    else if (name == "Min Temp") ui->minTempCheckBox->setChecked(checked);
    else if (name == "Max Temp") ui->maxTempCheckBox->setChecked(checked);
    else if (name == "Min RH")   ui->minRhCheckBox->setChecked(checked);
    else if (name == "Max RH")   ui->maxRhCheckBox->setChecked(checked);
}


