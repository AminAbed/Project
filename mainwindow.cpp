#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QMouseEvent>
#include <QDebug>
#include <QMimeData>
#include <CustomLineEdit.h>
#include <PatientInfo.h>
#include <QTimer>
#include <QChar>
#include <QMessageBox>
#include <QStringList>
#include <QVector>
#include <QDateTime>
#include "QCustomPlot.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //
    ui->setupUi(this);
    ui->mainToolBar->setMinimumHeight(32);
    ui->mainToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->pageControl->setCurrentWidget(ui->startPage);
    //this->checkFilePathLine();

    //install eventFilter
    ui->plotView->installEventFilter(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_triggered()
{
    ui->pageControl->setCurrentWidget(ui->fileSystemPage);
    this->setupFileSystemView();


    // get the change in selection
    connect(ui->fileSystemView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &))
            , this, SLOT(updateFilePathLine(const QItemSelection &, const QItemSelection &)));

    connect(ui->fileSystemView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &))
            , this, SLOT(updatePatientInfoBox()));

    connect(this->ui->filePathline, SIGNAL(dropped()) , this, SLOT(updatePatientInfoBox()));

    // to prevent from dragging out of bounds
    connect(ui->plotView->xAxis, SIGNAL(rangeChanged(QCPRange )), this, SLOT(xAxisLimit(QCPRange )));
    connect(ui->plotView->yAxis, SIGNAL(rangeChanged(QCPRange )), this, SLOT(yAxisLimit(QCPRange )));

    // connect slot that ties some axis selections together (especially opposite axes):
    connect(ui->plotView, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));

    ui->filePathline->clear();
}



void MainWindow::on_openButton_clicked()
{
    QString filePath = ui->filePathline->text();
    qDebug() << "filePath contains " << filePath.contains("_summaryFile");
    if (!filePath.isEmpty() && !filePath.contains("_summaryFile"))
    {
        QMessageBox::warning(this,"",tr("The supplied file is not supported."));
    }
    else if(filePath.isEmpty())
    {
        QMessageBox::warning(this,"",tr("Please provide a summary file for analysis."));
    }
    else
    {
        // get patient's information
        PatientInfo patientInfo;
        PatientInfo::PatientInformation patientInformation =  *(patientInfo.extractPatientInfo(filePath));


        ui->patientInfoLabel->setText(
                 tr("Patient Info:  ID #%1  |  Age: %2  |  Gender: %3  |  Height: %4  |  Weight: %5 ")
                .arg(patientInformation["Patient"])
                .arg(patientInformation["Age"])
                .arg(patientInformation["Gender"])
                .arg(patientInformation["Height"])
                .arg(patientInformation["Weight"]));
        this->readSession(filePath);
    }
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

// not used; lots of overhead
void MainWindow::checkFilePathLine()
{
    if(ui->filePathline->getDropFlag())
    {
        this->updatePatientInfoBox();
    }
    QTimer::singleShot(1000, this, SLOT(checkFilePathLine()));
}

void MainWindow::setupFileSystemView()
{
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
}

int MainWindow::readSession(QString filePath)
{
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "File could not be opened.";
        return -1;
    }

    QString fileContent = file.readAll();
    file.close();
    QStringList fileLines = fileContent.split(QRegExp("[\r\n]"),QString::SkipEmptyParts);

    for(int i = 0; i < fileLines.size(); i++)
    {
        QString line = fileLines.at(i);
        // ignoring the patient info and data header
        if (!line.startsWith("2")) continue;
        QStringList fields = line.split(",");
        if (fields.size() < total) continue;
        // extracting time stamps
        timeStamp << fields[0];
        for (int columnNum = 0; columnNum < total; columnNum++)
        {
            QString reading = fields.at(columnNum+1);
            readings[columnNum].append(reading.toDouble());
        }
    }
    this->plot();


    QString time ="10-56-56";
    //QDateTime timeDate = QDateTime::toTime_t();
    const QString sformat="hh-mm-ss"; //Generate Date
    QTime dateTime = QTime::fromString( time, sformat );
    qDebug() << "time is" << dateTime;
//    double timeSeconds = dateTime.toTime_t();
//    qDebug() << "time is" << timeSeconds;




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

//
}

void MainWindow::on_cancelButton_clicked()
{
    ui->pageControl->setCurrentWidget(ui->startPage);
}

void MainWindow::plot(/*QCustomPlot * customPlot*/)
{
    /* setting up the plot */
    // legend
    ui->plotView->legend->setVisible(true);
    QFont legendFont = font();
    legendFont.setPointSize(9); // and make a bit smaller for legend
    ui->plotView->legend->setFont(legendFont);
    ui->plotView->legend->setBrush(QBrush(QColor(255,255,255,150)));
    // to change legend placement:
    ui->plotView->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignBottom|Qt::AlignRight);
