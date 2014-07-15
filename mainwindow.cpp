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
#include <QToolTip>
#include "QCustomPlot.h"

#define COLUMN_COUNT 14



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->mainToolBar->setMinimumHeight(32);
    ui->mainToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->pageControl->setCurrentWidget(ui->startPage);
    //this->checkFilePathLine();

    // install eventFilter
    ui->plotView->installEventFilter(this);

    // graph and table splitter resize
//    ui->splitter->setStretchFactor(0, 1);
//    ui->splitter->setStretchFactor(1, 0);
    QList<int> sizes;
    sizes << 250 << 100;
    ui->splitter->setSizes(sizes);
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

    connect(ui->plotView, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
    connect(ui->plotView, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));

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
       // this->setupGraph();
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

   // this->plot();
    this->setupGraph();
    this->plot(MainWindow::O2Consumption, Qt::blue);
    this->populateTable();

    return 0;
}

void MainWindow::on_cancelButton_clicked()
{
    ui->pageControl->setCurrentWidget(ui->startPage);
}

void MainWindow::setupGraph()
{
    // legend
    ui->plotView->legend->setVisible(true);
    QFont legendFont = font();
    legendFont.setPointSize(9); // and make a bit smaller for legend
    ui->plotView->legend->setFont(legendFont);
    ui->plotView->legend->setBrush(QBrush(QColor(255,255,255,150)));
    // to change legend placement:
    ui->plotView->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignBottom|Qt::AlignRight);
    // graph interactions
    ui->plotView->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables | QCP::iSelectAxes | QCP::iSelectLegend);

    // set a fixed tick-step to one tick per month:
//      ui->plotView->xAxis->setTickLabelType(QCPAxis::ltDateTime);
//      ui->plotView->xAxis->setDateTimeFormat("yyyy/MM/dd-HH:mm:ss");
//      ui->plotView->xAxis->setAutoTickStep(true);
//      ui->plotView->axisRect()->setupFullAxesBox();
//      ui->plotView->xAxis->setTickStep(3600);
//      ui->plotView->xAxis->rescale(true);
//      ui->plotView->xAxis->
//     // ui->plotView->xAxis->setTickStep(2628000); // one month in seconds
//      ui->plotView->xAxis->setSubTickCount(5);
    // x-axis setup


    for (int i = 0; i < timeStamp.size(); i++)
    {
        QString sFormat = "yyyy-MM-dd-HH-mm-ss";
        QDateTime tmpTime = QDateTime::fromString(timeStamp[i], sFormat);
        double dTime = (double) tmpTime.toTime_t();
        x.append(dTime);
    }
    ui->plotView->xAxis->setRange(x.first(), x.last());

    ui->plotView->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    ui->plotView->xAxis->setDateTimeFormat("yyyy/MM/dd\nHH:mm:ss");
    //ui->plotView->axisRect()->setupFullAxesBox();
   // ui->plotView->rescaleAxes(true);

}

void MainWindow::plot(/*QCustomPlot * customPlot*/)
{
    /* setting up the plot */
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
//    connect(ui->horizontalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(horzScrollBarChanged(int)));
//    connect(ui->verticalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(vertScrollBarChanged(int)));
//    connect(ui->plotView->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(xAxisChanged(QCPRange)));
//    connect(ui->plotView->yAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(yAxisChanged(QCPRange)));

    ui->plotView->addGraph();
    ui->plotView->graph(0)->setPen(QPen(Qt::red));
    ui->plotView->graph(0)->setData(x,w);
    ui->plotView->addGraph();
    ui->plotView->graph(1)->setPen(QPen(Qt::blue));
    ui->plotView->graph(1)->setBrush(QBrush(QColor(0, 0, 255, 20)));
    ui->plotView->graph(1)->setData(x,y);
  //  ui->plotView->graph(1)->setValueAxis(ui->plotView->yAxis2);


    ui->plotView->yAxis2->setRange(10,50);

    ui->plotView->yAxis2->setVisible(true);
    ui->plotView->addGraph(0,ui->plotView->yAxis2);
    ui->plotView->graph(2)->setPen(QPen(Qt::green));
    ui->plotView->graph(2)->setBrush(QBrush(QColor(34,139,34, 35)));
    ui->plotView->graph(2)->setData(x,a);
    ui->pageControl->setCurrentWidget(ui->plotPage);
 //   ui->plotView->axisRect();


}

void MainWindow::plot(int parameter, Qt::GlobalColor color)
{
    QVector<double> value(readings[parameter].size());
    value = readings[parameter].toVector();
    ui->plotView->addGraph();
    QPen pen;
    pen.setColor(QColor(0, 0, 255, 200));

    ui->plotView->graph()->setData(x, value);
    ui->pageControl->setCurrentWidget(ui->plotPage);

}

void MainWindow::populateTable()
{
    int rowCount = timeStamp.size();
    ui->tableWidget->setRowCount(rowCount);
    ui->tableWidget->setColumnCount(COLUMN_COUNT);

    QString  text;
    double reading;
    for(int row=0; row < ui->tableWidget->rowCount(); row++)
    {
        // populate time stamps
        text = timeStamp[row];
        ui->tableWidget->setItem(row,0,new QTableWidgetItem(text));
        // populate the readings
        for (int col=1; col< ui->tableWidget->columnCount(); col++)
        {
            reading = readings[col-1].at(row);
            ui->tableWidget->setItem(row,col,new QTableWidgetItem(QString::number(reading)));
        }
    }
    // resize column and row to content
    ui->tableWidget->resizeRowsToContents();
    ui->tableWidget->resizeColumnsToContents();
}

