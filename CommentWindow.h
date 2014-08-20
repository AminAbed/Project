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
    static CommentWindow & getInstance();

private slots:
    void on_okButton_clicked();
    void on_cancelButton_clicked();
    void toggleOkButton();
    void setCommentText(QString );

private:
    Ui::CommentWindow *ui;
    /// Manages the singleton instance

signals:
    void commentSubmitted(QString );
    void commentDeleted();
    void cancelled();
};

#endif // COMMENTWINDOW_H
