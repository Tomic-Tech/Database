#include "commandwidget.h"

CommandWidget::CommandWidget(QWidget *parent /* = 0 */)
    : QWidget(parent)
    , _db()
    , _catalogList()
    , _isAddNew(false)
    , _nameListView(0)
    , _nameListModel(0)
    , _nameList()
    , _nameEdit(0)
    , _cmdEdit(0)
    , _currentCatalog()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    setLayout(mainLayout);

    _nameListView = new QListView(this);
    _nameListView->setMaximumWidth(240);

    _nameListModel = new QStringListModel();
    _nameListView->setModel(_nameListModel);
    _nameListView->setEditTriggers(QListView::NoEditTriggers);
    _nameListView->setSelectionMode(QListView::SingleSelection);

    mainLayout->addWidget(_nameListView);

    QVBoxLayout *editLayout = new QVBoxLayout(this);

    mainLayout->addLayout(editLayout);

    QGroupBox *box = new QGroupBox(this);
    box->setTitle(trUtf8("ID"));
    _idEdit = new QLineEdit(box);
    _idEdit->setReadOnly(true);

    QHBoxLayout *layout = new QHBoxLayout(box);
    box->setLayout(layout);
    layout->addWidget(_idEdit);

    editLayout->addWidget(box);

    box = new QGroupBox(this);
    box->setTitle(trUtf8("Name"));
    box->setMaximumHeight(50);
    _nameEdit = new QLineEdit(box);
    
    layout = new QHBoxLayout(box);
    box->setLayout(layout);
    layout->addWidget(_nameEdit);

    editLayout->addWidget(box);

    box = new QGroupBox(this);
    box->setTitle(trUtf8("Command"));
    box->setMaximumHeight(50);
    _cmdEdit = new QLineEdit(box);
    _cmdEdit->setInputMask(QString("HH hh hh hh hh hh hh hh hh hh hh hh hh hh hh hh hh hh hh hh hh hh hh hh"));
    
    layout = new QHBoxLayout(box);
    box->setLayout(layout);
    layout->addWidget(_cmdEdit);

    editLayout->addWidget(box);

    QSpacerItem *spacerItem = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    editLayout->addSpacerItem(spacerItem);

    connect(_cmdEdit, SIGNAL(textChanged(QString)), this, SLOT(updateCommand(QString)));
    connect(_nameListView, SIGNAL(clicked(QModelIndex)), this, SLOT(setCurrentName(QModelIndex)));
    connect(_nameEdit, SIGNAL(textChanged(QString)), this, SLOT(changeAddNew()));
}

CommandWidget::~CommandWidget()
{
    delete _nameListModel;
}

void CommandWidget::createTable(QSqlDatabase &db)
{
    QString text = QString("CREATE TABLE [Command] (ID int primary key, Name varchar(20), Command blob, Catalog varchar(20));");
    QSqlQuery query(db);
    query.exec(text);
    QString error = query.lastError().databaseText();
}

void CommandWidget::updateNameAndCatalog(QSqlDatabase &db)
{
    _catalogList.clear();

    QSqlQuery query(db);
    query.prepare(QString("SELECT Catalog FROM [Command]"));
    query.exec();
    while (query.next())
    {
        QStringList::const_iterator it = qFind(_catalogList.begin(), _catalogList.end(), query.value(0).toString());
        if (it == _catalogList.end())
        {
            _catalogList.append(query.value(0).toString());
        }
    }
}

void CommandWidget::adjustItems(QSqlDatabase &db)
{
    updateNameAndCatalog(db);
}

void CommandWidget::setDB(QSqlDatabase &db)
{
    _db = db;
    adjustItems(db);
}

void CommandWidget::insertNewItem(const QString &name, const QString &catalog, QSqlDatabase &db)
{
    _isAddNew = false;
    QSqlQuery query(db);
    if (!query.prepare(QString("SELECT max(ID) FROM [Command]")))
        return;
    if (!query.exec())
        return;
    if (!query.next())
        return;
    int count = query.value(0).toInt() + 1;

    QString text = _cmdEdit->text();
    text.remove(" ");
    //QString insertText(QString("INSERT INTO [Command] VALUES(%1, '%2', %3, '%4')")
    //    .arg(count)
    //    .arg(name)
    //    .arg(text)
    //    .arg(catalog));
    _idEdit->setText(QString("%1").arg(count));

    if (!query.prepare("INSERT INTO [Command] VALUES(:id, :name, :command, :catalog)"))
    {
        QString str = query.lastError().databaseText();
        return;
    }
    QByteArray cmd;
    for (int i = 0; i < text.size(); i += 2)
    {
        cmd.append(static_cast<quint8>(text.mid(i, 2).toUShort(NULL, 16)));
    }

    query.bindValue(":id", count);
    query.bindValue(":name", name);
    query.bindValue(":command", cmd);
    query.bindValue(":catalog", catalog);

    if (!query.exec())
        return;
}

void CommandWidget::insertNewItem(const QString &catalog, QSqlDatabase &db)
{
    insertNewItem(_nameEdit->text(), catalog, db);
    _nameList.append(_nameEdit->text());
    _nameListModel->setStringList(_nameList);
    _nameListView->setCurrentIndex(_nameListModel->index(_nameListModel->rowCount() - 1));
}

