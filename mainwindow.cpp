#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QMouseEvent>
#include <QDebug>
#include <QMimeData>
#include <CustomLineEdit.h>
#include <PatientInfo.h>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //
    ui->setupUi(this);
    ui->mainToolBar->setMinimumHeight(32);
    ui->mainToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->pageControl->setCurrentWidget(ui->startPage);
    this->checkFilePathLine();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openSession()
{
    
}


void MainWindow::on_actionOpen_triggered()
{
    ui->pageControl->setCurrentWidget(ui->fileSystemPage);

    fileSystemModel = new QFileSystemModel(this);
    fileSystemModel->setHeaderData(1,Qt::Horizontal,Qt::EditRole);
    ui->fileSystemView->clearSelection();


    fileSystemModel->setRootPath("C:/");
    fileSystemModel->setFilter(QDir::Dirs|QDir::Drives|QDir::NoDotAndDotDot|QDir::AllDirs|QDir::AllEntries);

    ui->fileSystemView->setModel(fileSystemModel);

    QModelIndex index = fileSystemModel->index("C:/");
//    ui->fileSystemView->setItemsExpandable(false);
    ui->fileSystemView->header()->setMovable(false);
//    ui->treeView->header()->resizeSections(QHeaderView::ResizeToContents);
    ui->fileSystemView->hideColumn(1);
    ui->fileSystemView->hideColumn(2);
    ui->fileSystemView->setColumnWidth(0,350);
    ui->fileSystemView->setColumnWidth(3,250);
    ui->fileSystemView->setRootIndex(index);

    // get the change in selection
    connect(ui->fileSystemView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &))
            , this, SLOT(updateFilePathLine(const QItemSelection &, const QItemSelection &)));

    connect(ui->fileSystemView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &))
            , this, SLOT(updatePatientInfoBox()));

    ui->filePathline->clear();
}



void MainWindow::on_openButton_clicked()
{
    QString filePath = ui->filePathline->text();
    // get patient's information
    PatientInfo patientInfo;
    PatientInfo::PatientInformation patientInformation =  *(patientInfo.extractPatientInfo(filePath));
}

void MainWindow::updateFilePathLine(const QItemSelection & , const QItemSelection & )
{
    QString path;
    QModelIndexList  selected = ui->fileSystemView->selectionModel()->selectedIndexes();
    foreach(QModelIndex current, selected)
    {
        path = fileSystemModel->filePath(current);
    }
    ui->filePathline->setText(path);
}

void MainWindow::updatePatientInfoBox()
{
    QString filePath = ui->filePathline->text();
    PatientInfo patientInfo;
    PatientInfo::PatientInformation patientInformation =  *(patientInfo.extractPatientInfo(filePath));
    ui->patientIDValueLabel->setText(patientInformation["Patient"]);
    ui->ageValueLabel->setText(patientInformation["Age"]);
    ui->heightValueLabel->setText(patientInformation["Height"]);
    ui->weightValueLabel->setText(patientInformation["Weight"]);
    ui->genderValueLabel->setText(patientInformation["Gender"]);
}

void MainWindow::checkFilePathLine()
{
    if(ui->filePathline->getDropFlag())
    {
        this->updatePatientInfoBox();
    }
    QTimer::singleShot(1000, this, SLOT(checkFilePathLine()));
}

