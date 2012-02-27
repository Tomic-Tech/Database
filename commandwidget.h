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
    void insertNewItem(const QString &catalog, QSqlDatabase &db);
    void deleteItem(const QString &catalog, QSqlDatabase &db);
    QStringList catalogList();
public slots:
    void setCurrentCatalog(const QString &catalog);
private:
    void adjustItems(QSqlDatabase &db);
    void updateNameAndCatalog(QSqlDatabase &db);
    void insertNewItem(const QString &name, const QString &catalog, QSqlDatabase &db);
private slots:
    void setCurrentName(const QModelIndex &index);
    void updateCommand(const QString &text);
    void changeAddNew();
private:
    QSqlDatabase _db;
    QStringList _catalogList;
    bool _isAddNew;
    QListView *_nameListView;
    QStringListModel *_nameListModel;
    QStringList _nameList;
    QLineEdit *_idEdit;
    QLineEdit *_nameEdit;
    QLineEdit *_cmdEdit;
    QString _currentCatalog;
};

#endif