//    ui->plotView->setBackground(Qt::transparent);
//    ui->plotView->axisRect()->setBackground(Qt::white);
    QVector<double> y(readings[O2Consumption].size());
    QVector<double> w(readings[minRH].size());
    QVector<double> z(readings[respiratoryEnthalpy].size());
    QVector<double> x(readings[O2Consumption].size());
    QVector<double> a(readings[maxRH].size());
    qDebug() << "readings[O2Consumption].size()" << readings[O2Consumption].size();
    for (int i = 0; i < x.size(); i++)
    {
        x[i]= i;
    }

    y = readings[O2Consumption].toVector();
    z = readings[respiratoryEnthalpy].toVector();
    w = readings[minRH].toVector();
    a = readings[maxRH].toVector();
    qDebug() << "y.size()" << y.size() << "x.size()" << x.size();
    connect(ui->horizontalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(horzScrollBarChanged(int)));
    connect(ui->verticalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(vertScrollBarChanged(int)));
    connect(ui->plotView->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(xAxisChanged(QCPRange)));
    connect(ui->plotView->yAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(yAxisChanged(QCPRange)));

    ui->plotView->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    ui->plotView->addGraph();
    ui->plotView->graph(0)->setPen(QPen(Qt::red));
    ui->plotView->graph(0)->setData(x,w);
    ui->plotView->addGraph();
    ui->plotView->graph(1)->setPen(QPen(Qt::blue));
    ui->plotView->graph(1)->setBrush(QBrush(QColor(0, 0, 255, 20)));
    ui->plotView->graph(1)->setData(x,y);
  //  ui->plotView->graph(1)->setValueAxis(ui->plotView->yAxis2);

    // give the axes some labels:
    ui->plotView->xAxis->setLabel("x");
    ui->plotView->yAxis->setLabel("y");
    // set axes ranges, so we see all data:
//    customPlot->xAxis->setRange(-1, 1);
//    customPlot->yAxis->setRange(0, 1);

//    ui->plotView->xAxis->setRange(-1, 1, Qt::AlignCenter);
    ui->plotView->xAxis->setRange(0, 200);
    ui->plotView->yAxis2->setRange(10,50);

    ui->plotView->yAxis2->setVisible(true);
    ui->plotView->addGraph(0,ui->plotView->yAxis2);
    ui->plotView->graph(2)->setPen(QPen(Qt::green));
    ui->plotView->graph(2)->setBrush(QBrush(QColor(34,139,34, 35)));
    ui->plotView->graph(2)->setData(x,a);
    ui->pageControl->setCurrentWidget(ui->plotPage);
 //   ui->plotView->axisRect();


}

void MainWindow::horzScrollBarChanged(int value)
{
  if (qAbs(ui->plotView->xAxis->range().center()-value/100.0) > 0.01) // if user is dragging plot, we don't want to replot twice
  {
    ui->plotView->xAxis->setRange(value/100.0, ui->plotView->xAxis->range().size(), Qt::AlignCenter);
    ui->plotView->replot();
  }
}

void MainWindow::vertScrollBarChanged(int value)
{
  if (qAbs(ui->plotView->yAxis->range().center()+value/100.0) > 0.01) // if user is dragging plot, we don't want to replot twice
  {
    ui->plotView->yAxis->setRange(-value/100.0, ui->plotView->yAxis->range().size(), Qt::AlignCenter);
    ui->plotView->replot();
  }
}

void MainWindow::xAxisChanged(QCPRange range)
{
  ui->horizontalScrollBar->setValue(qRound(range.center()*100.0)); // adjust position of scroll bar slider
  ui->horizontalScrollBar->setPageStep(qRound(range.size()*100.0)); // adjust size of scroll bar slider
}

void MainWindow::yAxisChanged(QCPRange range)
{
  ui->verticalScrollBar->setValue(qRound(-range.center()*100.0)); // adjust position of scroll bar slider
  ui->verticalScrollBar->setPageStep(qRound(range.size()*100.0)); // adjust size of scroll bar slider
}

