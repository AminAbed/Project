#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMouseEvent>
#include <QDebug>
#include <QMimeData>
#include <CustomLineEdit.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //
    ui->setupUi(this);
    ui->mainToolBar->setMinimumHeight(32);
    ui->mainToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->pageControl->setCurrentWidget(ui->startPage);
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

//    ui->fileSystemView->setSelectionMode(QAbstractItemView::MultiSelection);
//    QItemSelectionModel * selection = ui->treeView->selectedIndexes();
//    QModelIndexList  selected = ui->treeView->selectionModel()->selectedIndexes();
//    qDebug() <<  selected;

    // get the change in selection
    connect(ui->fileSystemView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &))
            , this, SLOT(updateFilePathLine(const QItemSelection &, const QItemSelection &)));
}



void MainWindow::on_openButton_clicked()
{
    QString filePath = ui->filePathline->text();
    qDebug() << filePath;
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

void MainWindow::mouseMoveEvent(QMouseEvent * event)
{
    QPoint pos;
   // qDebug() << event->buttons();
    if(event->buttons() == Qt::LeftButton )
    {
        pos = event->pos();
        //qDebug() << pos;
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

//        ui->inhaleEntropies->setRowCount(ui->inhaleEntropies->rowCount() +1);
//        setText(ui->inhaleEntropies, ui->inhaleEntropies->rowCount()-1, 0, dt.toString("hh:mm"));
//        setText(ui->inhaleEntropies, ui->inhaleEntropies->rowCount()-1, 1, tr("%1").arg( calc.globalTemperatureTrough, 0, 'f', 1 ));
//        setText(ui->inhaleEntropies, ui->inhaleEntropies->rowCount()-1, 2, tr("%1").arg( calc.globalHumidityTrough, 0, 'f', 1 ));

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


//        ui->hlPerSample->setRowCount(ui->hlPerSample->rowCount() +1);
//        setText(ui->hlPerSample, ui->hlPerSample->rowCount()-1, 0, dt.toString("hh:mm"));
//        setText(ui->hlPerSample, ui->hlPerSample->rowCount()-1, 1, tr("%1").arg( calc.rushHeatLossPerMinute  , 0, 'f', 1 ) );
//        setText(ui->hlPerSample, ui->hlPerSample->rowCount()-1, 2, tr("%1").arg( calc.rushHeatLossPerMinute / calc.rushVolumePerMinute, 0, 'f', 1 ) );
//        setText(ui->hlPerSample, ui->hlPerSample->rowCount()-1, 3, tr("%1").arg( calc.rushHeatLossPerMinute / bmi, 0, 'f', 2 ) );
//        setText(ui->hlPerSample, ui->hlPerSample->rowCount()-1, 4, tr("%1").arg( calc.rushHeatLossPerMinute / bsa, 0, 'f', 1 ) );
//        setText(ui->hlPerSample, ui->hlPerSample->rowCount()-1, 5, tr("%1").arg( calc.globalTemperatureTrough, 0, 'f', 1 ) );
//        setText(ui->hlPerSample, ui->hlPerSample->rowCount()-1, 6, tr("%1").arg( calc.globalTemperaturePeak  , 0, 'f', 1 ) );
//        setText(ui->hlPerSample, ui->hlPerSample->rowCount()-1, 7, tr("%1").arg( calc.globalHumidityTrough   , 0, 'f', 1 ) );
//        setText(ui->hlPerSample, ui->hlPerSample->rowCount()-1, 8, tr("%1").arg( calc.globalHumidityPeak     , 0, 'f', 1 ) );


//        // Vt - Average Tidal Volume
//        double vt = calc.rushVolumeSum / calc.numBreaths;
//        setText(ui->hlPerSample, ui->hlPerSample->rowCount()-1, 9, tr("%1").arg(  vt    , 0, 'f', 2 ) );
//        // J/Vt - Heat Loss/Average Tidal Volume
//        setText(ui->hlPerSample, ui->hlPerSample->rowCount()-1, 10, tr("%1").arg( calc.rushHeatContentSum / vt , 0, 'f', 1 ) );
//        // J/Vt/kg - Heat Loss/Average Tidal Volume/kg of patient wt
//        setText(ui->hlPerSample, ui->hlPerSample->rowCount()-1, 11, tr("%1").arg( calc.rushHeatContentSum / vt / weight_kg , 0, 'f', 2 ) );
//        // Min V - Minute Volume
//        setText(ui->hlPerSample, ui->hlPerSample->rowCount()-1, 12, tr("%1").arg( calc.rushVolumePerMinute , 0, 'f', 2 ) );
//        // Inh E - Inhaled Enthalpy
//        setText(ui->hlPerSample, ui->hlPerSample->rowCount()-1, 13, tr("%1").arg( calc.inhalationHeatContent / calc.rushVolumeSum , 0, 'f', 4 ) );

//        // Notes
//        setText(ui->hlPerSample, ui->hlPerSample->rowCount()-1, 14, notesDB.value(fileName, "").toString(), true );

//        setText(ui->hlPerSample, ui->hlPerSample->rowCount()-1, 15, tr("%1").arg( calc.rushO2ContentPerMinute , 0, 'f', 2 ) );

//        //channelsData.at(ChannelHL)->append(calc.rushHeatLossPerMinute * 1 ); // removing 1 causes the grapher to crash
//        channelsData.at(ChannelHL)->append(calc.rushO2ContentPerMinute ); // removing 1 causes the grapher to crash
//        channelsData.at(ChannelHLcc)->append(calc.rushHeatLossPerMinute  / calc.rushVolumePerMinute);
//        channelsData.at(ChannelHLbmi)->append(calc.rushHeatLossPerMinute / bmi);
//        channelsData.at(ChannelHLbsa)->append(calc.rushHeatLossPerMinute / bsa );

//        //channelsData.at(ChannelO2)->append(calc.rushO2ContentPerMinute * 1);
//       // channelsData.at(ChannelO2)->append(2);


//        channelsData.at(ChannelHLMinTemp)->append(calc.globalTemperaturePeak);
//        channelsData.at(ChannelHLMaxTemp)->append(calc.globalTemperatureTrough);
//        channelsData.at(ChannelHLMinHum)->append(calc.globalHumidityPeak);
//        channelsData.at(ChannelHLMaxHum)->append(calc.globalHumidityTrough);


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

//            ui->perBreath->setRowCount(ui->perBreath->rowCount() +1);
//            setText(ui->perBreath, ui->perBreath->rowCount()-1, 0, tr("%1").arg( ui->hlPerSample->rowCount() ) );
//            setText(ui->perBreath, ui->perBreath->rowCount()-1, 1, tr("%1").arg( calc.reducedReadings[HeatLossCalc::TEMPERATURE_CH].at(calc.temperatureTroughsIndexes.at(breath)), 0, 'f', 1 ) );
//            setText(ui->perBreath, ui->perBreath->rowCount()-1, 2, tr("%1").arg( calc.reducedReadings[HeatLossCalc::TEMPERATURE_CH].at(calc.temperaturePeaksIndexes.at(breath))  , 0, 'f', 1 ) );
//            setText(ui->perBreath, ui->perBreath->rowCount()-1, 3, tr("%1").arg( calc.reducedReadings[HeatLossCalc::HUMIDITY_CH   ].at(calc.humidityTroughsIndexes.at(breath))   , 0, 'f', 1 ) );
//            setText(ui->perBreath, ui->perBreath->rowCount()-1, 4, tr("%1").arg( calc.reducedReadings[HeatLossCalc::HUMIDITY_CH   ].at(calc.humidityPeaksIndexes.at(breath))     , 0, 'f', 1 ) );

//            channelsData.at(ChannelPBMinTemp)->append( calc.reducedReadings[HeatLossCalc::TEMPERATURE_CH].at(calc.temperatureTroughsIndexes.at(breath)) );
//            channelsData.at(ChannelPBMaxTemp)->append( calc.reducedReadings[HeatLossCalc::TEMPERATURE_CH].at(calc.temperaturePeaksIndexes.at(breath)) );
//            channelsData.at(ChannelPBMinHum)->append( calc.reducedReadings[HeatLossCalc::HUMIDITY_CH   ].at(calc.humidityTroughsIndexes.at(breath)) );
//            channelsData.at(ChannelPBMaxHum)->append( calc.reducedReadings[HeatLossCalc::HUMIDITY_CH   ].at(calc.humidityPeaksIndexes.at(breath)) );

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


//    // give temp min and max channels same zoom, ditto for humidity channels
////    ChannelUtils::setChannelMinMax(channelsData.at(ChannelHLMinTemp), & ui->hlChartHolder->chart()->channels().at(ChannelHLMinTemp - ChannelHLChartBegin), 2, channelsData.at(ChannelHLMaxTemp));
////    ChannelUtils::setChannelMinMax(channelsData.at(ChannelHLMinTemp), & ui->hlChartHolder->chart()->channels().at(ChannelHLMaxTemp - ChannelHLChartBegin), 2, channelsData.at(ChannelHLMaxTemp));
////    ChannelUtils::setChannelMinMax(channelsData.at(ChannelHLMinHum),  & ui->hlChartHolder->chart()->channels().at(ChannelHLMinHum - ChannelHLChartBegin), 2, channelsData.at(ChannelHLMaxHum));
////    ChannelUtils::setChannelMinMax(channelsData.at(ChannelHLMinHum),  & ui->hlChartHolder->chart()->channels().at(ChannelHLMaxHum - ChannelHLChartBegin), 2, channelsData.at(ChannelHLMaxHum));



////    // give temp min and max channels same zoom, ditto for humidity channels
////    ChannelUtils::setChannelMinMax(channelsData.at(ChannelPBMinTemp), & ui->perBreathChartHolder->chart()->channels().at(ChannelPBMinTemp - ChannelPBChartBegin), 2, channelsData.at(ChannelPBMaxTemp));
////    ChannelUtils::setChannelMinMax(channelsData.at(ChannelPBMinTemp), & ui->perBreathChartHolder->chart()->channels().at(ChannelPBMaxTemp - ChannelPBChartBegin), 2, channelsData.at(ChannelPBMaxTemp));
////    ChannelUtils::setChannelMinMax(channelsData.at(ChannelPBMinHum),  & ui->perBreathChartHolder->chart()->channels().at(ChannelPBMinHum - ChannelPBChartBegin), 2, channelsData.at(ChannelPBMaxHum));
////    ChannelUtils::setChannelMinMax(channelsData.at(ChannelPBMinHum),  & ui->perBreathChartHolder->chart()->channels().at(ChannelPBMaxHum - ChannelPBChartBegin), 2, channelsData.at(ChannelPBMaxHum));



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

