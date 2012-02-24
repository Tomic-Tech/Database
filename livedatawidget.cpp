#include "livedatawidget.h"

LiveDataWidget::LiveDataWidget(QStringList &langList, QWidget *parent /* = 0 */)
    : QWidget (parent)
    , _langList(langList)
    , _shortNameListView(0)
    , _shortNameListModel(0)
    , _shortNameEdit(0)
    , _unitEdit(0)
    , _commandIDBox(0)
    , _algorithmIDBox(0)
    , _contentEdits()
    , _defEdits()
    , _catalogList()
    , _shortNameList()
    , _currentCatalog()
    , _isAddNew(false)
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    setLayout(mainLayout);

    _shortNameListView = new QListView(this);
    _shortNameListView->setMaximumWidth(120);

    _shortNameListModel = new QStringListModel();
    _shortNameListView->setModel(_shortNameListModel);

    mainLayout->addWidget(_shortNameListView);

    QVBoxLayout *editLayout = new QVBoxLayout(this);

    mainLayout->addLayout(editLayout);

    {
        QHBoxLayout *hlayout = new QHBoxLayout(this);
        editLayout->addLayout(hlayout);
        // Short Name edit, for add new item
        QGroupBox *box = new QGroupBox(this);
        box->setTitle(trUtf8("Short Name"));
        box->setMaximumHeight(50);
        QHBoxLayout *layout = new QHBoxLayout(box);
        _shortNameEdit = new QLineEdit(box);
        box->setLayout(layout);
        layout->addWidget(_shortNameEdit);

        hlayout->addWidget(box);

        // Unit
        box = new QGroupBox(this);
        box->setTitle(trUtf8("Unit"));
        box->setMaximumWidth(100);
        box->setMaximumHeight(50);
        layout = new QHBoxLayout(box);
        _unitEdit = new QLineEdit(box);
        box->setLayout(layout);
        layout->addWidget(_unitEdit);

        hlayout->addWidget(box);

        // Command ID.
        box = new QGroupBox(this);
        box->setTitle(trUtf8("Command ID"));
        box->setMaximumWidth(100);
        box->setMaximumHeight(50);
        layout = new QHBoxLayout(box);
        _commandIDBox = new QComboBox(box);
        box->setLayout(layout);
        layout->addWidget(_commandIDBox);

        hlayout->addWidget(box);

        // Algorithm ID.
        box = new QGroupBox(this);
        box->setTitle(trUtf8("Algorithm ID"));
        box->setMaximumWidth(100);
        box->setMaximumHeight(50);
        layout = new QHBoxLayout(box);
        _algorithmIDBox = new QComboBox(box);
        box->setLayout(layout);
        layout->addWidget(_algorithmIDBox);

        hlayout->addWidget(box);
    }

    connect(_unitEdit, SIGNAL(textChanged(QString)), this, SLOT(updateUnit(QString)));
    connect(_commandIDBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(updateCommboxID(QString)));
    connect(_algorithmIDBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(updateAlgorithmID(QString)));
    connect(_shortNameEdit, SIGNAL(textChanged(QString)), this, SLOT(changeAddNew()));

    for (int i = 0; i < _langList.size(); i++)
    {
        QGroupBox *box = new QGroupBox(this);
        box->setTitle(_langList[i]);
        editLayout->addWidget(box);

        QVBoxLayout *boxLayout = new QVBoxLayout(box);
        box->setLayout(boxLayout);

        // Defalut Value
        QGroupBox *defBox = new QGroupBox(box);
        defBox->setTitle(trUtf8("Default Value"));
        boxLayout->addWidget(defBox);
        QLineEdit *defEdit = new QLineEdit(defBox);
        defEdit->setObjectName(_langList[i]);
        QVBoxLayout *layout = new QVBoxLayout(defBox);
        defBox->setLayout(layout);
        layout->addWidget(defEdit);
        defBox->setMaximumHeight(50);
        
        _defEdits.insert(_langList[i], defEdit);

        // Content Edit
        QGroupBox *contentBox = new QGroupBox(box);
        contentBox->setTitle(trUtf8("Content"));
        boxLayout->addWidget(contentBox);
        QPlainTextEdit *contentEdit = new QPlainTextEdit(contentBox);
        contentEdit->setObjectName(_langList[i]);
        layout = new QVBoxLayout(contentBox);
        layout->addWidget(contentEdit);
        contentBox->setLayout(layout);

        _contentEdits.insert(_langList[i], contentEdit);

        connect(defEdit, SIGNAL(textChanged(QString)), this, SLOT(updateDefaultValue(QString)));
        connect(contentEdit, SIGNAL(textChanged()), this, SLOT(updateContent()));
    }

    connect(_shortNameListView, SIGNAL(clicked(QModelIndex)), this, SLOT(setCurrentCode(QModelIndex)));
}

