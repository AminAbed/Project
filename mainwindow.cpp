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
#include <QSettings>
#include <QProcess>
#include <QFileDialog>
#include <QToolButton>
#include <QSignalMapper>
#include <QtWebKit/QWebView>
#include <QResource>
#include <QCheckBox>
#include "CommentWindow.h"
#include "QCustomPlot.h"
#include "SettingsPage.h"
#include "CommentTracer.h"

#define COLUMN_COUNT 15
#define NOTES_COLUMN 14

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    RE(false),
    ERE(false),
    O2(false),
    MinTemp(false),
    MaxTemp(false),
    MinRH(false),
    MaxRH(false),
    isCursorCloseToGraph(false)
{
    ui->setupUi(this);

    ui->mainToolBar->setMinimumHeight(32);
//    ui->mainToolBar->setMaximumWidth(300);
    ui->mainToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->pageControl->setCurrentWidget(ui->startPage);
    ui->actionSettings->setVisible(false);
    ui->actionOpenAnotherWindow->setVisible(false);
    ui->actionPDF->setVisible(false);
    ui->actionRescale->setVisible(false);

    this->nextTracerIndex = 0;




    //setWindowTitle( windowTitle() + " (" + Version::BUILD_NO + ")" );

    restoreWindowState();
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
    saveWindowState();
    delete ui;
}

void MainWindow::restoreWindowState()
{
    QSettings settings("../settings.ini", QSettings::IniFormat);
    settings.beginGroup("MainWindow");
    this->resize(settings.value("size", QSize(640, 480)).toSize());
    this->move(settings.value("pos", QPoint(0, 0)).toPoint());
    if (settings.value("isMaximized", false).toBool())
    {
        this->setWindowState(Qt::WindowMaximized);
    }
}

void MainWindow::saveWindowState()
{
    QSettings settings("../settings.ini", QSettings::IniFormat);
    settings.beginGroup("MainWindow");
    settings.setValue("isMaximized", this->isMaximized());
    if (! this->isMaximized())
    {
        settings.setValue("size", this->size());
        settings.setValue("pos", this->pos());
    }
}
void MainWindow::on_actionOpen_triggered()
{
    ui->actionOpen->setEnabled(false);
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

    // connect axis, tick labels and legend selections
    connect(ui->plotView, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));

    connect(ui->plotView, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
    connect(ui->plotView, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));

    connect(ui->plotView, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));

    ui->plotView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->plotView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(menuRequest(QPoint)));

    commentWindow = & CommentWindow::getInstance();
    connect(commentWindow, SIGNAL(commentSubmitted(QString )), this, SLOT(commentSubmitted(QString )));
    connect(this, SIGNAL(hasComments(QString )), commentWindow, SLOT(setCommentText(QString )));
    connect(commentWindow, SIGNAL(commentDeleted()), this, SLOT(commentDeleted()));
    connect(commentWindow, SIGNAL(cancelled()), this, SLOT(cancelled()));

    ui->filePathline->clear();
}

void MainWindow::on_actionSettings_triggered()
{
   // QSettings settings("../settings.ini", QSettings::IniFormat);
   // settings.value("size", QSize(640, 480)).toSize());
  //  this->move(settings.value("pos",);
    connect(&settingsPage, SIGNAL(removeAll()), this, SLOT(removeAllGraphs()));
    connect(&settingsPage, SIGNAL(plot(QString ,bool )), this, SLOT(actionMapper(QString ,bool )));
    connect(&settingsPage, SIGNAL(remove(QString ,bool )), this, SLOT(actionMapper(QString ,bool )));
    connect(this, SIGNAL(toggleCheckBox(QString ,bool )), &settingsPage, SLOT(updateCheckBoxes(QString ,bool )));

//    QPoint pos = this->pos();
//    QSize size = this->size();
//    QPoint settingsPagePos_x = pos.x() + size.width();
//    QPoint settingsPagePos_y = pos.y();


   // settingsPage.move(700,00);
    settingsPage.show();
}


void MainWindow::on_openButton_clicked()
{
    filePath = ui->filePathline->text();
    if (!filePath.isEmpty())
    {
        if(!filePath.endsWith(".csv") || !filePath.contains("_summaryFile"))
        {
            QMessageBox::warning(this,"",tr("The supplied file is not supported."));
            return;
        }
    }
    else
    {
        QMessageBox::warning(this,"",tr("Please provide a summary file for analysis."));
        return;
    }   
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
    if (!filePath.contains(".csv")) return;

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
    fileSystemModel->setFilter(QDir::Dirs|QDir::Drives|QDir::NoDotAndDotDot|QDir::AllDirs|QDir::AllEntries);
    fileSystemModel->setHeaderData(1,Qt::Horizontal,Qt::EditRole);
    fileSystemModel->setRootPath("");
    ui->fileSystemView->clearSelection();
    ui->fileSystemView->setModel(fileSystemModel);
    ui->fileSystemView->header()->setMovable(false);
    ui->fileSystemView->hideColumn(1);
    ui->fileSystemView->hideColumn(2);
    ui->fileSystemView->setColumnWidth(0,250);
    ui->fileSystemView->setColumnWidth(3,250);

    QSettings settings("../settings.ini", QSettings::IniFormat);
    QString lastDir = settings.value("app/last_dir", "").toString();
    if (lastDir.isEmpty()) return;

    QModelIndex index = fileSystemModel->index(lastDir);

    while (index.isValid())
    {
        ui->fileSystemView->expand(index);
        index = fileSystemModel->parent(index);
    }
    // now that all is expanded, scroll to the selected folder
     ui->fileSystemView->scrollTo(index, QAbstractItemView::PositionAtTop);
     ui->fileSystemView->setRootIndex(index);
}

