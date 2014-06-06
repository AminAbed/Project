#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QItemSelection>
#include <CustomLineEdit.h>


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
    QFileSystemModel * fileSystemModel;
    CustomLineEdit * customLineEdit;

protected:
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);


public slots:
    void on_actionOpen_triggered();
    void on_openButton_clicked();
//    void readSession();

private slots:
    void updateFilePathLine(const QItemSelection &, const QItemSelection &);


};

#endif // MAINWINDOW_H
