#include "PatientInfo.h"
#include <QFile>
#include <QDebug>
#include <QStringList>

PatientInfo::PatientInfo(QObject *parent) :
    QObject(parent)
{

}

PatientInfo::PatientInformation * PatientInfo::extractPatientInfo(QString fileName)
{
    _fileName = fileName;
    PatientInformation * patientInformation = new PatientInformation;
    QFile file(_fileName);
    if(!file.open(QIODevice::ReadOnly))
    {
        qCritical() << "Could not open " << _fileName << " file.";
    }
    while(!file.atEnd())
    {
        QString line = file.readLine();
        if (line[0] == 'P')
        {
            QStringList fields = line.split(",");
            foreach(QString field, fields)
            {
                QStringList keyVal = field.split(QRegExp("[:\\s]"), QString::SkipEmptyParts);
                QString key = keyVal[0];
                QString val = keyVal[1];
                (*patientInformation)[key] = val;
                //qDebug() << "patientInformation[" << key << "] = " << val;
            }
            break;
        }
    }
    return patientInformation;
}