int MainWindow::readSession(QString filePath)
{
    // remember the folder this file came from for later time
    QFileInfo fileInfo(filePath);
    QSettings settings("../settings.ini", QSettings::IniFormat);
    settings.setValue("app/last_dir", fileInfo.absolutePath());

    if (!filePath.contains(".csv")) return -1;
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
        // ignoring nans and -255's
        if (line.contains("-255")||line.contains("nan")) continue;
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

    this->setupGraph();
    this->populateTable();

    ui->pageControl->setCurrentWidget(ui->plotPage);
    this->loadCommentTracerDB();
    qDebug() << "commentTracerList.count()" << commentTracerList.count();

    ui->horizontalScrollBar->setVisible(false);
    ui->verticalScrollBar->setVisible(false);

    ui->actionOpen->setEnabled(true);
    ui->actionOpen->setVisible(false);
    ui->actionOpenAnotherWindow->setVisible(true);
//    ui->actionPDF->setVisible(true);
    ui->actionSettings->setVisible(false);

    ui->actionRescale->setVisible(true);
    // create a context menue in toolbar
    QMenu *menu = new QMenu();
    QAction * plotPDF = new QAction("Plot to PDF", this);
    QAction * tablePDF = new QAction("Table to PDF", this);
    menu->addAction(plotPDF);
    menu->addAction(tablePDF);

    QToolButton* pdfButton = new QToolButton();
    pdfButton->setIcon(QIcon("://images/pdf.jpg"));
    pdfButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    pdfButton->setText("PDF");
    pdfButton->setToolTip("Save to PDF");
    pdfButton->setMenu(menu);
    pdfButton->setPopupMode(QToolButton::InstantPopup);
    ui->mainToolBar->addWidget(pdfButton);
    connect(plotPDF, SIGNAL(triggered()), this, SLOT(on_actionPlotPDF_triggered()));
    connect(tablePDF, SIGNAL(triggered()),this, SLOT(on_actionTablePDF_triggered()));


    // spacer between PDF button and next buttons
    QWidget* empty = new QWidget();
    empty->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    ui->mainToolBar->addWidget(empty);

    tracerCheckBox = new QCheckBox(ui->mainToolBar);
    ui->mainToolBar->addWidget(tracerCheckBox);
    tracerCheckBox->setText("Hide Comment Indicators");
    tracerCheckBox->setToolTip("Show/hide comment indicators");
    connect(tracerCheckBox, SIGNAL(toggled(bool)), this, SLOT(on_hideCommentTracer_toggled(bool)));
    // make the checkbox disabled
    tracerCheckBox->setEnabled(false);

    // plot default graphs
    plot(MainWindow::respiratoryEnthalpy, Qt::red, "RE");
    plot(MainWindow::O2Consumption, Qt::darkCyan, "O2");
    RE = O2 = true;

    // add comment tracers (if applicable)
    for(int i = 0; i < ui->plotView->graphCount(); i++)
    {
        if(ui->plotView->graph(i)->name() == "RE" || ui->plotView->graph(i)->name() == "O2")
            this->showCommentTracers(ui->plotView->graph(i));
    }

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
    ui->plotView->legend->setSelectableParts(QCPLegend::spItems);
    // graph interactions
    ui->plotView->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables | QCP::iSelectAxes | QCP::iSelectLegend );

    // set a fixed tick-step to one tick per month:
//      ui->plotView->xAxis->setTickLabelType(QCPAxis::ltDateTime);
//      ui->plotView->xAxis->setDateTimeFormat("yyyy/MM/dd-HH:mm:ss");
//      ui->plotView->xAxis->setAutoTickStep(true);
//      ui->plotView->axisRect()->setupFullAxesBox();
      ui->plotView->xAxis->setTickStep(60);
//      ui->plotView->xAxis->rescale(true);
//      ui->plotView->xAxis->
//      ui->plotView->xAxis->setTickStep(2628000); // one month in seconds
//      ui->plotView->xAxis->setSubTickCount(5);



    for (int i = 0; i < timeStamp.size(); i++)
    {
        QString sFormat = "yyyy-MM-dd-HH-mm-ss";
        QDateTime tmpTime = QDateTime::fromString(timeStamp[i], sFormat);
        double dTime = (double) tmpTime.toTime_t();
        x.append(dTime);
    }

    // finding continious segments in timestamp
//    indexes.append(0);
//    for (int i = 0; i <x.size(); i++)
//    {
//        if(x[i+1] - x[i] > 300)
//        {
//            indexes.append(i+1);
//        }
//    }
//    indexes.append(x.count());
//    qDebug() << indexes;

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


    ui->plotView->replot();
}

void MainWindow::plot(int parameter, Qt::GlobalColor color, QString name)
{
    QVector<double> value(readings[parameter].size());
    value = readings[parameter].toVector();
    ui->plotView->addGraph();
    ui->plotView->graph()->setLineStyle(QCPGraph::lsLine);
    ui->plotView->graph()->setScatterStyle(QCPScatterStyle::ssPeace);
    ui->plotView->graph()->setPen(QPen(color));
    ui->plotView->graph()->setName(name);

//    ui->plotView->yAxis->setRange(value.first(),value.last());
//    ui->plotView->yAxis->rescale(true);
    //ui->plotView->graph()->rescaleValueAxis();
//    ui->plotView->rescaleAxes();

    // plotting continuous segments
//    QVector<double> xDivided;
//    QVector<double> yDivided;
//    for (int i = 0; i < indexes.size(); i++)
//    {
//        qDebug() << " i is " << i;
//        for(int j = indexes.at(i); j <indexes.at(i+1); j++)
//        {
//            xDivided.append(x.at(j));
//            yDivided.append(value.at(j));
//        }
//        QDateTime xDateTime;
//        for(int k = 0; k < xDivided.size(); k++)
//        {
//            xDateTime.setTime_t(xDivided.at(k));
//            qDebug() << xDateTime;
//        }
//        ui->plotView->graph()->setPen(QPen(color));
//        ui->plotView->graph()->setData(xPlot, yPlot);

        ui->plotView->graph()->setData(x,value);
        ui->plotView->rescaleAxes();
        ui->plotView->replot();

//        xDivided.clear();
//        yDivided.clear();
//    }
   // ui->plotView->graph()->setData(x, value);
}

