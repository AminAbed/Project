#ifndef COMMENTWINDOW_H
#define COMMENTWINDOW_H

#include <QDialog>

namespace Ui {
class CommentWindow;
}

class CommentWindow : public QDialog
{
    Q_OBJECT
    
public:
    explicit CommentWindow(QWidget *parent = 0);
    ~CommentWindow();

private slots:
    void on_submitButton_clicked();
    void on_cancelButton_clicked();

private:
    Ui::CommentWindow *ui;

signals:
    void commentSubmitted(QString );
};

#endif // COMMENTWINDOW_H
