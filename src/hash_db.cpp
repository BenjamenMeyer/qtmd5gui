#include <hash_db.h>

#include <QtSql/QSqlDatabase>

HashDb::HashDb(QObject* _parent) : QObject(_parent)
	{
	db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName(":memory:");
	}

HashDb::~HashDb()
	{
	}

void HashDb::addDirectory(QString _path)
	{
	}
void HashDb::addFile(int _module, QString _path)
	{
	}
void HashDb::setMode(bool _generate)
	{
	}
void HashDb::generateMissingObjects()
	{
	}
void HashDb::generateNewObjects()
	{
	}
