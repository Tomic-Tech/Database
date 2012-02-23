#include "jmsqlitedriverplugin.h"

JMSQLiteDriverPlugin::JMSQLiteDriverPlugin()
	: QSqlDriverPlugin() {

}

QSqlDriver* JMSQLiteDriverPlugin::create(const QString &name) {
    if (name == QLatin1String("JMSQLITE")) {
        JMSQLiteDriver* driver = new JMSQLiteDriver();
        return driver;
    }
    return 0;
}

QStringList JMSQLiteDriverPlugin::keys() const {
    QStringList l;
    l  << QLatin1String("JMSQLITE");
    return l;
}

Q_EXPORT_STATIC_PLUGIN(JMSQLiteDriverPlugin)
Q_EXPORT_PLUGIN2(jmsqlite, JMSQLiteDriverPlugin)
Q_IMPORT_PLUGIN(jmsqlite)