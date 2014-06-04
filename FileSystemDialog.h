#ifndef FILESYSTEMDIALOG_H
#define FILESYSTEMDIALOG_H
#include <QFileSystemModel>

#include <QWidget>

namespace Ui {
class FileSystemDialog;
}

class FileSystemDialog : public QWidget
{
    Q_OBJECT
    
public:
    explicit FileSystemDialog(QWidget *parent = 0);
    ~FileSystemDialog();
    
private:
    Ui::FileSystemDialog *ui;
    QFileSystemModel * fileSystemModel;
};

#endif // FILESYSTEMDIALOG_H