LiveDataWidget::~LiveDataWidget()
{
    delete _shortNameListModel;
}

void LiveDataWidget::createTable(QSqlDatabase &db)
{
    for (int i = 0; i < _langList.size(); i++)
    {
        QString text("create table [LiveData");
        text += _langList[i];
        text += QString("] (ID int primary key, ShortName varchar(100), Content varchar(100), "
            "DefaultValue varchar(100), Unit varchar(20), CommandID int, "
            "AlgorithmID int, Catalog varchar(20));");

        QSqlQuery query(db);
        query.exec(text);
        QString err = query.lastError().databaseText();
    }
}

void LiveDataWidget::setDB(QSqlDatabase &db)
{
    _db = db;
    adjustItems(db);
}

QHash<QString, QStringList> LiveDataWidget::queryShortNames(QSqlDatabase &db)
{
    QHash<QString, QStringList> shortNameHash;

    for (int i = 0; i < _langList.size(); i++)
    {
        QStringList shortNameList;
        QSqlQuery query(db);
        QString text("SELECT ShortName FROM [LiveData");
        text += _langList[i];
        text += "]";

        if (!query.prepare(text))
        {
            goto END;
        }

        if (!query.exec())
        {
            goto END;
        }

        while (query.next())
        {
            shortNameList.append(query.value(0).toString());
        }

END:
        shortNameHash.insert(_langList[i], shortNameList);
    }

    return shortNameHash;
}

QHash<QString, QStringList> LiveDataWidget::queryCatalogs(QSqlDatabase &db)
{
    QHash<QString, QStringList> catalogHash;

    for (int i = 0; i < _langList.size(); i++)
    {
        QStringList catalogList;
        QSqlQuery query(db);
        QString text("SELECT Catalog FROM [LiveData");
        text += _langList[i];
        text += "]";

        if (!query.prepare(text))
        {
            goto END;
        }

        if (!query.exec())
        {
            goto END;
        }

        while (query.next())
        {
            QStringList::const_iterator it = qFind(catalogList.begin(), catalogList.end(), query.value(0).toString());
            if (it == catalogList.end())
            {
                catalogList.append(query.value(0).toString());
            }
        }

END:
        catalogHash.insert(_langList[i], catalogList);
    }

    return catalogHash;
}

bool LiveDataWidget::checkShortNameAndCatalog(const QStringList &checkedShortName,
    const QStringList &checkedCatalog,
    const QString &shortName,
    const QString &catalog)
{
    QStringList::const_iterator shortNameIt = qFind(checkedShortName.begin(), checkedShortName.end(), shortName);
    QStringList::const_iterator catalogIt = qFind(checkedCatalog.begin(), checkedCatalog.end(), catalog);

    if (shortNameIt == checkedShortName.end() ||
        catalogIt == checkedCatalog.end())
    {
        return false;
    }

    return true;
}

void LiveDataWidget::insertNewItem(const QString shortName, 
    const QString &catalog,
    const QString &lang,
    QSqlDatabase &db)
{
    _isAddNew = false;
    QSqlQuery query(db);

    if (!query.prepare(QString("SELECT max(ID) from [LiveData") +
        lang + QString("]")))
    {
        return;
    }

    if (!query.exec())
    {
        return;
    }

    if (!query.next())
    {
        return;
    }

    int count = query.value(0).toInt() + 1;

    QString insertText("INSERT INTO [LiveData");
    insertText += lang;
    insertText += QString("] VALUES(%1, '%2', '%3', '%4', '%5', %6, %7, '%8'")
        .arg(count).arg(shortName).arg(_contentEdits.value(lang)->toPlainText())
        .arg(_defEdits.value(lang)->text())
        .arg(_unitEdit->text())
        .arg(_commandIDBox->currentText())
        .arg(_algorithmIDBox->currentText())
        .arg(catalog);

    if (!query.prepare(insertText))
    {
        QString str = query.lastError().databaseText();
        return;
    }

    query.exec();
}