void MainWindow::removeSelectedGraph()
{
  if (ui->plotView->selectedGraphs().size() > 0)
  {
      // needs better structuring... maybe putting a graph and its properties in a struct
      QString name = ui->plotView->selectedGraphs().first()->name();
      if (name == "O2") O2 = false;
      if (name == "RE") RE = false;
      if (name == "ERE") ERE = false;
      if (name == "Min Temp") MinTemp = false;
      if (name == "Max Temp") MaxTemp = false;
      if (name == "Min RH") MinRH = false;
      if (name == "Max RH") MaxRH = false;

      this->hideCommentTracers(ui->plotView->selectedGraphs().first());
      ui->plotView->removeGraph(ui->plotView->selectedGraphs().first());
      ui->plotView->rescaleAxes();
      ui->plotView->replot();

  }
}

void MainWindow::removeAllGraphs()
{
    tracerCheckBox->setEnabled(false);
    RE = ERE = O2 = MinTemp = MaxTemp = MinRH = MaxRH = false;
    ui->plotView->clearGraphs();
    ui->plotView->clearItems();
    O2ItemTracerPtrList.clear();
    reItemTracerPtrList.clear();
    ui->plotView->replot();
}

bool MainWindow::removeGraphByName(QString name)
{
    int foundIndex = -1;
    bool success = false;
    for (int i=0; i<ui->plotView->graphCount(); ++i)
    {
      if (ui->plotView->graph(i)->name() == name)
      {
        foundIndex = i;
        ui->plotView->removeGraph(foundIndex);
        ui->plotView->rescaleAxes();
        ui->plotView->replot();
        success = true;
        break;
      }
    }
    return success;
}

void MainWindow::menuRequest(QPoint pos)
{
    QMenu * menu = new QMenu(this);
    if(isCursorCloseToGraph)
    {
        QString selectedGraph = ui->plotView->selectedGraphs().first()->name();
        if(selectedGraph == "RE" || selectedGraph == "O2")
        {
            QMenu * commentMenu = menu->addMenu("&Notes");
            actionAddComment = commentMenu->addAction("&Add/Edit Notes");
            if(tracerList.contains(mappedXAxisPosition) && tracerList[mappedXAxisPosition] == ui->plotView->selectedGraphs().first()->name())
            {
                actionRemoveComment = commentMenu->addAction("&Remove Notes");
            }
            qDebug() << "tracer index" << this->findCommentTracer(mappedXAxisPosition);
        }
    }

    QMenu * addSubMenu = menu->addMenu("&Add Graphs");
    menu->setAttribute(Qt::WA_DeleteOnClose);

    actionREAdd = addSubMenu->addAction( "RE");
    actionEREAdd = addSubMenu->addAction( "ERE" );
    actionO2Add = addSubMenu->addAction( "O2" );
    actionMinTempAdd = addSubMenu->addAction( "Min Temp" );
    actionMaxTempAdd = addSubMenu->addAction( "Max Temp" );
    actionMinRHAdd = addSubMenu->addAction( "Min RH" );
    actionMaxRHAdd = addSubMenu->addAction( "Max RH" );

    actionREAdd->setCheckable(true);
    actionEREAdd->setCheckable(true);
    actionO2Add->setCheckable(true);
    actionMinTempAdd->setCheckable(true);
    actionMaxTempAdd->setCheckable(true);
    actionMinRHAdd->setCheckable(true);
    actionMaxRHAdd->setCheckable(true);




    if (ui->plotView->graphCount() > 0 )
    {
        QMenu * removeMenu = menu->addMenu("&Remove Graphs");
        menu->addAction( "Remove All &Graphs", this, SLOT(removeAllGraphs()));

        actionRERemove = removeMenu->addAction( "RE");
        actionERERemove = removeMenu->addAction( "ERE" );
        actionO2Remove = removeMenu->addAction( "O2" );
        actionMinTempRemove = removeMenu->addAction( "Min Temp" );
        actionMaxTempRemove = removeMenu->addAction( "Max Temp" );
        actionMinRHRemove = removeMenu->addAction( "Min RH" );
        actionMaxRHRemove = removeMenu->addAction( "Max RH" );

        actionRERemove->setCheckable(true);
        actionERERemove->setCheckable(true);
        actionO2Remove->setCheckable(true);
        actionMinTempRemove->setCheckable(true);
        actionMaxTempRemove->setCheckable(true);
        actionMinRHRemove->setCheckable(true);
        actionMaxRHRemove->setCheckable(true);
        this->toggleMenu();
    }


    if (ui->plotView->selectedGraphs().size() > 0)
    {
        menu->addAction("Remove &Selected Graph", this, SLOT(removeSelectedGraph()));
    }

    connect(menu, SIGNAL(triggered(QAction *)), this, SLOT(actionMapper(QAction *)));

    menu->popup(ui->plotView->mapToGlobal(pos));
}

