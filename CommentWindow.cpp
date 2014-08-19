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
    ui->okButton->setEnabled(false);
    connect(ui->commentSection, SIGNAL(textChanged()), this, SLOT(toggleOkButton()));
}

CommentWindow::~CommentWindow()
{
    delete ui;
}

void CommentWindow::on_okButton_clicked()
{
    QString comment = ui->commentSection->toPlainText();
    emit commentSubmitted(comment);
    ui->commentSection->clear();
    ui->okButton->setEnabled(false);
    this->close();
}

void CommentWindow::on_cancelButton_clicked()
{
    ui->commentSection->clear();
    this->close();
}

void CommentWindow::toggleOkButton()
{
    if(!ui->commentSection->toPlainText().isEmpty())
    {
        ui->okButton->setEnabled(true);
    }
}

void CommentWindow::setCommentText(QString comment)
{
    ui->commentSection->setPlainText(comment);
}

CommentWindow &CommentWindow::getInstance()
{
    static CommentWindow instance;

    return instance;
}
