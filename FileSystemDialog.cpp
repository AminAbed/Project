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
    fileSystemModel->setFilter(QDir::Dirs|QDir::Drives|QDir::NoDotAndDotDot|QDir::AllDirs);

    ui->fileSystemView->setModel(fileSystemModel);

    QModelIndex index = fileSystemModel->index("C:/");
    //ui->fileSystemView->setItemsExpandable(false);
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

void FileSystemDialog::on_openButton_clicked()
{
    QModelIndexList  selected = ui->fileSystemView->selectionModel()->selectedIndexes();
    QStringList temp;
    foreach(QModelIndex current, selected)
    {
        // `temp` contains the text in one row
        QVariant data = fileSystemModel->data(current, Qt::DisplayRole);
        QString path = fileSystemModel->filePath(current);
        qDebug() << "data:" << data;
        qDebug() << path;
        temp.append(data.toString());
    }
    QStringList selectedPatients = temp.filter("pt");
    qDebug() << selectedPatients;

}
