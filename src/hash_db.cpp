#include <hash_db.h>

#include <QtCore/QDebug>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

QStringList schemas = (
		QStringList() <<
		QString("CREATE TABLE IF NOT EXISTS master_directory (hash TEXT NOT NULL PRIMARY KEY, path TEXT NOT NULL)") <<
		QString("CREATE TEMPORARY TABLE IF NOT EXISTS checked_directory (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, hash TEXT NOT NULL, path TEXT NOT NULL)") <<
		QString("CREATE TABLE IF NOT EXISTS master_files (hash TEXT NOT NULL PRIMARY KEY, path TEXT NOT NULL)") <<
		QString("CREATE TEMPORARY TABLE IF NOT EXISTS checked_files (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, hash TEXT NOT NULL, path TEXT NOT NULL)")
);

QString SQL_INSERT_DIRECTORY = "INSERT INTO master_directory (hash, path) VALUES(?, ?)";
QString SQL_HAS_DIRECTORY_BY_HASH = "SELECT hash, path FROM master_directory WHERE hash = ?";

QString SQL_CHECK_INSERT_DIRECTORY = "INSERT INTO checked_directory (hash, path) VALUES (?, ?)";
QString SQL_NEW_DIRECTORIES = "SELECT hash, path FROM checked_directory WHERE hash not in (SELECT hash FROM master_directory)";
QString SQL_MISSING_DIRECTORIES = "SELECT hash, path FROM master_directory WHERE hash NOT IN (SELECT hash FROM check_directory)";


QString SQL_INSERT_FILE = "INSERT INTO master_files (hash, path) VALUES(?, ?)";
QString SQL_HAS_FILE_BY_HASH = "SELECT hash, path FROM master_files WHERE hash = ?";

QString SQL_CHECK_INSERT_FILE = "INSERT INTO checked_files (hash, path) VALUES(?, ?)";
QString SQL_NEW_FILES = "SELECT hash, path FROM checked_files WHERE hash NOT IN (SELECT hash FROM master_files)";
QString SQL_MISSING_FILES = "SELECT hash, path FROM master_files WHERE hash NOT IN (SELECT hash FROM checked_files)";

HashDb::HashDb(QObject* _parent) : QObject(_parent)
	{
	db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName(":memory:");
	db.open();
	}

HashDb::~HashDb()
	{
	}

void HashDb::init_database()
	{
	if (db.isOpen())
		{
		for (QStringList::iterator iter = schemas.begin();
			 iter != schemas.end();
			 ++iter)
			 {
			 QSqlQuery setup(db);
			 setup.prepare((*iter));
			 if (!setup.exec())
			 	{
				qDebug() << "Failed to create table...";
				qDebug() << "Query: " << setup.executedQuery();
				qDebug() << "Error: " << setup.lastError();
				}
			setup.clear();
			 // db.exec((*iter));
			 }
		 }
	}

void HashDb::addDirectory(QString _path, QByteArray _hash)
	{
	if (db.isOpen())
		{
		QSqlQuery insertion(db);
		if (generation)
			{
			insertion.prepare(SQL_INSERT_DIRECTORY);
			}
		else
			{
			insertion.prepare(SQL_CHECK_INSERT_DIRECTORY);
			}
		insertion.bindValue(":hash", _hash);
		insertion.bindValue(":path", _path);
		if (insertion.exec())
			{
			db.commit();
			}
		else
			{
			qDebug() << "Failed to insert directory " << _path << " with hash " << _hash << " into database";
			}
		}
	}
void HashDb::addFile(QString _path, QByteArray _hash)
	{
	if (db.isOpen())
		{
		db.transaction();
		QSqlQuery insertion(db);
		if (generation)
			{
			qDebug() << "insertion for creation...";
			insertion.prepare(SQL_INSERT_FILE);
			}
		else
			{
			qDebug() << "insertion for validation...";
			insertion.prepare(SQL_CHECK_INSERT_FILE);
			}
		insertion.bindValue(0, _hash);
		insertion.bindValue(1, _path);
		if (insertion.exec())
			{
			db.commit();
			}
		else
			{
			qDebug() << "Failed to insert file " << _path << " with hash " << _hash << " into database";
			qDebug() << "Query: " << insertion.executedQuery();
			QList<QVariant> bv = insertion.boundValues().values();
			for (int i = 0; i < bv.size(); ++i)
				{
				qDebug() << "Mapped - Index: " << i << ", Value: " << bv[i];
				}
			/*
			QMapIterator<QString, QVariant> bv = insertion.boundValues();
			while(bv.hasNext())
				{
				bv.next();
				qDebug() << "Mapped - Key: " << bv.key() << ", Value: "<< bv.value();
				}
			*/

			qDebug() << "Error: " << insertion.lastError();
			}
		insertion.clear();
		}
	}
void HashDb::setMode(bool _generate)
	{
	generation = _generate;
	}
void HashDb::generateMissingObjects()
	{
	}
void HashDb::generateNewObjects()
	{
	}
