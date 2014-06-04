#include "FileSystemDialog.h"
#include "ui_FileSystemDialog.h"
#include <QFileSystemModel>

FileSystemDialog::FileSystemDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileSystemDialog)
{
    ui->setupUi(this);
    fileSystemModel = new QFileSystemModel(this);
    fileSystemModel->setHeaderData(1,Qt::Horizontal,Qt::EditRole);
    ui->fileSystemView->clearSelection();


    fileSystemModel->setRootPath("C:\\");
    fileSystemModel->setFilter(QDir::Dirs|QDir::Drives|QDir::NoDotAndDotDot|QDir::AllDirs);

    ui->fileSystemView->setModel(fileSystemModel);

    QModelIndex index = fileSystemModel->index("C:\\");
    ui->fileSystemView->setItemsExpandable(false);
    ui->fileSystemView->header()->setMovable(false);
//    ui->treeView->header()->resizeSections(QHeaderView::ResizeToContents);
    ui->fileSystemView->hideColumn(1);
    ui->fileSystemView->hideColumn(2);
    ui->fileSystemView->setColumnWidth(0,350);
    ui->fileSystemView->setColumnWidth(3,250);
    ui->fileSystemView->setRootIndex(index);

  // ui->fileSystemView->setSelectionMode(QAbstractItemView::MultiSelection);
//    QItemSelectionModel * selection = ui->treeView->selectedIndexes();
//    QModelIndexList  selected = ui->treeView->selectionModel()->selectedIndexes();
//    qDebug() <<  selected;

}

FileSystemDialog::~FileSystemDialog()
{
    delete ui;
}