bool MainWindow::removeGraphByAction(QAction * action)
{
    int foundIndex = -1;
    bool success = false;
    QString theName = action->text();
    for (int i=0; i<ui->plotView->graphCount(); ++i)
    {
      if (ui->plotView->graph(i)->name() == theName)
      {
        foundIndex = i;
        ui->plotView->removeGraph(foundIndex);
        ui->plotView->rescaleAxes();
        ui->plotView->replot();
        success = true;
        break;
      }
    }
    return success;
}
void MainWindow::actionMapper(QString action, bool checked )
{
    QString triggeredAction = action;
    if (triggeredAction == "&Add/Edit Notes") actionMapper(actionAddComment);
    if (triggeredAction == "RE" && checked) actionMapper(actionREAdd);
    else if(triggeredAction == "RE" && !checked)
    {
       removeGraphByName(triggeredAction);
       RE = false;
    }

    if (triggeredAction == "ERE" && checked) actionMapper(actionEREAdd);
    else if(triggeredAction == "ERE" && !checked)
    {
       removeGraphByName(triggeredAction);
       ERE = false;
    }

    if (triggeredAction == "O2" && checked) actionMapper(actionO2Add);
    else if(triggeredAction == "O2" && !checked)
    {
       removeGraphByName(triggeredAction);
       O2 = false;
    }

    if (triggeredAction == "Min Temp" && checked) actionMapper(actionMinTempAdd);
    else if(triggeredAction == "Min Temp" && !checked)
    {
       removeGraphByName(triggeredAction);
       MinTemp = false;
    }

    if (triggeredAction == "Max Temp" && checked) actionMapper(actionMaxTempAdd);
    else if(triggeredAction == "Max Temp" && !checked)
    {
       removeGraphByName(triggeredAction);
       MaxTemp = false;
    }

    if (triggeredAction == "Min RH" && checked) actionMapper(actionMinRHAdd);
    else if(triggeredAction == "Min RH" && !checked)
    {
       removeGraphByName(triggeredAction);
       MinRH = false;
    }

    if (triggeredAction == "Max RH" && checked) actionMapper(actionMaxRHAdd);
    else if(triggeredAction == "Max RH" && !checked)
    {
       removeGraphByName(triggeredAction);
       MaxRH = false;
    }
}

void MainWindow::actionMapper(QAction * action)
{
    // add comments/tracers
    if (action == actionAddComment)
    { 
        this->setupCommentWindow(ui->plotView->selectedGraphs().first()->name());
    }

    // remove comment and comment tracer
    if (action == actionRemoveComment)
    {
  //      this->hideComments();
        this->removeCommentTracer(mousePosition);
        this->removeComment(mappedXAxisPosition);
    }
    // add a graph
    ui->statusBar->showMessage("Loading the graph...");
    if (action == actionREAdd )
    {
        plot(MainWindow::respiratoryEnthalpy, Qt::red, "RE");
        this->showCommentTracers(ui->plotView->graph());
    //    emit toggleCheckBox(action->text(), true);
        RE = true;

    }
    else if (action == actionEREAdd)
    {
        plot(MainWindow::eRespiratoryEnthalpy, Qt::darkRed, "ERE");
  //      emit toggleCheckBox(actionName, true);
        ERE = true;
    }
    else if (action == actionO2Add)
    {
        plot(MainWindow::O2Consumption, Qt::darkCyan, "O2");
        this->showCommentTracers(ui->plotView->graph());
  //      emit toggleCheckBox(actionName, true);
        O2 = true;
    }
    else if (action == actionMaxTempAdd)
    {
        plot(MainWindow::maxTemp, Qt::blue, "Max Temp");
   //     emit toggleCheckBox(actionName, true);
        MaxTemp = true;
    }
    else if (action == actionMinTempAdd)
    {
        plot(MainWindow::minTemp, Qt::darkBlue, "Min Temp");
   //     emit toggleCheckBox(actionName, true);
        MinTemp = true;
    }
    else if (action == actionMaxRHAdd)
    {
        plot(MainWindow::maxRH, Qt::green, "Max RH");
   //     emit toggleCheckBox(actionName, true);
        MaxRH = true;
    }
    else if (action == actionMinRHAdd)
    {
        plot(MainWindow::minRH, Qt::darkGreen, "Min RH");
    //    emit toggleCheckBox(actionName, true);
        MinRH = true;
    }

    // remove a graph
    if (action == actionRERemove )
    {
        // hide tracers
        for (int i=0; i < ui->plotView->graphCount(); ++i)
        {
            if(ui->plotView->graph(i)->name() == "RE")
                this->hideCommentTracers(ui->plotView->graph(i));
        }
        // disable checkbox
        if(!ui->plotView->selectedGraphs().isEmpty())
            if(ui->plotView->selectedGraphs().first()->name() == "RE")
                tracerCheckBox->setDisabled(true);

        // remove graph
        this->removeGraphByAction(action);
 //       emit toggleCheckBox(action->text(), false);
        RE = false;
    }
    else if (action == actionERERemove)
    {
        this->removeGraphByAction(action);
   //     emit toggleCheckBox(actionName, false);
        ERE = false;
    }
    else if (action == actionO2Remove)
    {
        // hide tracers
        for (int i=0; i < ui->plotView->graphCount(); ++i)
        {
            if(ui->plotView->graph(i)->name() == "O2")
                this->hideCommentTracers(ui->plotView->graph(i));
        }
        // disable the checkbox
        if(!ui->plotView->selectedGraphs().isEmpty())
            if(ui->plotView->selectedGraphs().first()->name() == "O2")
                tracerCheckBox->setDisabled(true);
        // remove graph
        this->removeGraphByAction(action);
    //    emit toggleCheckBox(actionName, false);
        O2 = false;
    }
    else if (action == actionMaxTempRemove)
    {
        this->removeGraphByAction(action);
   //     emit toggleCheckBox(actionName, false);
        MaxTemp = false;
    }
    else if (action == actionMinTempRemove)
    {
        this->removeGraphByAction(action);
 //       emit toggleCheckBox(actionName, false);
        MinTemp = false;
    }
    else if (action == actionMaxRHRemove)
    {
        this->removeGraphByAction(action);
  //      emit toggleCheckBox(actionName, false);
        MaxRH = false;
    }
    else if (action == actionMinRHRemove)
    {
        this->removeGraphByAction(action);
    //    emit toggleCheckBox(actionName, false);
        MinRH = false;
    }
    ui->plotView->replot();
    ui->statusBar->showMessage(" ");
}

