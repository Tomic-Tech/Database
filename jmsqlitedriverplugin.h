#ifndef JM_SQLITE_DRIVER_PLUGIN_H
#define JM_SQLITE_DRIVER_PLUGIN_H

#include <QtSql/qsqldriverplugin.h>
#include "jmsqlitedriver.h"

class JMSQLiteDriverPlugin : public QSqlDriverPlugin {
	Q_OBJECT
public:
	JMSQLiteDriverPlugin();
	QSqlDriver* create(const QString&);
	QStringList keys() const;
};

#endif