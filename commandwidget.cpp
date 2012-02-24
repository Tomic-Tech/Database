#include "commandwidget.h"

CommandWidget::CommandWidget(QWidget *parent /* = 0 */)
    : QWidget(parent)
{

}

CommandWidget::~CommandWidget()
{

}

void CommandWidget::createTable(QSqlDatabase &db)
{
    QString text = QString("CREATE TABLE [Command] (ID int primary key, Name varchar(20), Command blob, Catalog varchar(20));");
    QSqlQuery query(db);
    query.exec(text);
    QString error = query.lastError().databaseText();
}

void CommandWidget::setDB(QSqlDatabase &db)
{

}