void LiveDataWidget::fixItems(const QHash<QString, QStringList> &shortNameHash, 
    const QHash<QString, QStringList> &catalogHash, 
    QSqlDatabase &db)
{
    for (int first_index = 0; first_index < _langList.size(); first_index ++)
    {
        for (int second_index = first_index + 1; second_index < _langList.size(); second_index++)
        {
            QStringList firstShortName = shortNameHash.value(_langList[first_index]);
            QStringList secondShortName = shortNameHash.value(_langList[second_index]);
            QStringList firstCatalog = catalogHash.value(_langList[first_index]);
            QStringList secondCatalog = catalogHash.value(_langList[second_index]);

            for (int i = 0; i < firstShortName.size(); i++)
            {
                for (int j = 0; j < firstCatalog.size(); j++)
                {
                    if (!checkShortNameAndCatalog(secondShortName, secondCatalog, firstShortName[i], firstCatalog[j]))
                    {
                        insertNewItem(firstShortName[i], firstCatalog[j], _langList[second_index], db);
                    }
                }
            }

            for (int i = 0; i < secondShortName.size(); i++)
            {
                for (int j = 0; j < secondCatalog.size(); j++)
                {
                    if (!checkShortNameAndCatalog(firstShortName, firstCatalog, secondShortName[i], secondCatalog[j]))
                    {
                        insertNewItem(secondShortName[i], secondCatalog[j], _langList[first_index], db);
                    }

                }
            }
        }
    }
}

