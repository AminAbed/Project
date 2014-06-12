#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QItemSelection>
#include <CustomLineEdit.h>
#include "QCustomPlot.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void plot(QCustomPlot *customPlot);

private:
    Ui::MainWindow *ui;
    QFileSystemModel * fileSystemModel;

protected:
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);


public slots:
    void on_actionOpen_triggered();
    void on_openButton_clicked();
    void updatePatientInfoBox();
//    void readSession();
    void checkFilePathLine();

private slots:
    void updateFilePathLine(const QItemSelection &, const QItemSelection &);
    void on_cancelButton_clicked();
};

#endif // MAINWINDOW_H
