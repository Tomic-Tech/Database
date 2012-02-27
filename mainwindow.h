#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include <QMainWindow>
#include <QtSql>
#include <QTableView>
#include "ui_mainwindow.h"
#include "textwidget.h"
#include "troublecodewidget.h"
#include "livedatawidget.h"
#include "commandwidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private:
    void createTables();
    void newModels();
    void deleteModels();
    void enableAllButtons();
    void disableAllButtons();
    void emptyCatalogLists();
private slots:
    void newDB();
    void openDB();
    void closeDB();
    void insertItem();
    void submit();
    void deleteItem();
    void addCatalog();
    void changePage(int index);
    void catalogChange(QModelIndex index);
private:
    Ui::MainWindow _ui;
    QStringList _langList;
    QSqlDatabase _db;
    QStringListModel _catalogList;
    TextWidget *_text;
    TroubleCodeWidget *_troubleCode;
    LiveDataWidget *_liveData;
    CommandWidget *_command;
};

#endif