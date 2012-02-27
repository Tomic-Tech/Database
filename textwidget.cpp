#include "textwidget.h"

TextWidget::TextWidget(QStringList &langList, QWidget *parent /* = 0 */)
    : QWidget(parent)
    , _langList(langList)
    , _textEdits()
    , _nameListView(0)
    , _nameListModel(0)
    , _nameList()
    , _isAddNew(false)
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    setLayout(mainLayout);

    _nameListView = new QListView(this);
    _nameListView->setMaximumWidth(120);

    _nameListModel = new QStringListModel();
    _nameListView->setModel(_nameListModel);
    _nameListView->setEditTriggers(QListView::NoEditTriggers);
    _nameListView->setSelectionMode(QListView::SingleSelection);

    mainLayout->addWidget(_nameListView);

    QVBoxLayout *editLayout = new QVBoxLayout(this);

    {
        QGroupBox *box = new QGroupBox(this);
        box->setTitle(trUtf8("Name"));
        QHBoxLayout *layout = new QHBoxLayout(box);
        _nameEdit = new QLineEdit(box);
        box->setLayout(layout);
        layout->addWidget(_nameEdit);
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

    mainLayout->addLayout(editLayout);
    connect(_nameListView, SIGNAL(clicked(QModelIndex)), this, SLOT(setCurrentName(QModelIndex)));
    connect(_nameEdit, SIGNAL(textChanged(QString)), this, SLOT(changeAddNew()));
}

TextWidget::~TextWidget()
{
    delete _nameListModel;
}

void TextWidget::setDB(QSqlDatabase &db)
{
    _db = QSqlDatabase(db);
    adjustItems(db);
}

void TextWidget::createTable(QSqlDatabase &db)
{
    for (int i = 0; i < _langList.size(); i++)
    {
        QString text = QString("create table [Text");
        text += _langList[i];
        text += QString("] (ID int primary key, Name varchar(20), Content varchar(100))");
        QSqlQuery query(db);
        query.exec(text);
        QString error = query.lastError().databaseText();
    }
}

QHash<QString, QStringList> TextWidget::queryNames(QSqlDatabase &db)
{
    QHash<QString, QStringList> nameHash;

    for (int i = 0; i < _langList.size(); i++)
    {
        QStringList nameList;
        QSqlQuery query(db);
        QString text("SELECT Name FROM [Text");
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
            nameList.append(query.value(0).toString());
        }

END:
        nameHash.insert(_langList[i], nameList);
    }

    return nameHash;
}

bool TextWidget::checkName(const QStringList &beChecked, const QString &key)
{
    QStringList::const_iterator it = qFind(beChecked.begin(), beChecked.end(), key);
    if (it == beChecked.end())
    {
        return false;
    }
    return true;
}

void TextWidget::insertNewName(const QString &key, const QString &lang, QSqlDatabase &db)
{
    _isAddNew = false;
    QSqlQuery query(db);
    if (!query.prepare(QString("SELECT max(ID) from [Text") + lang + QString("]")))
        return;
    if (!query.exec())
        return;
    if (!query.next())
        return;
    int count = query.value(0).toInt() + 1;

    //QString insertText("INSERT INTO [Text");
    //insertText += lang;
    //insertText += QString("] VALUES(%1, '%2', '%3')").arg(count).arg(key).arg(_textEdits.value(lang)->toPlainText());
    QString insertText("INSERT INTO [Text");
    insertText += lang;
    insertText += QString("] VALUES(:id, :name, :content");

    if (!query.prepare(insertText))
    {
        QString str = query.lastError().databaseText();
        return;
    }

    query.bindValue(":id", count);
    query.bindValue(":name", key);
    query.bindValue(":content", _textEdits.value(lang)->toPlainText());

    if (!query.exec())
        return;
}

void TextWidget::fixNames(const QHash<QString, QStringList> &nameHash, QSqlDatabase &db)
{
    for (int first_index = 0; first_index < _langList.size(); first_index++)
    {
        for (int second_index = first_index + 1; second_index < _langList.size(); second_index++)
        {
            QStringList first = nameHash.value(_langList[first_index]);
            QStringList second = nameHash.value(_langList[second_index]);

            for (int i = 0; i < first.size(); i++)
            {
                if (!checkName(second, first[i]))
                {
                    insertNewName(first[i], _langList[second_index], db);
                }
            }

            for (int i = 0; i < second.size(); i++)
            {
                if (!checkName(first, second[i]))
                {
                    insertNewName(second[i], _langList[first_index], db);
                }
            }
        }
    }
}