void MainWindow::mouseMoveEvent(QMouseEvent * event)
{
    QPoint pos;
    if(event->buttons() == Qt::LeftButton )
    {
        pos = event->pos();
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent * event)
{

}

void MainWindow::mousePressEvent(QMouseEvent * event)
{

}

//void MainWindow::readSession(SessionMetaData::PatientInfo patientInfo, QString logDirName)
//{
//    _tablesBeingFilled = true;
//    loadedFiles.clear();
//    currentPatientDir.setPath(logDirName);

//    double height_m = patientInfo["Height"].toDouble() / 100.0;
//    double weight_kg = patientInfo["Weight"].toDouble();
//    double bmi = weight_kg / (height_m*height_m);
//    double bsa = sqrt (weight_kg * height_m / 36);


//    QDir logDir(logDirName, QString("*_mode-x_*.csv") );

//    logDir.setSorting(QDir::Name);
//    QStringList entries = logDir.entryList();

//    QSettings notesDB(currentPatientDir.filePath(NOTES_DB_FILE_NAME), QSettings::IniFormat);

//    ui->currentSampleLabel->setText("");
//    ui->sessionLoadingProgressBar->setMaximum(entries.length());
//    ui->sessionLoadingProgressBar->setValue(0);

//    ui->hlPerSample->setRowCount(0);
//    ui->perBreath->setRowCount(0);
//    ui->inhaleEntropies->setRowCount(0);

//    HeatLossCalc calc;

//    foreach (QString fileName, entries)
//    {
//        qDebug() << fileName;
//        // filenames are "datetime_mode_pt"
//        QStringList metadata = fileName.split('_');

//        // datetime is the long date format
//        QDateTime dt = QDateTime::fromString ( metadata.at (0), "yyyy-MM-dd-HH-mm-ss" );

//        ui->sessionLoadingProgressBar->setValue(ui->sessionLoadingProgressBar->value() + 1);
//        ui->currentSampleLabel->setText(tr("Analyzing %1").arg(fileName));
//        QApplication::processEvents();

//        try
//        {
//            if (! calc.calcInhaleEntropy(logDir.filePath(fileName)) )
//            {
//                qDebug() << fileName << ": Unable to calculate Inhale Entropy, ignorring";
//                continue;
//            }
//        }
//        catch (exception& e)
//        {
//            qDebug() << e.what();
//            continue;
//        }

//        calc.calcExhaleEntropy(logDir.filePath(fileName));

//        if ( std::isnan(calc.rushHeatLossPerMinute) ) continue;
//        loadedFiles.append(fileName);

//        static qint64 firstSampleTimeStamp = 0;//dt.toMSecsSinceEpoch();
//        if (firstSampleTimeStamp == 0)
//        {
//            firstSampleTimeStamp = dt.toMSecsSinceEpoch();
//            ui->hlChartHolder->chart()->scaleGrid().baseTime = firstSampleTimeStamp;
//            ui->perBreathChartHolder->chart()->scaleGrid().baseTime = firstSampleTimeStamp;
//        }

//        // add timestamp to all channels
//        for (int i = ChannelHLChartBegin; i < ChannelHlChartEnd -1; i ++)
//        {
//            // to seconds
//            qint64 timeStamp = (dt.toMSecsSinceEpoch() -firstSampleTimeStamp)/ 1000;
////            qDebug() << "Sample TimeStamp" << timeStamp;
//            channelsTimeStamps.at(i)->append(timeStamp);
////            channelsTimeStamps.at(i)->append(dt.toMSecsSinceEpoch());
//        }


//        // Per Breath Info
//        for (int breath = 0; breath < calc.temperatureTroughsIndexes.size(); breath ++ )
//        {
////            pbTimeStamps.append(dt.toMSecsSinceEpoch() + 1000 * (int)calc.reducedTimeStamps.at(calc.temperatureTroughsIndexes.at(breath)));
//            for (int i = ChannelPBChartBegin; i < ChannelPBChartEnd; i ++)
//            {
//                qint64 timestamp = (dt.toMSecsSinceEpoch() - firstSampleTimeStamp) / 1000
//                        + calc.reducedTimeStamps.at(calc.temperatureTroughsIndexes.at(breath))
//                        - calc.reducedTimeStamps.last();
////                qDebug() << "Temp Time Stamp" << calc.reducedTimeStamps.at(calc.temperatureTroughsIndexes.at(breath));
//                channelsTimeStamps.at(i)->append(timestamp);
//            }

//        }
//    }

////    qDebug() << "Temp Time Stamps ===========================";
////    for (int i = 0; i < channelsTimeStamps.at(ChannelPBChartBegin)->length(); i++)
////    {
////        qDebug() << channelsTimeStamps.at(ChannelPBChartBegin)->at(i);
////    }


//    if (channelsTimeStamps.at(ChannelHL)->count() > 1)
//    {
//        ui->hlChartHolder->chart()->setPosition(channelsTimeStamps.at(ChannelHL)->first());
//        ui->hlChartHolder->chart()->setSize(channelsTimeStamps.at(ChannelHL)->last() - channelsTimeStamps.at(ChannelHL)->first());
//    }

//    if (channelsTimeStamps.at(ChannelPBChartBegin)->count() > 1)
//    {
//        ui->perBreathChartHolder->chart()->setPosition(channelsTimeStamps.at(ChannelPBChartBegin)->first());
//        ui->perBreathChartHolder->chart()->setSize(channelsTimeStamps.at(ChannelPBChartBegin)->last() - channelsTimeStamps.at(ChannelPBChartBegin)->first());
//    }

//    for (int i = ChannelHLChartBegin; i < ChannelHlChartEnd; i++)
//    {
//        ChannelUtils::setChannelMinMax(channelsData.at(i), & ui->hlChartHolder->chart()->channels().at(i), 2);
//    }





//    ui->hlPerSample     ->resizeColumnsToContents();
//    ui->perBreath       ->resizeColumnsToContents();
//    ui->inhaleEntropies ->resizeColumnsToContents();

//    ui->hlPerSample     ->resizeRowsToContents();
//    ui->perBreath       ->resizeRowsToContents();
//    ui->inhaleEntropies ->resizeRowsToContents();

//    ui->hlChartHolder->chart()->updateWithBackground();
//    ui->hlChartHolder->updateChart();
//    ui->perBreathChartHolder->chart()->updateWithBackground();
//    ui->perBreathChartHolder->updateChart();

//    ui->actionExportAllToPDF->setEnabled(true);
//    ui->actionPrintAll->setEnabled(true);

//    _tablesBeingFilled = false;
//}


void MainWindow::on_cancelButton_clicked()
{
    ui->pageControl->setCurrentWidget(ui->startPage);
}
