#include "CustomLineEdit.h"
#include "MainWindow.h"

#include <QLineEdit>
#include <QMimeData>
#include <QUrl>

CustomLineEdit::CustomLineEdit(QWidget *parent) :
    QLineEdit(parent)
{
    setAcceptDrops(true);
}

void CustomLineEdit::dragEnterEvent(QDragEnterEvent * event)
{
    setDropFlag(false);
    event->accept();
}

void CustomLineEdit::dragLeaveEvent(QDragLeaveEvent * event)
{
    event->accept();
}

void CustomLineEdit::dragMoveEvent(QDragMoveEvent * event)
{
    event->accept();
}

void CustomLineEdit::dropEvent(QDropEvent * event)
{   
    // reimplement drop event
    event->accept();
    QList <QUrl> pathList = event->mimeData()->urls();
    path = pathList.at(0).toString();
    path.remove(0,8);    // string formatting
    qDebug() << path;
    setText(path);
    setDropFlag(true);
    emit dropped();
 }

void CustomLineEdit::setText(const QString & path)
{
    // reimplement setText function
    QLineEdit::setText(path);
}

QString CustomLineEdit::getFilePath()
{
    return path;
}

bool CustomLineEdit::getDropFlag()
{
    return dropFlag;
}

void CustomLineEdit::setDropFlag(bool drop)
{
    dropFlag = drop;
}
