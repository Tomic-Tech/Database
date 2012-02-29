#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , _ui()
    , _langList()
    , _db(QSqlDatabase::addDatabase(QString::fromLocal8Bit("JMSQLITE")))
    , _catalogList()
    , _text(0)
    , _troubleCode(0)
    , _liveData(0)
    , _command(0)
{
    _ui.setupUi(this);

    _langList.append(QString::fromUtf8("zh-CN"));
    _langList.append(QString::fromUtf8("en-US"));

    QByteArray password;

    password.append('2');
    password.append('0');
    password.append('4');
    password.append('3');
    password.append('E');
    password.append('D');
    password.append('8');
    password.append('E');
    password.append('-');
    password.append('4');
    password.append('E');
    password.append('3');
    password.append('5');
    password.append('-');
    password.append('4');
    password.append('B');
    password.append('F');
    password.append('A');
    password.append('-');
    password.append('A');
    password.append('C');
    password.append('2');
    password.append('9');
    password.append('-');
    password.append('1');
    password.append('7');
    password.append('5');
    password.append('6');
    password.append('E');
    password.append('5');
    password.append('5');
    password.append('3');
    password.append('4');
    password.append('0');
    password.append('5');
    password.append('D');

    _db.setPassword(password);

    _ui.catalogList->setModel(&_catalogList);

    _text = new TextWidget(_langList, _ui.tabWidget);
    _ui.tabWidget->addTab(_text, trUtf8("Text"));

    _troubleCode = new TroubleCodeWidget(_langList, _ui.tabWidget);
    _ui.tabWidget->addTab(_troubleCode, trUtf8("Trouble Code"));

    _liveData = new LiveDataWidget(_langList, _ui.tabWidget);
    _ui.tabWidget->addTab(_liveData, trUtf8("Live Data"));

    _command = new CommandWidget(_ui.tabWidget);
    _ui.tabWidget->addTab(_command, trUtf8("Command"));

    connect(_ui.action_New, SIGNAL(triggered()), this, SLOT(newDB()));
    connect(_ui.action_Open, SIGNAL(triggered()), this, SLOT(openDB()));
    connect(_ui.pushButtonInsert, SIGNAL(clicked()), this, SLOT(insertItem()));
    connect(_ui.pushButtonSubmit, SIGNAL(clicked()), this, SLOT(submit()));
    connect(_ui.pushButtonDelete, SIGNAL(clicked()), this, SLOT(deleteItem()));
    connect(_ui.pushButtonDeleteAll, SIGNAL(clicked()), this, SLOT(deleteAll()));
    connect(_ui.pushButtonAddCatalog, SIGNAL(clicked()), this, SLOT(addCatalog()));
    connect(_ui.pushButtonModifyCatalog, SIGNAL(clicked()), this, SLOT(modifyCatalog()));
    connect(_ui.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(changePage(int)));
    connect(_ui.catalogList, SIGNAL(clicked(QModelIndex)), this, SLOT(catalogChange(QModelIndex)));
}

MainWindow::~MainWindow()
{

}

void MainWindow::closeDB()
{
    deleteModels();
    disableAllButtons();
    _db.close();
    _catalogList.removeRows(0, _catalogList.rowCount());
}

void MainWindow::newDB()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("New Database"), QDir::currentPath(), tr("Database Files (*.db)"));

    if (fileName.isEmpty())
    {
        return;
    }

    closeDB();
    emptyCatalogLists();

    _db.setDatabaseName(fileName);
    if (!_db.open())
    {
        return;
    }

    createTables();
    newModels();
    enableAllButtons();
}

void MainWindow::openDB()
{
    QString fileName = QFileDialog::getOpenFileName(this, trUtf8("Open Database"), QDir::currentPath(), trUtf8("Database Files (*.db)"));

    if (fileName.isEmpty())
    {
        return;
    }

    closeDB();
    emptyCatalogLists();

    _db.setDatabaseName(fileName);
    if (!_db.open())
    {
        return;
    }

    createTables();
    newModels();
    enableAllButtons();
}

void MainWindow::createTables()
{
    _text->createTable(_db);
    _troubleCode->createTable(_db);
    _liveData->createTable(_db);
    _command->createTable(_db);
}

void MainWindow::enableAllButtons()
{
    _ui.pushButtonDelete->setEnabled(true);
    _ui.pushButtonInsert->setEnabled(true);
    _ui.pushButtonRevert->setEnabled(true);
    _ui.pushButtonSubmit->setEnabled(true);
    _ui.pushButtonDeleteAll->setEnabled(true);
}

void MainWindow::disableAllButtons()
{
    _ui.pushButtonDelete->setEnabled(false);
    _ui.pushButtonInsert->setEnabled(false);
    _ui.pushButtonRevert->setEnabled(false);
    _ui.pushButtonSubmit->setEnabled(false);
    _ui.pushButtonDeleteAll->setEnabled(false);
}

