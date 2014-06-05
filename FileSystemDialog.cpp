#include "FileSystemDialog.h"
#include "ui_FileSystemDialog.h"
#include <QFileSystemModel>
#include <QDebug>

FileSystemDialog::FileSystemDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileSystemDialog)
{
    ui->setupUi(this);
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

FileSystemDialog::~FileSystemDialog()
{
    delete ui;
}

void FileSystemDialog::on_openButton_clicked()
{
    QString filePath = ui->filePathline->text();
    qDebug() << filePath;
}

void FileSystemDialog::updateFilePathLine(const QItemSelection & , const QItemSelection & )
{
    QString path;
    QModelIndexList  selected = ui->fileSystemView->selectionModel()->selectedIndexes();
    foreach(QModelIndex current, selected)
    {
        path = fileSystemModel->filePath(current);
    }
    ui->filePathline->setText(path);
}