void MainWindow::toggleMenu()
{
    if (RE)
    {
        actionREAdd->setChecked(true);
        actionREAdd->setEnabled(false);
        actionRERemove->setEnabled(true); 
    }
    else
    {
        actionREAdd->setChecked(false);
        actionREAdd->setEnabled(true);
        actionRERemove->setEnabled(false);
    }

    if (ERE)
    {
        actionEREAdd->setChecked(true);
        actionEREAdd->setEnabled(false);
        actionERERemove->setEnabled(true);
    }
    else
    {
        actionEREAdd->setChecked(false);
        actionEREAdd->setEnabled(true);
        actionERERemove->setEnabled(false);
    }

    if (O2)
    {
        actionO2Add->setChecked(true);
        actionO2Add->setEnabled(false);
        actionO2Remove->setEnabled(true);
    }
    else
    {
        actionO2Add->setChecked(false);
        actionO2Add->setEnabled(true);
        actionO2Remove->setEnabled(false);
    }

    if (MinTemp)
    {
        actionMinTempAdd->setChecked(true);
        actionMinTempAdd->setEnabled(false);
        actionMinTempRemove->setEnabled(true);
    }
    else
    {
        actionMinTempAdd->setChecked(false);
        actionMinTempAdd->setEnabled(true);
        actionMinTempRemove->setEnabled(false);
    }

    if (MaxTemp)
    {
        actionMaxTempAdd->setChecked(true);
        actionMaxTempAdd->setEnabled(false);
        actionMaxTempRemove->setEnabled(true);
    }
    else
    {
        actionMaxTempAdd->setChecked(false);
        actionMaxTempAdd->setEnabled(true);
        actionMaxTempRemove->setEnabled(false);
    }

    if (MinRH)
    {
        actionMinRHAdd->setChecked(true);
        actionMinRHAdd->setEnabled(false);
        actionMinRHRemove->setEnabled(true);
    }
    else
    {
        actionMinRHAdd->setChecked(false);
        actionMinRHAdd->setEnabled(true);
        actionMinRHRemove->setEnabled(false);
    }

    if (MaxRH)
    {
        actionMaxRHAdd->setChecked(true);
        actionMaxRHAdd->setEnabled(false);
        actionMaxRHRemove->setEnabled(true);
    }
    else
    {
        actionMaxRHAdd->setChecked(false);
        actionMaxRHAdd->setEnabled(true);
        actionMaxRHRemove->setEnabled(false);
    }
}

void MainWindow::populateTable()
{
    QSettings commentFile("../comments.ini", QSettings::IniFormat);
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
        for (int col=1; col< ui->tableWidget->columnCount()-1; col++)
        {

            reading = readings[col-1].at(row);
            ui->tableWidget->setItem(row,col,new QTableWidgetItem(QString::number(reading)));
        }

        // inserting the comments
        if(commentFile.value(QString::number(row),"-1") != "-1")
        {
            ui->tableWidget->setItem(row,NOTES_COLUMN,new QTableWidgetItem(commentFile.value(QString::number(row)).toString()));
        }
    }
    // resize column and row to content
    ui->tableWidget->resizeRowsToContents();
    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->setColumnWidth(NOTES_COLUMN, 250);
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
    double upperRangeBound = 70;
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
        isCursorCloseToGraph = false;
        if( !ui->plotView->selectedGraphs().isEmpty())
        {
            double distance = ui->plotView->selectedGraphs().first()->selectTest(mouseEvent->posF(),true);
            if( distance < 10 && distance > 0)
            {
                mousePosition.setX(mouseEvent->posF().x());
                mousePosition.setY(mouseEvent->posF().y());
                isCursorCloseToGraph = true;
                double xAxisValue = ui->plotView->selectedGraphs().first()->keyAxis()->pixelToCoord(mouseEvent->posF().x());
                QDateTime xDateTime;
                xDateTime.setTime_t(xAxisValue);
                double y = ui->plotView->selectedGraphs().first()->valueAxis()->pixelToCoord(mouseEvent->posF().y());

                mappedXAxisPosition = dataPointMap(xAxisValue);
                int commentIndex = this->findCommentTracer(mappedXAxisPosition);
//                if(commentIndex>0) //&& !ui->tableWidget->item(commentIndex, NOTES_COLUMN)->text().isEmpty()*)
//                qDebug() << ui->tableWidget->item(commentIndex, NOTES_COLUMN)->text();
                QString comment = "";
                if(commentIndex >= 0)
                comment = ui->tableWidget->item(commentIndex, NOTES_COLUMN)->text();
                else comment = "N/A";

                QString selectedGraph = ui->plotView->selectedGraphs().first()->name();
                // showing the tooltip
                QToolTip::showText(mouseEvent->globalPos(),
                                   tr("<table>"
                                      "<tr>"
                                      "<td>Date/Time:</td>" "<td>%L1</td>"
                                      "</tr>"
                                      "<tr>"
                                      "<td>%L2:</td>" "<td>%L3</td>"
                                      "</tr>"
                                      "<tr>"
                                      "<td>Notes:</td>" "<td>%L4</td>"
                                      "</tr>"
                                      "</table>").
                                   arg(xDateTime.toString()).
                                   arg(selectedGraph).
                                   arg(y)
                                  .arg(comment),
                                   ui->plotView, ui->plotView->rect());
            }
        }
    }
    return false;
}

