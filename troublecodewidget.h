#ifndef TROUBLE_CODE_WIDGET_H
#define TROUBLE_CODE_WIDGET_H

#include <QtGui>
#include <QtSql>

class TroubleCodeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TroubleCodeWidget(QStringList &langList, QWidget *parent = 0);
    ~TroubleCodeWidget();
    void setDB(QSqlDatabase &db);
    void createTable(QSqlDatabase &db);
    void insertNewItem(const QString &catalog, QSqlDatabase &db);
    void deleteItem(const QString &catalog, QSqlDatabase &db);
    QStringList catalogList();
public slots:
    void setCurrentCatalog(const QString &catalog);
private:
    void adjustItems(QSqlDatabase &db);
    QHash<QString, QStringList> queryCodes(QSqlDatabase &db);
    QHash<QString, QStringList> queryCatalogs(QSqlDatabase &db);
    void fixItems(const QHash<QString, QStringList> &codeHash, 
        const QHash<QString, QStringList> &catalogHash, 
        QSqlDatabase &db);
    bool checkCodeAndCatalog(const QStringList &checkedCode, 
        const QStringList &checkedCatalog, 
        const QString &code,
        const QString &catalog);
    void insertNewItem(const QString &code, const QString &catalog, const QString &lang, QSqlDatabase &db);
    void updateCodeAndCatalog(QSqlDatabase &db);
private slots:
    void setCurrentCode(const QModelIndex &index);
    void updateContent();
    void changeAddNew();
private:
    QStringList &_langList;
    QSqlDatabase _db;
    QLineEdit *_codeEdit;
    QHash<QString, QPlainTextEdit*> _textEdits;
    QListView *_codeListView;
    QStringListModel *_codeListModel;
    QStringList _codeList;
    QStringList _catalogList;
    QString _currentCatalog;
    bool _isAddNew;
};

#endif