void MainWindow::xAxisLimit(QCPRange newRange)
{
    //qDebug() << "x-axis range changing" << newRange.size();
    QCPRange boundedRange = newRange;
    double lowerRangeBound = 0;
    double upperRangeBound = 180;
    if (boundedRange.size() > upperRangeBound-lowerRangeBound)
    {
        boundedRange = QCPRange(lowerRangeBound, upperRangeBound);
    }
    else
    {
        double oldSize = boundedRange.size();
        if (boundedRange.lower < lowerRangeBound)
        {
            boundedRange.lower = lowerRangeBound;
            boundedRange.upper = lowerRangeBound+oldSize;
        }
        if (boundedRange.upper > upperRangeBound)
        {
            boundedRange.lower = upperRangeBound-oldSize;
            boundedRange.upper = upperRangeBound;
        }
    }
    ui->plotView->xAxis->setRange(boundedRange);
}

void MainWindow::yAxisLimit(QCPRange newRange)
{
    //qDebug() << "y-axis range changing" << newRange.size();
    QCPRange boundedRange = newRange;
    double lowerRangeBound = 0;
    double upperRangeBound = 50;
    if (boundedRange.size() > upperRangeBound-lowerRangeBound)
    {
        boundedRange = QCPRange(lowerRangeBound, upperRangeBound);
    }
    else
    {
        double oldSize = boundedRange.size();
        if (boundedRange.lower < lowerRangeBound)
        {
            boundedRange.lower = lowerRangeBound;
            boundedRange.upper = lowerRangeBound+oldSize;
        }
        if (boundedRange.upper > upperRangeBound)
        {
            boundedRange.lower = upperRangeBound-oldSize;
            boundedRange.upper = upperRangeBound;
        }
    }
    ui->plotView->yAxis->setRange(boundedRange);
}

bool MainWindow::eventFilter(QObject *target, QEvent *event)
{
    if(target == ui->plotView && event->type() == QEvent::MouseMove)
    {
        QMouseEvent * mouseEvent = static_cast<QMouseEvent*>(event);
        int x = ui->plotView->xAxis->pixelToCoord(mouseEvent->pos().x());
        int y = ui->plotView->yAxis->pixelToCoord(mouseEvent->pos().y());
        qDebug() << "x" << x << "y" << y;

        if (y <= 0 && x >= 0)
        {
            qDebug() << "zooming in x";
            ui->plotView->axisRect()->setRangeZoom(ui->plotView->xAxis->orientation());
        }
        else if (x <= 0 && y >= 0)
        {
            qDebug() << "zooming in y";
            ui->plotView->axisRect()->setRangeZoom(/*ui->plotView->yAxis->orientation()*/Qt::Vertical);
        }
        else
        {
            qDebug() << "zooming in both x and y";
            ui->plotView->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
        }
          //setToolTip(QString("%1 , %2").arg(x).arg(y));
    }
    return false;
}

void MainWindow::selectionChanged()
{
  /*
   normally, axis base line, axis tick labels and axis labels are selectable separately, but we want
   the user only to be able to select the axis as a whole, so we tie the selected states of the tick labels
   and the axis base line together. However, the axis label shall be selectable individually.

   The selection state of the left and right axes shall be synchronized as well as the state of the
   bottom and top axes.

   Further, we want to synchronize the selection of the graphs with the selection state of the respective
   legend item belonging to that graph. So the user can select a graph by either clicking on the graph itself
   or on its legend item.
  */



  // make top and bottom axes be selected synchronously, and handle axis and tick labels as one selectable object:
  if (ui->plotView->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->plotView->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels)// ||
     // ui->plotView->xAxis2->selectedParts().testFlag(QCPAxis::spAxis) || ui->plotView->xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels)
          )
  {
      qDebug() << "selected";
    ui->plotView->yAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    ui->plotView->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
  }
//  // make left and right axes be selected synchronously, and handle axis and tick labels as one selectable object:
//  if (ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
//      ui->customPlot->yAxis2->selectedParts().testFlag(QCPAxis::spAxis) || ui->customPlot->yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
//  {
//    ui->customPlot->yAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
//    ui->customPlot->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
//  }

//  // synchronize selection of graphs with selection of corresponding legend items:
//  for (int i=0; i<ui->customPlot->graphCount(); ++i)
//  {
//    QCPGraph *graph = ui->customPlot->graph(i);
//    QCPPlottableLegendItem *item = ui->customPlot->legend->itemWithPlottable(graph);
//    if (item->selected() || graph->selected())
//    {
//      item->setSelected(true);
//      graph->setSelected(true);
//    }
//  }
}



void MainWindow::mouseMoveEvent(QMouseEvent * event)
{
    QPoint pos;
    if(event->buttons() == Qt::LeftButton )
    {
        pos = event->pos();
    }
    qDebug() << pos;
}

void MainWindow::mouseReleaseEvent(QMouseEvent * event)
{

}

void MainWindow::mousePressEvent(QMouseEvent * event)
{

}