void MainWindow::selectionChanged()
{
  // toggle TracerCheckBox
    if(!ui->plotView->selectedGraphs().isEmpty() && (ui->plotView->selectedGraphs().first()->name() == "O2" || ui->plotView->selectedGraphs().first()->name() == "RE"))
    {
        for(int i = 0; i < commentTracerList.count(); i++)
        {
            if(commentTracerList.at(i).name == ui->plotView->selectedGraphs().first()->name())
                tracerCheckBox->setEnabled(true);
        }
    }
    else tracerCheckBox->setEnabled(false);

  // handle axis and tick labels as one selectable object:
  if (ui->plotView->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->plotView->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) /*||
      ui->plotView->yAxis2->selectedParts().testFlag(QCPAxis::spAxis) || ui->plotView->yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels)*/)
  {
   // ui->plotView->yAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    ui->plotView->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
  }

  if (ui->plotView->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->plotView->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) )
  {
    ui->plotView->xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
  }

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
  }
}

void MainWindow::on_actionOpenAnotherWindow_triggered()
{
    qDebug() << "Starting" << QCoreApplication::applicationFilePath() ;
    QProcess::startDetached( QCoreApplication::applicationFilePath(), QStringList() );
    ui->statusBar->showMessage(tr("Starting %1").arg(QCoreApplication::applicationFilePath()), 5000);
}

void MainWindow::on_actionPlotPDF_triggered()
{
    QFileDialog fileDialog;
    fileDialog.setOptions(QFileDialog::DontUseNativeDialog);
    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setDirectory(".");

    fileDialog.selectFile(filePath + "_plot.pdf");
    int ret = fileDialog.exec();
    if (ret == QDialog::Accepted)
    {
        QStringList fileName = fileDialog.selectedFiles();
        ui->plotView->savePdf(fileName[0]/*, false, 3508, 2480*/);
    }
}

void MainWindow::on_actionTablePDF_triggered()
{
    QWebView * htmlRenderer = new QWebView();
    htmlRenderer->setHtml(generateHTML());
    this->saveHTMLToPDF(htmlRenderer);
}

// converts the table to HTML
QString tableWidgetToHTML( QTableWidget * table )
{
    QString html;
    html = "<table>";

    html += "<thead><tr>";
    // table header
    for (int col = 0; col < table->columnCount(); col ++)
    {
        QTableWidgetItem* item = table->horizontalHeaderItem(col);
        if (item)
        {
            html += "<th>" + item->text() + "</th>";
        }
    }
    html += "</tr></thead>";

    html += "<tbody>";
    // table body
    for (int row = 0; row < table->rowCount(); row ++ )
    {
        html += "<tr>";
        for (int col = 0; col < table->columnCount(); col ++)
        {

            QTableWidgetItem* item = table->item(row, col);
            if (item)
            {
                html += "<td>" + item->text() + "</td>";
            }
        }
        html += "</tr>";
    }
    html += "</tbody></table>";
    return html;
}

// loads and fills the HTML tmeplate with content
QString MainWindow::generateHTML()
{
    QString html ( (char*) QResource(":/export-template.html").data() );
    html = html
            .arg( ui->patientInfoLabel->text() )
//            .arg( Version::getSoftwareId() )
//            .arg( ui->patientInfoLabel->text() )
//            .arg( pixmapToImgTag(hlChart) )
//            .arg( pixmapToImgTag(pbChart) )
//            .arg( tableWidgetToHTML(ui->hlPerSample) )
//            .arg( tableWidgetToHTML(ui->perBreath) )
//            .arg( tableWidgetToHTML(ui->inhaleEntropies) )
            .arg(tableWidgetToHTML(ui->tableWidget))

    ;
    return html;
}

// saves the html(table) to PDF
void MainWindow::saveHTMLToPDF(QWebView * renderer)
{
    QWebView *htmlRenderer = renderer;
    if ( htmlRenderer == NULL) return;

    // sets up the printer
    QPrinter printer;
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPrintRange(QPrinter::AllPages);
    printer.setOrientation(QPrinter::Landscape);
    printer.setPaperSize(QPrinter::Letter);
    printer.setResolution(QPrinter::HighResolution);
    printer.setFullPage(false);
    printer.setNumCopies(1);
    printer.setOutputFileName(filePath + "_table.pdf");

    // sets up the dialog box
    QFileDialog fileDialog;
    fileDialog.setOptions(QFileDialog::DontUseNativeDialog);
    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setDirectory(".");
    fileDialog.selectFile(filePath + "_table.pdf");

    int ret = fileDialog.exec();
    if(ret == QDialog::Accepted)
    {
        QStringList fileName = fileDialog.selectedFiles();
        printer.setOutputFileName(fileName[0]);
        htmlRenderer->print(&printer);
    }
    delete renderer;
}

