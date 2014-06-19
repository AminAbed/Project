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

    connect(this->ui->filePathline, SIGNAL(dropped())
            , this, SLOT(updatePatientInfoBox()));

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
        ui->pageControl->setCurrentWidget(ui->plotPage);

        ui->patientInfoLabel->setText(
                 tr("Patient Info:  ID #%1  |  Age: %2  |  Gender: %3  |  Height: %4  |  Weight: %5 ")
                .arg(patientInformation["Patient"])
                .arg(patientInformation["Age"])
                .arg(patientInformation["Gender"])
                .arg(patientInformation["Height"])
                .arg(patientInformation["Weight"]));
        this->readSession(filePath);
        this->plot(ui->plotView);
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

void MainWindow::plot(QCustomPlot * customPlot)
{
    QVector<double> y;
    QVector<double> z(readings[respiratoryEnthalpy].size());
    QVector<double> x(readings[O2Consumption].size());
    qDebug() << "readings[O2Consumption].size()" << readings[O2Consumption].size();
    for (int i = 0; i < x.size(); i++)
    {
        x[i]= i;
    }
   // y = QVector::fromList(readings[O2Consumption]);
    y = readings[O2Consumption].toVector();
    z = readings[respiratoryEnthalpy].toVector();
    qDebug() << "y.size()" << y.size() << "x.size()" << x.size();
    ui->plotView->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    customPlot->addGraph();
    customPlot->graph(0)->setData(x,z);
    customPlot->xAxis->setRange(0, 200);

//    customPlot->yAxis->setRange(0, 5);
 //   customPlot->addGraph();
   // customPlot->graph(1)->setData(z,y);
    customPlot->graph(0)->setPen(QPen(Qt::red));
    customPlot->replot();



//    ui->plotView->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    // generate some data:
//    QVector<double> x(101), y(101); // initialize with entries 0..100
//    for (int i=0; i<101; ++i)
//    {
//      x[i] = i/50.0 - 1; // x goes from -1 to 1
//      y[i] = x[i]*x[i]; // let's plot a quadratic function
//    }
    // create graph and assign data to it:
    customPlot->addGraph();
    customPlot->graph(0)->setData(x, y);
    // give the axes some labels:
    customPlot->xAxis->setLabel("x");
    customPlot->yAxis->setLabel("y");
    // set axes ranges, so we see all data:
    customPlot->xAxis->setRange(-1, 1);
    customPlot->yAxis->setRange(0, 1);
    customPlot->replot();


//    customPlot->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom)); // period as decimal separator and comma as thousand separator
//    customPlot->legend->setVisible(true);
//    QFont legendFont = font();  // start out with MainWindow's font..
//    legendFont.setPointSize(9); // and make a bit smaller for legend
//    customPlot->legend->setFont(legendFont);
//    customPlot->legend->setBrush(QBrush(QColor(255,255,255,230)));
//    // by default, the legend is in the inset layout of the main axis rect. So this is how we access it to change legend placement:
//    customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignBottom|Qt::AlignRight);

//    // setup for graph 0: key axis left, value axis bottom
//    // will contain left maxwell-like function
//    customPlot->addGraph(customPlot->yAxis, customPlot->xAxis);
//    customPlot->graph(0)->setPen(QPen(QColor(255, 100, 0)));
//    customPlot->graph(0)->setBrush(QBrush(QPixmap("./dali.png"))); // fill with texture of specified png-image
//    customPlot->graph(0)->setLineStyle(QCPGraph::lsLine);
//    customPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));
//    customPlot->graph(0)->setName("Left maxwell function");

//    // setup for graph 1: key axis bottom, value axis left (those are the default axes)
//    // will contain bottom maxwell-like function
//    customPlot->addGraph();
//    customPlot->graph(1)->setPen(QPen(Qt::red));
//    customPlot->graph(1)->setBrush(QBrush(QPixmap("./dali.png"))); // same fill as we used for graph 0
//    customPlot->graph(1)->setLineStyle(QCPGraph::lsStepCenter);
//    customPlot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::red, Qt::white, 7));
//    customPlot->graph(1)->setErrorType(QCPGraph::etValue);
//    customPlot->graph(1)->setName("Bottom maxwell function");

//    // setup for graph 2: key axis top, value axis right
//    // will contain high frequency sine with low frequency beating:
//    customPlot->addGraph(customPlot->xAxis2, customPlot->yAxis2);
//    customPlot->graph(2)->setPen(QPen(Qt::blue));
//    customPlot->graph(2)->setName("High frequency sine");

//    // setup for graph 3: same axes as graph 2
//    // will contain low frequency beating envelope of graph 2
//    customPlot->addGraph(customPlot->xAxis2, customPlot->yAxis2);
//    QPen blueDotPen;
//    blueDotPen.setColor(QColor(30, 40, 255, 150));
//    blueDotPen.setStyle(Qt::DotLine);
//    blueDotPen.setWidthF(4);
//    customPlot->graph(3)->setPen(blueDotPen);
//    customPlot->graph(3)->setName("Sine envelope");

//    // setup for graph 4: key axis right, value axis top
//    // will contain parabolically distributed data points with some random perturbance
//    customPlot->addGraph(customPlot->yAxis2, customPlot->xAxis2);
//    customPlot->graph(4)->setPen(QColor(50, 50, 50, 255));
//    customPlot->graph(4)->setLineStyle(QCPGraph::lsNone);
//    customPlot->graph(4)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 4));
//    customPlot->graph(4)->setName("Some random data around\na quadratic function");

//    // generate data, just playing with numbers, not much to learn here:
//    QVector<double> x0(25), y0(25);
//    QVector<double> x1(15), y1(15), y1err(15);
//    QVector<double> x2(250), y2(250);
//    QVector<double> x3(250), y3(250);
//    QVector<double> x4(250), y4(250);
//    for (int i=0; i<25; ++i) // data for graph 0
//    {
//      x0[i] = 3*i/25.0;
//      y0[i] = exp(-x0[i]*x0[i]*0.8)*(x0[i]*x0[i]+x0[i]);
//    }
//    for (int i=0; i<15; ++i) // data for graph 1
//    {
//      x1[i] = 3*i/15.0;;
//      y1[i] = exp(-x1[i]*x1[i])*(x1[i]*x1[i])*2.6;
//      y1err[i] = y1[i]*0.25;
//    }
//    for (int i=0; i<250; ++i) // data for graphs 2, 3 and 4
//    {
//      x2[i] = i/250.0*3*M_PI;
//      x3[i] = x2[i];
//      x4[i] = i/250.0*100-50;
//      y2[i] = sin(x2[i]*12)*cos(x2[i])*10;
//      y3[i] = cos(x3[i])*10;
//      y4[i] = 0.01*x4[i]*x4[i] + 1.5*(rand()/(double)RAND_MAX-0.5) + 1.5*M_PI;
//    }

//    // pass data points to graphs:
//    customPlot->graph(0)->setData(x0, y0);
//    customPlot->graph(1)->setDataValueError(x1, y1, y1err);
//    customPlot->graph(2)->setData(x2, y2);
//    customPlot->graph(3)->setData(x3, y3);
//    customPlot->graph(4)->setData(x4, y4);
//    // activate top and right axes, which are invisible by default:
//    customPlot->xAxis2->setVisible(true);
//    customPlot->yAxis2->setVisible(true);
//    // set ranges appropriate to show data:
//    customPlot->xAxis->setRange(0, 2.7);
//    customPlot->yAxis->setRange(0, 2.6);
//    customPlot->xAxis2->setRange(0, 3.0*M_PI);
//    customPlot->yAxis2->setRange(-70, 35);
//    // set pi ticks on top axis:
//    QVector<double> piTicks;
//    QVector<QString> piLabels;
//    piTicks << 0  << 0.5*M_PI << M_PI << 1.5*M_PI << 2*M_PI << 2.5*M_PI << 3*M_PI;
//    piLabels << "0" << QString::fromUtf8("½π") << QString::fromUtf8("π") << QString::fromUtf8("1½π") << QString::fromUtf8("2π") << QString::fromUtf8("2½π") << QString::fromUtf8("3π");
//    customPlot->xAxis2->setAutoTicks(false);
//    customPlot->xAxis2->setAutoTickLabels(false);
//    customPlot->xAxis2->setTickVector(piTicks);
//    customPlot->xAxis2->setTickVectorLabels(piLabels);
//    // add title layout element:
//    customPlot->plotLayout()->insertRow(0);
//    customPlot->plotLayout()->addElement(0, 0, new QCPPlotTitle(customPlot, "Way too many graphs in one plot"));
//    // set labels:
//    customPlot->xAxis->setLabel("Bottom axis with outward ticks");
//    customPlot->yAxis->setLabel("Left axis label");
//    customPlot->xAxis2->setLabel("Top axis label");
//    customPlot->yAxis2->setLabel("Right axis label");
//    // make ticks on bottom axis go outward:
//    customPlot->xAxis->setTickLength(0, 5);
//    customPlot->xAxis->setSubTickLength(0, 3);
//    // make ticks on right axis go inward and outward:
//    customPlot->yAxis2->setTickLength(3, 3);
//    customPlot->yAxis2->setSubTickLength(1, 1);

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

