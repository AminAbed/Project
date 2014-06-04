#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <FileSystemDialog.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void openSession();

private:
    Ui::MainWindow *ui;
    FileSystemDialog * fileSystemDialog;


public slots:
    void on_actionOpen_triggered();

};

#endif // MAINWINDOW_H