void MainWindow::horzScrollBarChanged(int value)
{
//  if (qAbs(ui->plotView->xAxis->range().center()-value/100.0) > 0.01) // if user is dragging plot, we don't want to replot twice
//  {
//    ui->plotView->xAxis->setRange(value/100.0, ui->plotView->xAxis->range().size(), Qt::AlignCenter);
//    ui->plotView->replot();
//  }
}

void MainWindow::vertScrollBarChanged(int value)
{
//  if (qAbs(ui->plotView->yAxis->range().center()+value/100.0) > 0.01) // if user is dragging plot, we don't want to replot twice
//  {
//    ui->plotView->yAxis->setRange(-value/100.0, ui->plotView->yAxis->range().size(), Qt::AlignCenter);
//    ui->plotView->replot();
//  }
}

void MainWindow::xAxisChanged(QCPRange range)
{
//  ui->horizontalScrollBar->setValue(qRound(range.center()*100.0)); // adjust position of scroll bar slider
//  ui->horizontalScrollBar->setPageStep(qRound(range.size()*100.0)); // adjust size of scroll bar slider
}

void MainWindow::yAxisChanged(QCPRange range)
{
//  ui->verticalScrollBar->setValue(qRound(-range.center()*100.0)); // adjust position of scroll bar slider
//  ui->verticalScrollBar->setPageStep(qRound(range.size()*100.0)); // adjust size of scroll bar slider
}

void MainWindow::xAxisLimit(QCPRange newRange)
{
    QCPRange boundedRange = newRange;
    double lowerRangeBound = x.first();
    double upperRangeBound = x.last();
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

void MainWindow::mousePress()
{
  // if an axis is selected, only allow the direction of that axis to be dragged

  if (ui->plotView->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
  {
      ui->plotView->axisRect()->setRangeDrag(ui->plotView->xAxis->orientation());
  }
  else if (ui->plotView->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
  {
      ui->plotView->axisRect()->setRangeDrag(ui->plotView->yAxis->orientation());
  }
  else
  {
      ui->plotView->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
  }
}

void MainWindow::mouseWheel()
{
  // if an axis is selected, only allow the direction of that axis to be zoomed

  if (ui->plotView->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
  {
      ui->plotView->axisRect()->setRangeZoom(ui->plotView->xAxis->orientation());
  }
  else if (ui->plotView->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
  {
      ui->plotView->axisRect()->setRangeZoom(ui->plotView->yAxis->orientation());
  }
}


bool MainWindow::eventFilter(QObject *target, QEvent *event)
{
    if(target == ui->plotView && event->type() == QEvent::MouseMove)
    {
        QMouseEvent * mouseEvent = static_cast<QMouseEvent*>(event);
        double x = ui->plotView->xAxis->pixelToCoord(mouseEvent->pos().x());
        double y = ui->plotView->yAxis->pixelToCoord(mouseEvent->pos().y());

        // zooming functinality
        if (y <= 0 && x >= MainWindow::x.first())
        {
            // zooming in x direction
            ui->plotView->axisRect()->setRangeZoom(ui->plotView->xAxis->orientation());
        }
        else if (x <= MainWindow::x.first() && y >= 0)
        {
            // zooming in y direction
            ui->plotView->axisRect()->setRangeZoom(/*ui->plotView->yAxis->orientation()*/Qt::Vertical);
        }
        else
        {
            // zooming in x and y directions;
            ui->plotView->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
        }


        //QCPAbstractPlottable *plottable = ui->plotView->plottableAt(mouseEvent->posF());
        //if(plottable)
        //{
        //      double x = ui->plotView->pixelToCoord(mouseEvent->posF().x());
        //}

        // getting the selected graph coordinates and showing it in a tooltip
        QToolTip::hideText();
        if( !ui->plotView->selectedGraphs().isEmpty())
        {
            double distance = ui->plotView->selectedGraphs().first()->selectTest(mouseEvent->posF(),true);
            if( distance < 10 && distance > 0)
            {
                double x = ui->plotView->selectedGraphs().first()->keyAxis()->pixelToCoord(mouseEvent->posF().x());
                QDateTime xDateTime;
                xDateTime.setTime_t(x);
                double y = ui->plotView->selectedGraphs().first()->valueAxis()->pixelToCoord(mouseEvent->posF().y());


                // showing the tooltip
                QToolTip::showText(mouseEvent->globalPos(),
                                   tr("<table>"
                                      "<tr>"
                                      "<td>x:</td>" "<td>%L2</td>"
                                      "</tr>"
                                      "<tr>"
                                      "<td>y:</td>" "<td>%L3</td>"
                                      "</tr>"
                                      "</table>").
                                   arg(xDateTime.toString()).
                                   arg(y),
                                   ui->plotView, ui->plotView->rect());
            }
        }
    }
    return false;
}

void MainWindow::selectionChanged()
{
  // make left and right axes be selected synchronously, and handle axis and tick labels as one selectable object:
//  if (ui->plotView->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->plotView->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
//      ui->plotView->yAxis2->selectedParts().testFlag(QCPAxis::spAxis) || ui->plotView->yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
//  {
//    ui->plotView->yAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
//    ui->plotView->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
//  }

  // synchronize selection of graphs with selection of corresponding legend items:
  for (int i=0; i<ui->plotView->graphCount(); ++i)
  {
    QCPGraph *graph = ui->plotView->graph(i);
    QCPPlottableLegendItem *item = ui->plotView->legend->itemWithPlottable(graph);
    if (item->selected() || graph->selected())
    {
      item->setSelected(true);
      graph->setSelected(true);
    }
    if(!item->selected() || !graph->selected())
    {
        item->setSelected(false);
        graph->setSelected(false);
    }
  }
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
