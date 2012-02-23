#ifndef COMMAND_MODEL_H
#define COMMAND_MODEL_H

#ifdef _MSC_VER
#pragma once
#endif

#include <QtCore>
#include <QtSql>

class CommandModel : public QSqlTableModel {
public:
	CommandModel(QObject *parent = 0, QSqlDatabase db = QSqlDatabase());
	QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
};


#endif
