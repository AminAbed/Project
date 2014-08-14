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
    
private:
    Ui::CommentWindow *ui;
};

#endif // COMMENTWINDOW_H