void LiveDataWidget::updateShortNameAndCatalog(QSqlDatabase &db)
{
    _catalogList.clear();

    QSqlQuery query(db);
    query.prepare(QString("SELECT Catalog FORM [LiveData") + _langList[0] + QString("]"));
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

void LiveDataWidget::adjustItems(QSqlDatabase &db)
{
    QHash<QString, QStringList> shortNameHash = queryShortNames(db);
    QHash<QString, QStringList> catalogHash = queryCatalogs(db);
    fixItems(shortNameHash, catalogHash, db);
    updateShortNameAndCatalog(db);
}

void LiveDataWidget::setCurrentShortName(const QModelIndex &index)
{
    QString shortName = index.data().toString();
    for (int i = 0; i < _langList.size(); i++)
    {
        QSqlQuery query(_db);
        if (!query.prepare(QString("SELECT Content, DefaultValue, Unit, CommandID, AlgorithmID FROM [LiveData") +
            _langList[i] + 
            QString("]") +
            QString("WHERE ShortName='") +
            shortName +
            QString("'") +
            QString(" AND Catalog='") +
            _currentCatalog +
            QString("'")))
        {
            continue;
        }

        if (!query.exec())
        {
            continue;
        }

        if (query.next())
        {
            _contentEdits.value(_langList[i])->setPlainText(query.value(0).toString());
            _defEdits.value(_langList[i])->setText(query.value(1).toString());
            _unitEdit->setText(query.value(2).toString());
            _commandIDBox->setCurrentIndex(_commandIDBox->findText(query.value(3).toString()));
            _algorithmIDBox->setCurrentIndex(_algorithmIDBox->findText(query.value(4).toString()));
        }
    }
}

void LiveDataWidget::setCurrentCatalog(const QString &catalog)
{
    _currentCatalog = catalog;
    QSqlQuery query(_db);
    if (!query.prepare(QString("SELECT ShortName FROM [LiveData") +
        _langList[0] +
        QString("] WHERE Catalog='") +
        catalog +
        QString("'")))
    {
        return;
    }

    if (!query.exec())
    {
        return;
    }

    _shortNameList.clear();
    while (query.next())
    {
        _shortNameList.append(query.value(0).toString());
    }

    QStringList::const_iterator it = qFind(_catalogList.begin(), _catalogList.end(), catalog);
    if (it == _catalogList.end())
    {
        _catalogList.append(catalog);
    }

    _shortNameListModel->setStringList(_shortNameList);
    _shortNameListView->setCurrentIndex(_shortNameListModel->index(0));
    setCurrentShortName(_shortNameListModel->index(0));
}

void LiveDataWidget::updateContent()
{
    QPlainTextEdit *edit = qobject_cast<QPlainTextEdit*>(sender());
    if (edit == NULL)
        return;

    if (_shortNameListView->hasFocus())
        return;

    if (_isAddNew)
        return;

    QSqlQuery query(_db);
    if (!query.prepare(QString("UPDATE [LiveData") +
        edit->objectName() + 
        QString("] Set Content='") +
        edit->toPlainText() +
        QString("'") + 
        QString("'") +
        QString(" WHERE ShortName='") +
        _shortNameListView->currentIndex().data().toString() +
        QString("' AND Catalog='") +
        _currentCatalog +
        QString("'")))
    {
        QString err = query.lastError().databaseText();
        return;
    }

    query.exec();
}

void LiveDataWidget::updateUnit(const QString &text)
{
    if (_shortNameListView->hasFocus())
        return;

    if (_isAddNew)
        return;

    QSqlQuery query(_db);

    for (int i = 0; i < _langList.size(); i++)
    {
        if (!query.prepare(QString("UPDATE [LiveData") +
            _langList[i] +
            QString("] SET Unit='") +
            text +
            QString("' WHERE ShortName='") +
            _shortNameListView->currentIndex().data().toString() +
            QString("' AND Catalog='") +
            _currentCatalog +
            QString("'")))
        {
            QString err = query.lastError().databaseText();
            continue;
        }

        query.exec();
    }
}

void LiveDataWidget::updateCommboxID(const QString &text)
{
    if (_shortNameListView->hasFocus())
        return;

    if (_isAddNew)
        return;

    QSqlQuery query(_db);

    for (int i = 0; i < _langList.size(); i++)
    {
        if (!query.prepare(QString("UPDATE [LiveData") +
            _langList[i] +
            QString("] SET CommboxID=") +
            text +
            QString(" WHERE ShortName='") +
            _shortNameListView->currentIndex().data().toString() +
            QString("' AND Catalog='") +
            _currentCatalog))
        {
            QString err = query.lastError().databaseText();
            continue;
        }

        query.exec();
    }
}

void LiveDataWidget::updateAlgorithmID(const QString &text)
{
    if (_shortNameListView->hasFocus())
        return;

    if (_isAddNew)
        return;

    QSqlQuery query(_db);

    for (int i = 0; i < _langList.size(); i++)
    {
        if (!query.prepare(QString("UPDATE [LiveData") +
            _langList[i] +
            QString("] SET AlgorithmID=") +
            text +
            QString(" WHERE ShortName='") +
            _shortNameListView->currentIndex().data().toString() +
            QString("' AND Catalog='") +
            _currentCatalog +
            QString("'")))
        {
            QString err = query.lastError().databaseText();
            continue;
        }

        query.exec();
    }
}

void LiveDataWidget::updateDefaultValue(const QString &text)
{
    if (_shortNameListView->hasFocus())
        return;

    if (_isAddNew)
        return;

    QSqlQuery query(_db);

    if (!query.prepare(QString("UPDATE [LiveData") +
        sender()->objectName() +
        QString("] SET DefaultValue=") +
        text +
        QString(" WHERE ShortName='") +
        _shortNameListView->currentIndex().data().toString() +
        QString("' AND Catalog='") +
        _currentCatalog +
        QString("'")))
    {
        QString err = query.lastError().databaseText();
        return;
    }

    query.exec();
}

void LiveDataWidget::changeAddNew()
{
    _isAddNew = true;
    for (int i = 0; i < _langList.size(); i++)
    {
        _contentEdits.value(_langList[i])->setPlainText("");
        _defEdits.value(_langList[i])->setText("");
        _unitEdit->setText("");
        _commandIDBox->setCurrentIndex(0);
        _algorithmIDBox->setCurrentIndex(0);
    }
}

void LiveDataWidget::deleteItem(const QString &catalog, QSqlDatabase &db)
{
    QString shortName = _shortNameListView->currentIndex().data().toString();
    int row = _shortNameListView->currentIndex().row();

    for (int i = 0; i < _langList.size(); i++)
    {
        QSqlQuery query(db);

        if (!query.prepare(QString("DELETE FROM [LiveData") +
            _langList[i] +
            QString("] WHERE ShortName='") +
            shortName +
            QString("' AND Catalog='") +
            catalog +
            QString("'")))
        {
            continue;
        }

        if (!query.exec())
        {
            continue;
        }
    }

    _shortNameList.removeAt(_shortNameList.indexOf(shortName));
    _shortNameListModel->setStringList(_shortNameList);

    if (row >= _shortNameList.size())
        row -= 1;

    _shortNameListView->setCurrentIndex(_shortNameListModel->index(row));
    setCurrentShortName(_shortNameListModel->index(row));
}

QStringList LiveDataWidget::catalogList()
{
    return _catalogList;
}

void LiveDataWidget::insertNewItem(const QString &catalog, QSqlDatabase &db)
{
    for (int i = 0; i < _langList.size(); i++)
    {
        insertNewItem(_shortNameEdit->text(), catalog, _langList[i], db);
    }
    _shortNameList.append(_shortNameEdit->text());
    _shortNameListModel->setStringList(_shortNameList);
    _shortNameListView->setCurrentIndex(_shortNameListModel->index(_shortNameListModel->rowCount()  -1));
}