// adds a tracer/marker to the graph for comments
void MainWindow::addCommentTracer(QCPGraph * selectedGraph, double position)
{
    QCPItemTracer *commentTracer = new QCPItemTracer(ui->plotView);
    ui->plotView->addItem(commentTracer);
    commentTracer->setGraph(selectedGraph);
  //         qDebug() << "name is" <<commentTracer->graph()->name();
    commentTracer->setGraphKey(position);
    commentTracer->setInterpolating(true);
    commentTracer->setStyle(QCPItemTracer::tsCircle);
    commentTracer->setPen(QPen(QColor(199,97,20)));
    commentTracer->setBrush(QColor(199,97,20,180));
    commentTracer->setSize(10);

    if(selectedGraph->name()== "O2") O2ItemTracerPtrList.append(commentTracer);
    else reItemTracerPtrList.append(commentTracer);

    qDebug() << "graphs key is" <<commentTracer->graphKey() << "position is "<< position;
    tracerList[position] = selectedGraph->name();
//    QMap<double, int>::const_iterator i = tracerList.constBegin();
//    while (i != tracerList.constEnd()) {
//        qDebug() << i.key() << ": " << i.value() << endl;
//         ++i;
//     }

}

void MainWindow::removeCommentTracer(QPoint mouePosition)
{
    if(ui->plotView->selectedGraphs().first()->name()== "O2")
    {
        for(int i = 0; i < O2ItemTracerPtrList.count(); i++)
            if(O2ItemTracerPtrList[i] == ui->plotView->itemAt(mouePosition))
            {
                O2ItemTracerPtrList.removeAt(i);
                qDebug() << "O2ItemTracerPtrList.count()" << O2ItemTracerPtrList.count();
                break;
            }
    }
    else
    {
        for(int i = 0; i < reItemTracerPtrList.count(); i++)
            if(reItemTracerPtrList[i] == ui->plotView->itemAt(mouePosition))
            {
                reItemTracerPtrList.removeAt(i);
                qDebug() << "reItemTracerPtrList.count()" << reItemTracerPtrList.count();
                break;
            }
    }
    ui->plotView->removeItem(ui->plotView->itemAt(mouePosition));

//    for(int i = 0; i < commentTracerList.count(); i++)
//    {
//        if ( commentTracerList.at(i) == ui->plotView->itemAt(mouePosition) )
//        {

//            break;
//        }
//    }
}

void MainWindow::removeComment(double position)
{
    QSettings commentFile("../comments.ini", QSettings::IniFormat);
    qDebug() << position;
    int index = x.indexOf(position);
    qDebug() << "index to be romved is: "<< index;

    // remove from table
    ui->tableWidget->setItem(index, NOTES_COLUMN, new QTableWidgetItem(""));
    // remove from tracerList
    tracerList.remove(position);
    // remove from commentTracerList and update database file
    for(int i = 0; i < commentTracerList.count(); i++)
    {
        if(commentTracerList[i].position == position)
        {
            commentTracerList.removeAt(i);

            QSettings commentTracerDB("../commentTracerDB.ini", QSettings::IniFormat);
            commentTracerDB.beginWriteArray("commentTracers");
            commentTracerDB.setArrayIndex(i);
            commentTracerDB.remove("");
            commentTracerDB.endArray();

        }
    }
    // removing from commentFile
    commentFile.remove(QString::number(index));

}

// sets up the comment area
void MainWindow::setupCommentWindow(QString name)
{
//    commentWindow = & CommentWindow::getInstance();
//    connect(commentWindow, SIGNAL(commentSubmitted(QString )), this, SLOT(commentSubmitted(QString )));

    copyOfMappedValue = mappedXAxisPosition;
    commentWindow->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    commentWindow->show();
    QString comment = "";
    //int row = tracerList.value(copyOfMappedValue, -1);
    if(tracerList.contains(copyOfMappedValue))
    {
        comment = ui->tableWidget->item(x.indexOf(copyOfMappedValue), NOTES_COLUMN)->text();
//        if(tracerList[copyOfMappedValue] != name)
//        {
//            comment += tr(" \n%1: ").arg(name);
//        }
    }
//    else
//    {
//        comment = tr("%1: ").arg(name);
//    }
    emit hasComments(comment);
}

// finds and returns the closest actual data point to where the curser is
double MainWindow::dataPointMap(double point)
{
    double position = 0.0;
    if(ui->plotView->selectedGraphs().count() > 0)
    {
        const QCPDataMap *dataMap = ui->plotView->selectedGraphs().first()->data();
        QMap<double, QCPData>::const_iterator i = dataMap->lowerBound(point);
        if((i.key() - point) < (point - (i-1).key()))
        {
            position = i.key();
        }
        else
        {
            position = (i-1).key();
        }
    }
    return position;
}

void MainWindow::commentSubmitted(QString comment)
{
    this->addCommentTracer(ui->plotView->selectedGraphs().first(), copyOfMappedValue);
    ui->plotView->replot();
    publishComments(comment, copyOfMappedValue);

    // filling the properties of tracer
    CommentTracer tracer;
    tracer.index = this->nextTracerIndex;
    tracer.name = ui->plotView->selectedGraphs().first()->name();
    tracer.position = copyOfMappedValue;
    tracer.comment = comment;
    commentTracerList.append(tracer);

    // saving the tracer object to db
    this->saveCommentTracerDB(&tracer);

    // make the tracerCheckBox active
    tracerCheckBox->setEnabled(true);

//    commentWindow->deleteLater();

//    emit finished();
//    connect(this, SIGNAL(finished()), commentWindow, SLOT(deleteLater()));

}

void MainWindow::publishComments(QString comment, double position)
{
    int index = x.indexOf(position);

    QSettings commentFile("../comments.ini", QSettings::IniFormat);
    commentFile.setValue(QString::number(index), comment );

    QTableWidgetItem * item = new QTableWidgetItem(tr("%1").arg(comment));
    ui->tableWidget->setItem(index, NOTES_COLUMN,item);
    ui->tableWidget->item(index, NOTES_COLUMN)->setToolTip(comment);
}

