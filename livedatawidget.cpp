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

    for (int i = 0; i < _langList.size(); i++)
    {
        QGroupBox *box = new QGroupBox(this);
        box->setTitle(_langList[i]);
        editLayout->addWidget(box);

        QVBoxLayout *boxLayout = new QVBoxLayout(box);
        box->setLayout(boxLayout);

        QGroupBox *defBox = new QGroupBox(box);
        defBox->setTitle(trUtf8("Default Value"));
        boxLayout->addWidget(defBox);
        QLineEdit *defEdit = new QLineEdit(defBox);
        QVBoxLayout *layout = new QVBoxLayout(defBox);
        defBox->setLayout(layout);
        layout->addWidget(defEdit);
        defBox->setMaximumHeight(50);
        
        _defEdits.insert(_langList[i], defEdit);

        QGroupBox *contentBox = new QGroupBox(box);
        contentBox->setTitle(trUtf8("Content"));
        boxLayout->addWidget(contentBox);
        QPlainTextEdit *contentEdit = new QPlainTextEdit(contentBox);
        layout = new QVBoxLayout(contentBox);
        layout->addWidget(contentEdit);
        contentBox->setLayout(layout);

        _contentEdits.insert(_langList[i], contentEdit);
    }
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
}


