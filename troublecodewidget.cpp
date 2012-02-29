#include "troublecodewidget.h"

TroubleCodeWidget::TroubleCodeWidget(QStringList &list, QWidget *parent /* = 0 */)
    : QWidget(parent)
    , _langList(list)
    , _textEdits()
    , _codeListView(0)
    , _codeListModel(0)
    , _codeList()
    , _currentCatalog()
    , _isAddNew(false)
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    setLayout(mainLayout);

    _codeListView = new QListView(this);
    _codeListView->setMaximumWidth(240);

    _codeListModel = new QStringListModel();
    _codeListView->setModel(_codeListModel);
    _codeListView->setEditTriggers(QListView::NoEditTriggers);
    _codeListView->setSelectionMode(QListView::SingleSelection);

    mainLayout->addWidget(_codeListView);

    QVBoxLayout *editLayout = new QVBoxLayout(this);

    mainLayout->addLayout(editLayout);

    {
        QGroupBox *box = new QGroupBox(this);
        box->setTitle(trUtf8("Code"));
        _codeEdit = new QLineEdit(box);
        QHBoxLayout *layout = new QHBoxLayout(box);
        box->setLayout(layout);
        layout->addWidget(_codeEdit);
        editLayout->addWidget(box);
    }

    for (int i = 0; i < _langList.size(); i++)
    {
        QGroupBox *box = new QGroupBox(this);
        box->setTitle(_langList[i]);
        QHBoxLayout *layout = new QHBoxLayout(box);
        box->setLayout(layout);

        QPlainTextEdit *edit = new QPlainTextEdit(box);
        edit->setObjectName(_langList[i]);
        layout->addWidget(edit);

        _textEdits.insert(_langList[i], edit);

        editLayout->addWidget(box);

        connect(edit, SIGNAL(textChanged()), this, SLOT(updateContent()));
    }

    connect(_codeListView, SIGNAL(clicked(QModelIndex)), this, SLOT(setCurrentCode(QModelIndex)));
    connect(_codeEdit, SIGNAL(textChanged(QString)), this, SLOT(changeAddNew()));
}

TroubleCodeWidget::~TroubleCodeWidget()
{
    delete _codeListModel;
}

void TroubleCodeWidget::setCurrentCatalog(const QString &catalog)
{
    _currentCatalog = catalog;
    QSqlQuery query(_db);
    if (!query.prepare(QString("SELECT Code FROM [TroubleCode") + 
        _langList[0] +
        QString("] WHERE Catalog=:catalog")/* +
        catalog +
        QString("'")*/))
    {
        return;
    }
    query.bindValue(":catalog", catalog);

    if (!query.exec())
    {
        return;
    }

    _codeList.clear();
    while (query.next())
    {
        _codeList.append(query.value(0).toString());
    }

    QStringList::const_iterator it = qFind(_catalogList.begin(), _catalogList.end(), catalog);
    if (it == _catalogList.end())
    {
        _catalogList.append(catalog);
    }

    _codeListModel->setStringList(_codeList);
    _codeListView->setCurrentIndex(_codeListModel->index(0));
    setCurrentCode(_codeListModel->index(0));
}

void TroubleCodeWidget::setDB(QSqlDatabase &db)
{
    _db = db;
    adjustItems(db);
}

void TroubleCodeWidget::createTable(QSqlDatabase &db)
{
    for (int i = 0; i < _langList.size(); i++)
    {
        QString text = QString("create table [TroubleCode");
        text += _langList[i];
        text += QString("] (ID int primary key, Code varchar(20), Content varchar(100), Catalog varchar(20))");
        QSqlQuery query(db);
        query.exec(text);
        QString error = query.lastError().databaseText();
    }
}

QHash<QString, QStringList> TroubleCodeWidget::queryCodes(QSqlDatabase &db)
{
    QHash<QString, QStringList> codeHash;

    for (int i = 0; i < _langList.size(); i++)
    {
        QStringList codeList;
        QSqlQuery query(db);
        QString text("SELECT Code FROM [TroubleCode");
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
            codeList.append(query.value(0).toString());
        }

END:
        codeHash.insert(_langList[i], codeList);
    }

    return codeHash;
}