void TextWidget::updateCurrentNameList(QSqlDatabase &db)
{
    _nameList.clear();
    QSqlQuery query(db);
    query.prepare("SELECT Name FROM [Text" + _langList[0] + QString("]"));
    query.exec();
    while (query.next())
    {
        _nameList.append(query.value(0).toString());
    }
    _nameListModel->setStringList(_nameList);
    _nameListView->scrollTo(_nameListModel->index(0));
}

void TextWidget::adjustItems(QSqlDatabase &db)
{
    QHash<QString, QStringList> nameHash = queryNames(db);
    fixNames(nameHash, db);
    updateCurrentNameList(db);
}

void TextWidget::insertNewName(QSqlDatabase &db)
{
    for (int i = 0; i < _langList.size(); i++)
    {
        insertNewName(_nameEdit->text(), _langList[i], db);
    }
    _nameList.append(_nameEdit->text());
    _nameListModel->setStringList(_nameList);
    _nameListView->setCurrentIndex(_nameListModel->index(_nameListModel->rowCount() - 1));
}

void TextWidget::setCurrentName(const QModelIndex &index)
{
    QString name = index.data().toString();
    for (int i = 0; i < _langList.size(); i++)
    {
        QSqlQuery query(_db);
        //if (!query.prepare(QString("SELECT Content FROM [Text") + 
        //    _langList[i] + 
        //    QString("]") +
        //    QString(" WHERE Name='") +
        //    name +
        //    QString("'")))
        //{
        //    continue;
        //}
        if (!query.prepare(QString("SELECT Content FROM [Text") +
            _langList[i] +
            QString("] WHERE Name=:name")))
        {
            continue;
        }
        query.bindValue(":name", name);

        if (!query.exec())
        {
            continue;
        }

        if (query.next())
        {
            _textEdits.value(_langList[i])->setPlainText(query.value(0).toString());
        }
    }
}

void TextWidget::updateContent()
{
    QPlainTextEdit *edit = qobject_cast<QPlainTextEdit*>(sender());
    if (edit == NULL)
        return;

    if (_nameListView->hasFocus())
        return;

    if (_isAddNew)
        return;

    QSqlQuery query(_db);
    //if (!query.prepare(QString("UPDATE [Text") + 
    //    edit->objectName() + 
    //    QString("] SET Content='") + 
    //    edit->toPlainText() +
    //    QString("'") +
    //    QString(" WHERE Name='") +
    //    _nameListView->currentIndex().data().toString() +
    //    QString("'")))
    //{
    //    QString err = query.lastError().databaseText();
    //    return;
    //}
    if (!query.prepare(QString("UPDATE [Text") +
        edit->objectName() +
        QString("] SET Content=:content WHERE Name=:name")))
    {
        QString err = query.lastError().databaseText();
        return;
    }
    query.bindValue(":content", edit->toPlainText());
    query.bindValue(":name", _nameListView->currentIndex().data());

    if (!query.exec())
    {
        return;
    }
}

void TextWidget::changeAddNew()
{
    _isAddNew = true;
    for (int i = 0; i < _langList.size(); i++)
    {
        _textEdits.value(_langList[i])->setPlainText("");
    }
}

void TextWidget::deleteItem(QSqlDatabase &db)
{
    QString name = _nameListView->currentIndex().data().toString();
    int row = _nameListView->currentIndex().row();

    for (int i = 0; i < _langList.size(); i++)
    {
        QSqlQuery query(db);

        //if (!query.prepare(QString("DELETE FROM [Text") +
        //    _langList[i] +
        //    QString("] WHERE Name='") +
        //    name +
        //    QString("'")))
        //{
        //    continue;
        //}
        if (!query.prepare(QString("DELETE FROM [Text") +
            _langList[i] +
            QString("] WHERE Name=:name")))
        {
            continue;
        }
        query.bindValue(":name", name);

        if (!query.exec())
        {
            continue;
        }
    }

    _nameList.removeAt(_nameList.indexOf(name));
    _nameListModel->setStringList(_nameList);

    if (row >= _nameList.size())
        row -= 1;
    _nameListView->setCurrentIndex(_nameListModel->index(row));
    setCurrentName(_nameListModel->index(row));
}