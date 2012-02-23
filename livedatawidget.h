#ifndef LIVE_DATA_WIDGET_H
#define LIVE_DATA_WIDGET_H

#include <QtGui>
#include <QtSql>

class LiveDataWidget : public QWidget
{
    Q_OBJECT
public:
    LiveDataWidget(QStringList &langList, QWidget *parent = 0);
    ~LiveDataWidget();
    void setDB(QSqlDatabase &db);
    void createTable(QSqlDatabase &db);
private:
    QStringList &_langList;
    QSqlDatabase _db;
    QListView *_shortNameListView;
    QStringListModel *_shortNameListModel;
    QLineEdit *_shortNameEdit;
    QLineEdit *_unitEdit;
    QComboBox *_commandIDBox;
    QComboBox *_algorithmIDBox;
    QHash<QString, QPlainTextEdit*> _contentEdits;
    QHash<QString, QLineEdit *> _defEdits;
};

#endif