QHash<QString, QStringList> TroubleCodeWidget::queryCatalogs(QSqlDatabase &db)
{
    QHash<QString, QStringList> catalogHash;

    for (int i = 0; i < _langList.size(); i++)
    {
        QStringList catalogList;
        QSqlQuery query(db);
        QString text("SELECT Catalog FROM [TroubleCode");
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

bool TroubleCodeWidget::checkCodeAndCatalog(const QStringList &checkedCode, 
    const QStringList &checkedCatalog, 
    const QString &code,
    const QString &catalog)
{
    QStringList::const_iterator codeIt = qFind(checkedCode.begin(), checkedCode.end(), code);
    QStringList::const_iterator catalogIt = qFind(checkedCatalog.begin(), checkedCatalog.end(), catalog);

    if (codeIt == checkedCode.end() ||
        catalogIt == checkedCatalog.end())
    {
        return false;
    }

    return true;
}

void TroubleCodeWidget::insertNewItem(const QString &code, const QString &catalog, const QString &lang, QSqlDatabase &db)
{
    _isAddNew = false;
    QSqlQuery query(db);
    if (!query.prepare(QString("SELECT max(ID) from [TroubleCode") + lang + QString("]")))
        return;
    if (!query.exec())
        return;
    if (!query.next())
        return;
    int count = query.value(0).toInt() + 1;

    QString insertText("INSERT INTO [TroubleCode");
    insertText += lang;
    insertText += QString("] VALUES(:id, :code, :content, :catalog)");
        //.arg(count)
        //.arg(code)
        //.arg(_textEdits.value(lang)->toPlainText())
        //.arg(catalog);

    if (!query.prepare(insertText))
    {
        QString str = query.lastError().databaseText();
        return;
    }
    query.bindValue(":id", count);
    query.bindValue(":code", code);
    query.bindValue(":content", _textEdits.value(lang)->toPlainText());
    query.bindValue(":catalog", catalog);
    if (!query.exec())
        return;
}

void TroubleCodeWidget::fixItems(const QHash<QString, QStringList> &codeHash, 
    const QHash<QString, QStringList> &catalogHash, 
    QSqlDatabase &db)
{
    for (int first_index = 0; first_index < _langList.size(); first_index++)
    {
        for (int second_index = first_index + 1; second_index < _langList.size(); second_index++)
        {
            QStringList firstCode = codeHash.value(_langList[first_index]);
            QStringList secondCode = codeHash.value(_langList[second_index]);
            QStringList firstCatalog = catalogHash.value(_langList[first_index]);
            QStringList secondCatalog = catalogHash.value(_langList[second_index]);

            for (int i = 0; i < firstCode.size(); i++)
            {
                for (int j = 0; j < firstCatalog.size(); j++)
                {
                    if (!checkCodeAndCatalog(secondCode, secondCatalog, firstCode[i], firstCatalog[j]))
                    {
                        insertNewItem(firstCode[i], firstCatalog[j], _langList[second_index], db);
                    }
                }
            }

            for (int i = 0; i < secondCode.size(); i++)
            {
                for (int j = 0; j < secondCatalog.size(); j++)
                {
                    if (!checkCodeAndCatalog(firstCode, firstCatalog, secondCode[i], secondCatalog[j]))
                    {
                        insertNewItem(secondCode[i], secondCatalog[j], _langList[first_index], db);
                    }

                }
            }
        }
    }
}

void TroubleCodeWidget::updateCodeAndCatalog(QSqlDatabase &db)
{
    _catalogList.clear();

    QSqlQuery query(db);
    query.prepare(QString("SELECT Catalog FROM [TroubleCode") + _langList[0] + QString("]"));
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

void TroubleCodeWidget::adjustItems(QSqlDatabase &db)
{
    QHash<QString, QStringList> codeHash = queryCodes(db);
    QHash<QString, QStringList> catalogHash = queryCatalogs(db);
    fixItems(codeHash, catalogHash, db);
    updateCodeAndCatalog(db);
}

void TroubleCodeWidget::insertNewItem(const QString &catalog, QSqlDatabase &db)
{
    for (int i = 0; i < _langList.size(); i++)
    {
        insertNewItem(_codeEdit->text(), catalog, _langList[i], db);
    }
    _codeList.append(_codeEdit->text());
    _codeListModel->setStringList(_codeList);
    _codeListView->setCurrentIndex(_codeListModel->index(_codeListModel->rowCount() - 1));
}

void TroubleCodeWidget::setCurrentCode(const QModelIndex &index)
{
    QString code = index.data().toString();
    for (int i = 0; i < _langList.size(); i++)
    {
        QSqlQuery query(_db);
        //if (!query.prepare(QString("SELECT Code, Content FROM [TroubleCode") + 
        //    _langList[i] + 
        //    QString("]") +
        //    QString(" WHERE Code='") +
        //    code +
        //    QString("'") +
        //    QString(" AND Catalog='") +
        //    _currentCatalog +
        //    QString("'")))
        //{
        //    continue;
        //}
        if (!query.prepare(QString("SELECT Code, Content FROM [TroubleCode") +
            _langList[i] +
            QString("] WHERE Code=:code AND Catalog=:catalog")))
        {
            continue;
        }
        query.bindValue(":code", code);
        query.bindValue(":catalog", _currentCatalog);

        if (!query.exec())
        {
            continue;
        }

        if (query.next())
        {
            _codeEdit->setText(query.value(0).toString());
            _textEdits.value(_langList[i])->setPlainText(query.value(1).toString());
        }
    }
}

void TroubleCodeWidget::updateContent()
{
    QPlainTextEdit *edit = qobject_cast<QPlainTextEdit*>(sender());
    if (edit == NULL)
        return;

    if (!edit->hasFocus())
        return;

    if (_codeListView->hasFocus())
        return;

    if (_isAddNew)
        return;

    QSqlQuery query(_db);
    //if (!query.prepare(QString("UPDATE [TroubleCode") + 
    //    edit->objectName() + 
    //    QString("] SET Content='") + 
    //    edit->toPlainText() +
    //    QString("' WHERE Code='") +
    //    _codeListView->currentIndex().data().toString() +
    //    QString("' AND Catalog='") +
    //    _currentCatalog +
    //    QString("'")))
    //{
    //    QString err = query.lastError().databaseText();
    //    return;
    //}
    if (!query.prepare(QString("UPDATE [TroubleCode") +
        edit->objectName() +
        QString("] SET Content=:content WHERE Code=:code AND Catalog=:catalog")))
    {
        QString err = query.lastError().databaseText();
        return;
    }

    query.bindValue(":content", edit->toPlainText());
    query.bindValue(":code", _codeListView->currentIndex().data().toString());
    query.bindValue(":catalog", _currentCatalog);

    if (!query.exec())
    {
        return;
    }
}

void TroubleCodeWidget::changeAddNew()
{
    _isAddNew = true;
    for (int i = 0; i < _langList.size(); i++)
    {
        _textEdits.value(_langList[i])->setPlainText("");
    }
}

void TroubleCodeWidget::deleteItem(const QString &catalog, QSqlDatabase &db)
{
    QString code = _codeListView->currentIndex().data().toString();
    int row = _codeListView->currentIndex().row();

    for (int i = 0; i < _langList.size(); i++)
    {
        QSqlQuery query(db);

        //if (!query.prepare(QString("DELETE FROM [TroubleCode") +
        //    _langList[i] +
        //    QString("] WHERE Code='") +
        //    code +
        //    QString("'") +
        //    QString(" AND Catalog='") +
        //    catalog +
        //    QString("'")))
        //{
        //    continue;
        //}
        if (!query.prepare(QString("DELETE FROM [TroubleCode") +
            _langList[i] +
            QString("] WHERE Code=:code AND Catalog=:catalog")))
        {
            continue;
        }
        query.bindValue(":code", code);
        query.bindValue(":catalog", catalog);

        if (!query.exec())
        {
            continue;
        }
    }

    _codeList.removeAt(_codeList.indexOf(code));
    _codeListModel->setStringList(_codeList);

    if (row >= _codeList.size())
        row -= 1;
    _codeListView->setCurrentIndex(_codeListModel->index(row));
    setCurrentCode(_codeListModel->index(row));
}

QStringList TroubleCodeWidget::catalogList()
{
    return _catalogList;
}

void TroubleCodeWidget::modifyCatalog(const QString &current, const QString &modify, QSqlDatabase &db)
{
    for (int i = 0; i < _langList.size(); i++)
    {
        QSqlQuery query(_db);
        if (!query.prepare(QString("UPDATE [TroubleCode") +
            _langList[i] +
            QString("] SET Catalog=:modify WHERE Catalog=:current")))
            return;
        query.bindValue(":modify", modify);
        query.bindValue(":current", current);
        if (!query.exec())
            continue;
    }
    updateCodeAndCatalog(db);
}