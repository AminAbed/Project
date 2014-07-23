#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QWidget>

namespace Ui {
class SettingsPage;
}

class SettingsPage : public QWidget
{
    Q_OBJECT
    
public:
    explicit SettingsPage(QWidget *parent = 0);
    ~SettingsPage();
    
private slots:
    void on_removeAllPushButton_clicked();
    void on_plotAllPushButton_clicked();
    void on_resetScalePushButton_clicked();

    void on_reCheckBox_toggled(bool checked);
    void on_ereCheckBox_toggled(bool checked);
    void on_o2CheckBox_toggled(bool checked);
    void on_minTempCheckBox_toggled(bool checked);
    void on_maxTempCheckBox_toggled(bool checked);
    void on_minRhCheckBox_toggled(bool checked);
    void on_maxRhCheckBox_toggled(bool checked);
    void updateCheckBoxes(QString ,bool );




signals:
    void removeAll();
    void removeIndividualPlot(int );
    void plot(QString , bool );
    void remove(QString, bool );

private:
    Ui::SettingsPage *ui;
};

#endif // SETTINGSPAGE_H
