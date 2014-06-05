#ifndef FILESYSTEMDIALOG_H
#define FILESYSTEMDIALOG_H
#include <QFileSystemModel>
#include <QItemSelection>

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
    
private slots:
    void on_openButton_clicked();
    void updateFilePathLine(const QItemSelection &, const QItemSelection &);


private:
    Ui::FileSystemDialog *ui;
    QFileSystemModel * fileSystemModel;
};

#endif // FILESYSTEMDIALOG_H
