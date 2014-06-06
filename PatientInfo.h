#ifndef PATIENTINFO_H
#define PATIENTINFO_H

#include <QObject>

class PatientInfo : public QObject
{
    Q_OBJECT
public:
    explicit PatientInfo(QObject *parent = 0);
    
signals:
    
public slots:

private:
    QString fileName;
    
};

#endif // PATIENTINFO_H
