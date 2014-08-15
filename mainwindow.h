#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QItemSelection>
#include <CustomLineEdit.h>
#include <QTableWidget>
#include <QtWebKit/QWebView>
#include "QCustomPlot.h"
#include "SettingsPage.h"
#include "CommentWindow.h"


typedef QList <double> DoublesList;
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void setupFileSystemView();
    void populateTable();
    void setupGraph();
    bool removeGraphByAction(QAction * );
    void toggleMenu();
    void restoreWindowState();
    void saveWindowState();
    void addCommentTracer(QCPGraph * , double );
    void setupCommentWindow();
    double dataPointMap(double );
    void publishComments(QString ,double );
    int findCommentTracer(double );

private:
    Ui::MainWindow *ui;
    QFileSystemModel * fileSystemModel;
    
    enum summaryParameters 
    {     
        respiratoryEnthalpy,
        eRespiratoryEnthalpy,
        tVolume,
        mVolume,
        BPM,
        minTemp,
        maxTemp,
        minRH,
        maxRH,
        inhaledEnthalpy,
        O2Consumption,
        minO2,
        maxO2,
        indexNumber,
        total
    };

    DoublesList readings[total];
    QStringList timeStamp;

    // universal x-axis (timestamps)
    QVector<double> x;
    QList<int> indexes;
    QMap<double, int> tracerList;

    double mappedXAxisPosition;
    bool isCursorCloseToGraph;

    // context menu actions
    QAction* actionREAdd;
    QAction* actionEREAdd;
    QAction* actionO2Add;
    QAction* actionMinTempAdd;
    QAction* actionMaxTempAdd;
    QAction* actionMinRHAdd;
    QAction* actionMaxRHAdd;

    QAction* actionRERemove;
    QAction* actionERERemove;
    QAction* actionO2Remove;
    QAction* actionMinTempRemove;
    QAction* actionMaxTempRemove;
    QAction* actionMinRHRemove;
    QAction* actionMaxRHRemove;
    bool RE, ERE, O2, MinTemp, MaxTemp, MinRH, MaxRH;

    QAction * actionAddComment;
    QAction * actionRemoveComment;

    SettingsPage  settingsPage;
    QString summaryFileName;
    QString filePath;

    // pointer to comment window
    CommentWindow * commentWindow;


protected:
    bool eventFilter(QObject *, QEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);


public slots:
    void on_actionOpen_triggered();
    void on_actionSettings_triggered();
    void on_actionOpenAnotherWindow_triggered();
    void on_actionPlotPDF_triggered();
    void on_actionTablePDF_triggered();
    void on_openButton_clicked();
    void updatePatientInfoBox();
    int readSession(QString );
    void checkFilePathLine();
    void selectionChanged();
    void menuRequest(QPoint );
    void plot(int , Qt::GlobalColor , QString );
    void plot();
    void removeSelectedGraph();
    void removeAllGraphs();
    bool removeGraphByName(QString );
    void actionMapper(QAction * );
    void actionMapper(QString, bool );

    QString generateHTML();
    void saveHTMLToPDF(QWebView * );

    // slots to limit x/y-axis
    void xAxisLimit(QCPRange newRange);
    void yAxisLimit(QCPRange newRange);

    void horzScrollBarChanged(int value);
    void vertScrollBarChanged(int value);
    void xAxisChanged(QCPRange range);
    void yAxisChanged(QCPRange range);

    void mousePress();
    void mouseWheel();

private slots:
    void updateFilePathLine(const QItemSelection &, const QItemSelection &);
    void on_cancelButton_clicked();
    void commentSubmitted(QString );

signals:
    void toggleCheckBox(QString ,bool );
};

#endif // MAINWINDOW_H
