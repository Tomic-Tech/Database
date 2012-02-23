#ifndef JM_SQLITE_DRIVER_H
#define JM_SQLITE_DRIVER_H

#include <QtSql/qsqldriver.h>
#include <QtSql/qsqlresult.h>
#include <QtSql/private/qsqlcachedresult_p.h>
#include "sqlite3.h"

class JMSQLiteDriverPrivate;
class JMSQLiteResultPrivate;
class JMSQLiteDriver;

class JMSQLiteResult : public QSqlCachedResult {
	friend class JMSQLiteDriver;
	friend class JMSQLiteResultPrivate;
public:
	explicit JMSQLiteResult(const JMSQLiteDriver *db);
	~JMSQLiteResult();
	QVariant handle() const;

protected:
	bool gotoNext(QSqlCachedResult::ValueCache& row, int idx);
	bool reset(const QString &query);
	bool prepare(const QString &query);
	bool exec();
	int size();
	int numRowsAffected();
	QVariant lastInsertId() const;
	QSqlRecord record() const;
	void virtual_hook(int id, void *data);

private:
	JMSQLiteResultPrivate *d;
};

class JMSQLiteDriver : public QSqlDriver {
	Q_OBJECT
	friend class JMSQLiteResult;
public:
	explicit JMSQLiteDriver(QObject *parent = 0);
	explicit JMSQLiteDriver(sqlite3 *connection, QObject *parent = 0);
	~JMSQLiteDriver();
	bool hasFeature(DriverFeature f) const;
	bool open(const QString & db,
                   const QString & user,
                   const QString & password,
                   const QString & host,
                   int port,
                   const QString & connOpts);
	void close();
	QSqlResult *createResult() const;
	bool beginTransaction();
	bool commitTransaction();
	bool rollbackTransaction();
	QStringList tables(QSql::TableType) const;

	QSqlRecord record(const QString& tablename) const;
	QSqlIndex primaryIndex(const QString &table) const;
	QVariant handle() const;
	QString escapeIdentifier(const QString &identifier, IdentifierType) const;

private:
	JMSQLiteDriverPrivate *d;
};



#endif