void MainWindow::emptyCatalogLists()
{
    
}

void MainWindow::newModels()
{
    _text->setDB(_db);
    _troubleCode->setDB(_db);
    _liveData->setDB(_db);
    _command->setDB(_db);
}

void MainWindow::deleteModels()
{

}

void MainWindow::insertItem()
{
    if (_ui.tabWidget->currentWidget() == _text)
    {
        _text->insertNewName(_db);
    }
    else if (_ui.tabWidget->currentWidget() == _troubleCode)
    {
        _troubleCode->insertNewItem(_ui.catalogList->currentIndex().data().toString(), _db);
    }
    else if (_ui.tabWidget->currentWidget() == _liveData)
    {
        _liveData->insertNewItem(_ui.catalogList->currentIndex().data().toString(), _db);
    }
    else if (_ui.tabWidget->currentWidget() == _command)
    {
        _command->insertNewItem(_ui.catalogList->currentIndex().data().toString(), _db);
    }
}

void MainWindow::submit()
{
    _db.transaction();
    _db.commit();
}

void MainWindow::deleteItem()
{
    if (_ui.tabWidget->currentWidget() == _text)
    {
        _text->deleteItem(_db);
    }
    else if (_ui.tabWidget->currentWidget() == _troubleCode)
    {
        _troubleCode->deleteItem(_ui.catalogList->currentIndex().data().toString(), _db);
    }
    else if (_ui.tabWidget->currentWidget() == _liveData)
    {
        _liveData->deleteItem(_ui.catalogList->currentIndex().data().toString(), _db);
    }
    else if (_ui.tabWidget->currentWidget() == _command)
    {
        _command->deleteItem(_ui.catalogList->currentIndex().data().toString(), _db);
    }
}

void MainWindow::deleteAll()
{
    if (_ui.tabWidget->currentWidget() == _command)
    {
        _command->deleteAllItems(_db);
    }

    _catalogList.setStringList(_command->catalogList());
}

void MainWindow::addCatalog()
{
    QString catalog = _ui.lineEditAddSystem->text();
    if (_ui.tabWidget->currentWidget() == _troubleCode)
    {
        _troubleCode->setCurrentCatalog(catalog);
        _catalogList.setStringList(_troubleCode->catalogList());
    }
    else if (_ui.tabWidget->currentWidget() == _liveData)
    {
        _liveData->setCurrentCatalog(catalog);
        _catalogList.setStringList(_liveData->catalogList());
    }
    else if (_ui.tabWidget->currentWidget() == _command)
    {
        _command->setCurrentCatalog(catalog);
        _catalogList.setStringList(_command->catalogList());
    }

    _ui.catalogList->setCurrentIndex(_catalogList.index(_catalogList.rowCount() - 1));
}

void MainWindow::changePage(int index)
{
    if (_ui.tabWidget->currentWidget() == _troubleCode)
    {
        _catalogList.setStringList(_troubleCode->catalogList());
        _troubleCode->setCurrentCatalog(_catalogList.index(0).data().toString());
    }
    else if (_ui.tabWidget->currentWidget() == _liveData)
    {
        _catalogList.setStringList(_liveData->catalogList());
        _liveData->setCurrentCatalog(_catalogList.index(0).data().toString());
    }
    else if (_ui.tabWidget->currentWidget() == _command)
    {
        _catalogList.setStringList(_command->catalogList());
        _command->setCurrentCatalog(_catalogList.index(0).data().toString());
    }
    else
    {
        _catalogList.setStringList(QStringList());
    }

    _ui.catalogList->setCurrentIndex(_catalogList.index(0));
}

void MainWindow::catalogChange(QModelIndex index)
{
    QString catalog = index.data().toString();
    if (_ui.tabWidget->currentWidget() == _troubleCode)
    {
        _troubleCode->setCurrentCatalog(catalog);
    }
    else if (_ui.tabWidget->currentWidget() == _liveData)
    {
        _liveData->setCurrentCatalog(catalog);
    }
    else if (_ui.tabWidget->currentWidget() == _command)
    {
        _command->setCurrentCatalog(catalog);
    }
}

void MainWindow::modifyCatalog()
{
    bool ok;
    QString text = QInputDialog::getText(this, trUtf8("Modify Current Catalog"), trUtf8("Catalog"), QLineEdit::Normal, _ui.catalogList->currentIndex().data().toString(), &ok);
    if (ok && !text.isEmpty())
    {
        if (_ui.tabWidget->currentWidget() == _troubleCode)
        {
            _troubleCode->modifyCatalog(_ui.catalogList->currentIndex().data().toString(), text, _db);
            _catalogList.setStringList(_troubleCode->catalogList());
        }
        else if (_ui.tabWidget->currentWidget() == _liveData)
        {
            _liveData->modifyCatalog(_ui.catalogList->currentIndex().data().toString(), text, _db);
            _catalogList.setStringList(_liveData->catalogList());
        }
    }
}