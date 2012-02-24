#ifndef COMMAND_WIDGET_H
#define COMMAND_WIDGET_H

#include <QtGui>
#include <QtSql>

class CommandWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CommandWidget(QWidget *parent = 0);
    ~CommandWidget();
    void createTable(QSqlDatabase &db);
    void setDB(QSqlDatabase &db);
};

#endif