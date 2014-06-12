#ifndef CUSTOMLINEEDIT_H
#define CUSTOMLINEEDIT_H

#include <QObject>
#include <QLineEdit>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QDebug>

class CustomLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit CustomLineEdit(QWidget *parent = 0);
    
    void dragEnterEvent(QDragEnterEvent *);
    void dragLeaveEvent(QDragLeaveEvent *);
    void dragMoveEvent(QDragMoveEvent *);
    void dropEvent(QDropEvent *);
    void setText(const QString &);
    QString getFilePath();
    bool getDropFlag();
    void setDropFlag(bool );

signals:
    void dropped();

public slots:

private:
    QString path;
    bool dropFlag;
    

};

#endif // CUSTOMLINEEDIT_H
