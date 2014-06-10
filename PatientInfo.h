#ifndef PATIENTINFO_H
#define PATIENTINFO_H

#include <QObject>
#include <QHash>

class PatientInfo : public QObject
{
    Q_OBJECT
public:
    explicit PatientInfo(QObject *parent = 0);
    typedef QHash <QString, QString> PatientInformation;
    PatientInformation * extractPatientInfo(QString );
    
signals:
    
public slots:

private:
    QString _fileName;
    
};

#endif // PATIENTINFO_H