void CommandWidget::setCurrentName(const QModelIndex &index)
{
    QString name = index.data().toString();
    QSqlQuery query(_db);
    //if (!query.prepare(QString("SELECT ID, Name, Command FROM [Command] WHERE Name='") +
    //    name +
    //    QString("' AND Catalog='") +
    //    _currentCatalog +
    //    QString("'")))
    //{
    //    return;
    //}
    if (!query.prepare(QString("SELECT ID, Name, Command FROM [Command] WHERE Name=:name AND Catalog=:catalog")))
    {
        return;
    }
    query.bindValue(":name", name);
    query.bindValue(":catalog", _currentCatalog);

    if (!query.exec())
    {
        return;
    }

    if (query.next())
    {
        _idEdit->setText(query.value(0).toString());
        _nameEdit->setText(query.value(1).toString());
        QByteArray cmd = query.value(2).toByteArray();
        QString text;
        for (int i = 0; i < cmd.size(); i++)
        {
            text.append(QString("%1").arg(static_cast<quint8>(cmd[i]), 2, 16, QChar('0')));
        }
        _cmdEdit->setText(text.toUpper());
    }
}

void CommandWidget::updateCommand(const QString &text)
{
    if (_nameListView->hasFocus())
        return;

    if (!_cmdEdit->hasFocus())
        return;

    if (_isAddNew)
        return;
    QString temp = text;
    temp.remove(" ");
    QByteArray cmd;
    for (int i = 0 ; i < temp.size(); i += 2)
    {
        cmd.append(static_cast<quint8>(temp.mid(i, 2).toInt(NULL, 16)));
    }
    QSqlQuery query(_db);
    //if (!query.prepare(QString("UPDATE [Command] SET Command='") +
    //    temp +
    //    QString("' WHERE Name='") +
    //    _nameListView->currentIndex().data().toString() +
    //    QString("' AND Catalog='") +
    //    _currentCatalog +
    //    QString("'")))
    //{
    //    QString err = query.lastError().databaseText();
    //    return;
    //}
    if (!query.prepare(QString("UPDATE [Command] SET Command=:command WHERE Name=:name AND Catalog=:catalog")))
    {
        QString err = query.lastError().databaseText();
        return;
    }
    query.bindValue(":command", cmd);
    query.bindValue(":name", _nameListView->currentIndex().data());
    query.bindValue(":catalog", _currentCatalog);

    if (!query.exec())
    {
        return;
    }
}

void CommandWidget::changeAddNew()
{
    if (!_nameEdit->hasFocus())
        return;
    _isAddNew = true;
    _cmdEdit->setText("");
}

void CommandWidget::deleteItem(const QString &catalog, QSqlDatabase &db)
{
    QString name = _nameListView->currentIndex().data().toString();
    int row = _nameListView->currentIndex().row();

    QSqlQuery query(db);

    //if (!query.prepare(QString("DELETE FROM [Command] WHERE Name='") +
    //    name +
    //    QString("' AND Catalog='") +
    //    catalog +
    //    QString("'")))
    //{
    //    return;
    //}
    if (!query.prepare(QString("DELETE FROM [Command] WHERE Name=:name AND Catalog=:catalog")))
    {
        return;
    }
    query.bindValue(":name", name);
    query.bindValue(":catalog", catalog);

    if (!query.exec())
    {
        return;
    }

    _nameList.removeAt(_nameList.indexOf(name));
    _nameListModel->setStringList(_nameList);

    if (row >= _nameList.size())
        row -= 1;
    _nameListView->setCurrentIndex(_nameListModel->index(row));
    setCurrentName(_nameListModel->index(row));
}

QStringList CommandWidget::catalogList()
{
    return _catalogList;
}

void CommandWidget::setCurrentCatalog(const QString &catalog)
{
    if (catalog.isEmpty())
        return;

    _currentCatalog = catalog;
    QSqlQuery query(_db);

    //if (!query.prepare(QString("SELECT Name FROM [Command] WHERE Catalog='") +
    //    catalog +
    //    QString("'")))
    //{
    //    return;
    //}
    if (!query.prepare(QString("SELECT Name FROM [Command] WHERE Catalog=:catalog")))
    {
        return;
    }
    query.bindValue(":catalog", catalog);
    if (!query.exec())
    {
        return;
    }
    _nameList.clear();
    while (query.next())
    {
        _nameList.append(query.value(0).toString());
    }

    QStringList::const_iterator it = qFind(_catalogList.begin(), _catalogList.end(), catalog);
    if (it == _catalogList.end())
    {
        _catalogList.append(catalog);
    }

    _nameListModel->setStringList(_nameList);
    _nameListView->setCurrentIndex(_nameListModel->index(0));
    setCurrentName(_nameListModel->index(0));
}

void CommandWidget::deleteAllItems(QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.exec("DELETE FROM [Command]");
    _catalogList.clear();
    _nameList.clear();
    _nameListModel->setStringList(_nameList);
    _nameListView->setCurrentIndex(_nameListModel->index(0));
    setCurrentName(_nameListModel->index(0));
}