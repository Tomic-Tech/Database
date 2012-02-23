#include "commandmodel.h"

CommandModel::CommandModel(QObject *parent, QSqlDatabase db)
	: QSqlTableModel(parent, db) {
}

QVariant CommandModel::data(const QModelIndex &idx, int role) const {
	if (!idx.isValid())
		return QVariant();

	switch (role) {
	case Qt::DisplayRole:
		if (idx.column() == 2) {
			QVariant var = QSqlTableModel::data(idx, role);
			QByteArray cmd = var.toByteArray();
			QString disp;
			for (int i = 0; i < cmd.size(); i++) {
				disp.append(QString("%1 ").arg(static_cast<ushort>(cmd[i]), 2, 16, QChar('0')));
			}
			return QVariant(disp);
		} else {
			return QSqlTableModel::data(idx, role);
		}
		break;
	default:
		return QSqlTableModel::data(idx, role);
	}
}

bool CommandModel::setData(const QModelIndex &index, const QVariant &value, int role) {
	if (!index.isValid())
		return false;
	switch (role) {
	case Qt::EditRole:
		if (index.column() == 2) {
			QString disp = value.toString();
			disp.replace(" ", "");
			if (disp.size() %2 != 0) {
				disp.prepend("0");
			}
			QByteArray cmd;
			for (int i = 0; i < disp.size(); i += 2) {
				QString temp = disp.mid(i, 2);
				ushort b = temp.toUShort(NULL, 16);
				cmd.append(static_cast<quint8>(b));
			}
			return QSqlTableModel::setData(index, QVariant(cmd), role);
		} else {
			return QSqlTableModel::setData(index, value, role);
		}
		break;
	default:
		return QSqlTableModel::setData(index, value, role);
	}
}