// finds the closest comment to the point
int MainWindow::findCommentTracer(double point)
{
    if(!tracerList.contains(point))
    {
        return -1;
    }
    int index = x.indexOf(point);
//    QMap<double, QString>::const_iterator i = tracerList.constBegin();
//    while (i != tracerList.constEnd()) {
//        qDebug() << i.key() << ": " << i.value() << endl;
//         ++i;
//    }
    return index;
}

// removes the comment tracer and comment when the deleted comment is okayed
void MainWindow::commentDeleted()
{
    this->removeCommentTracer(mousePosition);
    this->removeComment(copyOfMappedValue);
}

// redraws the comment tracer after canceling edit/delete
void MainWindow::cancelled()
{
    this->addCommentTracer(ui->plotView->selectedGraphs().first(), copyOfMappedValue);
    ui->plotView->replot();
}

bool MainWindow::saveCommentTracerDB(CommentTracer * commentTracer)
{
    qDebug() << "saveCommentTracerDB()";

    int dbIndex = commentTracer->index;
    int dbSize = commentTracerList.count();
    // allocate a new index at the end of the db if needed
    if (dbIndex == 0)
    {
        dbSize++;
    }

    QSettings commentTracerDB("../commentTracerDB.ini", QSettings::IniFormat);

    commentTracerDB.beginWriteArray( "commentTracers"/*, dbSize */);
        commentTracerDB.setArrayIndex( dbIndex );
        commentTracerDB.setValue ( TRACER_INDEX,  commentTracer->index );
        commentTracerDB.setValue ( PLOT_NAME,     commentTracer->name );
        commentTracerDB.setValue ( PLOT_POSITION, commentTracer->position );
        commentTracerDB.setValue ( PLOT_COMMENT,  commentTracer->comment);
    commentTracerDB.endArray();

    this->nextTracerIndex++;

    return true;
}

void MainWindow::loadCommentTracerDB()
{
    qDebug() << "loadCommentTracerDB()";
    this->commentTracerList.clear();

        QSettings commentTracerDB("../commentTracerDB.ini", QSettings::IniFormat);
        int numCommentTracers = commentTracerDB.beginReadArray("commentTracers");
        for (int i = 0; i < numCommentTracers; ++i)
        {
            commentTracerDB.setArrayIndex(i);
            // load into commentTracer structure
            CommentTracer commentTracer;
            commentTracer.index    = commentTracerDB.value(TRACER_INDEX   ,    i ).toInt();
            commentTracer.name     = commentTracerDB.value(PLOT_NAME      ,   "" ).toString();
            commentTracer.position = commentTracerDB.value(PLOT_POSITION  ,  0.0 ).toDouble();
            commentTracer.comment  = commentTracerDB.value(PLOT_COMMENT   ,   "" ).toString();
            this->commentTracerList.append(commentTracer);
        }
        commentTracerDB.endArray();
        this->nextTracerIndex = numCommentTracers;
}

void MainWindow::hideCommentTracers(QCPGraph * selectedGraph)
{
    ItemTracerList itemTracerPtrList;
    QString name = selectedGraph->name();

    if (name == "O2")
    {
        itemTracerPtrList = O2ItemTracerPtrList;
//        o2TracersShown = false;
    }
    else if (name == "RE")
    {
        itemTracerPtrList = reItemTracerPtrList;
 //       reTracersShown = false;
    }
    else return;

    if (itemTracerPtrList.isEmpty()) return;
    for(int i = 0; i < itemTracerPtrList.count(); i++)
    {


        qDebug() << "itemTracerPtrList[i]->graph()->name()" << itemTracerPtrList[i]->graph()->name();
        qDebug() << "itemTracerPtr count " << itemTracerPtrList.count();
        ui->plotView->removeItem(itemTracerPtrList[i]);
        qDebug() << "itemTracerPtrList.count() within the loop" << itemTracerPtrList.count();
    }
    qDebug() << "itemTracerPtrList.count()" << itemTracerPtrList.count();
    ui->plotView->replot();
    itemTracerPtrList.clear();
    if (name == "O2") O2ItemTracerPtrList = itemTracerPtrList;
    else reItemTracerPtrList =  itemTracerPtrList;

    qDebug() << "itemTracerPtrList.count() end of the loop" << itemTracerPtrList.count();

}

void MainWindow::showCommentTracers(QCPGraph * selectedGraph)
{
    QString name = selectedGraph->name();
    qDebug() << name;
    if (name != "RE" && name != "O2")
    {
        qDebug() << "returning";
        return;
    }
    for(int i = 0; i < commentTracerList.count(); i++)
    {
        qDebug() << "name is: " << commentTracerList[i].name;
        if(commentTracerList[i].name != name) continue;
        qDebug() << "adding" << commentTracerList[i].index;
        this->addCommentTracer(selectedGraph, commentTracerList[i].position);
        ui->plotView->replot();
    }
}

void MainWindow::on_hideCommentTracer_toggled(bool checked)
{
    if(checked)
    {
        qDebug() << "hello";
        this->hideCommentTracers(ui->plotView->selectedGraphs().first());
    }
    else
    {

        qDebug() << "bye";
        this->showCommentTracers(ui->plotView->selectedGraphs().first());
    }
}

void MainWindow::toggleTracersCheckBox()
{
    if(ui->plotView->selectedGraphs().first()->name() == "O2" && o2TracersShown) tracerCheckBox->setChecked(true);
    else tracerCheckBox->setChecked(false);
    if(ui->plotView->selectedGraphs().first()->name() == "RE" && reTracersShown) tracerCheckBox->setChecked(true);
    else tracerCheckBox->setChecked(false);
}

// rescale plots
void MainWindow::on_actionRescale_triggered()
{
    ui->plotView->rescaleAxes();
    ui->plotView->replot();
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
