#include "CommentWindow.h"
#include "ui_CommentWindow.h"
#include <QDebug>

#define WIN_WIDTH 250
#define WIN_HEIGHT 122

CommentWindow::CommentWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CommentWindow)
{
    ui->setupUi(this);
    this->resize(WIN_WIDTH, WIN_HEIGHT);
}

CommentWindow::~CommentWindow()
{
    delete ui;
}

void CommentWindow::on_submitButton_clicked()
{
    QString comment = ui->commentSection->toPlainText();
    emit commentSubmitted(comment);
    ui->commentSection->clear();
    this->close();
}

void CommentWindow::on_cancelButton_clicked()
{
    ui->commentSection->clear();
    this->close();
}
