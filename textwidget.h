#ifndef TEXT_WIDGET_H
#define TEXT_WIDGET_H

#include <QtGui>
#include <QWidget>
#include <QDataWidgetMapper>
#include <QSqlTableModel>
#include <QtSql>

class TextWidget : public QWidget
{
    Q_OBJECT
public:
    TextWidget(QStringList &langList, QWidget *parent = 0);
    ~TextWidget();
    void setDB(QSqlDatabase &db);
    void createTable(QSqlDatabase &db);
    void insertNewName(QSqlDatabase &db);
    void deleteItem(QSqlDatabase &db);
private:
    void adjustItems(QSqlDatabase &db);
    QHash<QString, QStringList> queryNames(QSqlDatabase &db);
    void fixNames(const QHash<QString, QStringList> &nameHash, QSqlDatabase &db);
    bool checkName(const QStringList &beChecked, const QString &key);
    void insertNewName(const QString &key, const QString &lang, QSqlDatabase &db);
    void updateCurrentNameList(QSqlDatabase &db);
private slots:
    void setCurrentName(const QModelIndex &index);
    void updateContent();
    void changeAddNew();
private:
    QStringList &_langList;
    QSqlDatabase _db;
    QLineEdit *_nameEdit;
    QHash<QString, QPlainTextEdit*> _textEdits;
    QListView *_nameListView;
    QStringListModel *_nameListModel;
    QStringList _nameList;
    bool _isAddNew;
};

#endif