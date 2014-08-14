#include "CommentWindow.h"
#include "ui_CommentWindow.h"

CommentWindow::CommentWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CommentWindow)
{
    ui->setupUi(this);
}

CommentWindow::~CommentWindow()
{
    delete ui;
}
