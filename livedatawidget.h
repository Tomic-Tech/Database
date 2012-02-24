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
    void insertNewItem(const QString &catalog, QSqlDatabase &db);
    void deleteItem(const QString &catalog, QSqlDatabase &db);
    QStringList catalogList();
public slots:
    void setCurrentCatalog(const QString &catalog);
private:
    void adjustItems(QSqlDatabase &db);
    QHash<QString, QStringList> queryShortNames(QSqlDatabase &db);
    QHash<QString, QStringList> queryCatalogs(QSqlDatabase &db);
    bool checkShortNameAndCatalog(const QStringList &checkedShortName,
        const QStringList &checkedCatalog,
        const QString &shortName,
        const QString &catalog);
    void insertNewItem(const QString shortName, 
        const QString &catalog, 
        const QString &lang,
        QSqlDatabase &db);
    void fixItems(const QHash<QString, QStringList> &shortNameHash,
        const QHash<QString, QStringList> &catalogHash,
        QSqlDatabase &db);
    void updateShortNameAndCatalog(QSqlDatabase &db);
private slots:
    void setCurrentShortName(const QModelIndex &index);
    void updateContent();
    void updateUnit(const QString &text);
    void updateCommboxID(const QString &text);
    void updateAlgorithmID(const QString &text);
    void updateDefaultValue(const QString &text);
    void changeAddNew();
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
    QStringList _catalogList;
    QStringList _shortNameList;
    QString _currentCatalog;
    bool _isAddNew;
};

#endif