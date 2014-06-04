#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //
    ui->setupUi(this);
    ui->mainToolBar->setMinimumHeight(32);
    ui->mainToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->lineEdit->setAcceptDrops(true);

    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(on_actionOpen_triggered()));

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
    qDebug() << "triggerd";
    fileSystemDialog = new FileSystemDialog(this);
    //fileSystemDialog->setGeometry(110,100,250,280);
    //fileSystemDialog->setWindowFlags(Qt::CustomizeWindowHint);
    fileSystemDialog->show();
}
