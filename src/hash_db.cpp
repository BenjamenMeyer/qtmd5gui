#include <hash_db.h>

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

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

QStringList resetSchemas = (
	QStringList() <<
		QString("DROP TABLE checked_files") <<
		QString("DROP TABLE master_files") <<
		QString("DROP TABLE checked_directory") <<
		QString("DROP TABLE master_directory")
);

#define MAKE_QT_SQL_STATEMENT(name, the_db, the_statement)					\
	name = QSqlQuery(the_db); 												\
    if (!name.prepare(the_statement))										\
		{																	\
		Q_EMIT message(QString("Failed to prepare %1").arg(the_statement));	\
		Q_EMIT message(QString("Error: %1").arg(name.lastError().text()));	\
		}

HashDb::HashDb(QObject* _parent) : QObject(_parent)
	{
	db = QSqlDatabase::addDatabase("QSQLITE");
	// db.setDatabaseName(":memory:");
	db.setDatabaseName("test_db.sqlite");
	db.open();
	init_database();
	MAKE_QT_SQL_STATEMENT(SQL_INSERT_DIRECTORY, db, "INSERT INTO master_directory (hash, path) VALUES(:hash, :path);");
	MAKE_QT_SQL_STATEMENT(SQL_HAS_DIRECTORY_BY_HASH, db, "SELECT hash, path FROM master_directory WHERE hash = :hash;");
	MAKE_QT_SQL_STATEMENT(SQL_CHECK_INSERT_DIRECTORY, db, "INSERT INTO checked_directory (hash, path) VALUES (:hash, :path);");

	MAKE_QT_SQL_STATEMENT(SQL_INSERT_FILE, db, "INSERT INTO master_files (hash, path) VALUES(:hash, :path);");
	MAKE_QT_SQL_STATEMENT(SQL_HAS_FILE_BY_HASH, db, "SELECT hash, path FROM master_files WHERE hash = :hash;");
	MAKE_QT_SQL_STATEMENT(SQL_CHECK_INSERT_FILE, db, "INSERT INTO checked_files (hash, path) VALUES(:hash, :path);");

	MAKE_QT_SQL_STATEMENT(SQL_NEW_DIRECTORIES, db, "SELECT hash, path FROM checked_directory WHERE hash NOT IN (SELECT hash FROM master_directory)");
	MAKE_QT_SQL_STATEMENT(SQL_MISSING_DIRECTORIES, db, "SELECT hash, path FROM master_directory WHERE hash NOT IN (SELECT hash FROM checked_directory)");

	MAKE_QT_SQL_STATEMENT(SQL_NEW_FILES, db, "SELECT hash, path FROM checked_files WHERE hash NOT IN (SELECT hash FROM master_files)");
	MAKE_QT_SQL_STATEMENT(SQL_MISSING_FILES, db, "SELECT hash, path FROM master_files WHERE hash NOT IN (SELECT hash FROM checked_files)");
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
				Q_EMIT message("Failed to create table...");
				Q_EMIT message(QString("Query: %1").arg(setup.executedQuery()));
				Q_EMIT message(QString("Error: %1").arg(setup.lastError().text()));
				}
			 setup.clear();
			 db.commit();
			 }
		 }
	}
void HashDb::resetDatabase()
	{
	if (db.isOpen())
		{
		for (QStringList::iterator iter = resetSchemas.begin();
			 iter != resetSchemas.end();
			 ++iter)
			 {
			 QSqlQuery setup(db);
			 setup.prepare((*iter));
			 if (!setup.exec())
			 	{
				Q_EMIT message("Failed to drop table...");
				Q_EMIT message(QString("Query: %1").arg(setup.executedQuery()));
				Q_EMIT message(QString("Error: %1").arg(setup.lastError().text()));
				}
	  		 setup.clear();
			 db.commit();
			 }
		 }
	}

void HashDb::addDirectory(QString _path, QByteArray _hash, bool _generate)
	{
	if (db.isOpen())
		{
		QSqlQuery insertion;
		if (_generate)
			{
			Q_EMIT message("Mode: Generate");
			insertion = SQL_INSERT_DIRECTORY;
			}
		else
			{
			Q_EMIT message("Mode: Validate");
			insertion = SQL_CHECK_INSERT_DIRECTORY;
			}
		insertion.bindValue(":hash", QVariant(_hash));
		insertion.bindValue(":path", _path);
		if (insertion.exec())
			{
			db.commit();
			}
		else
			{
			Q_EMIT message(QString("Failed to insert directory %1 with hash %2 into database").arg(_path).arg(QString(_hash)));
			Q_EMIT message(QString("Query: %1").arg(insertion.executedQuery()));
			QMapIterator<QString, QVariant> bv = insertion.boundValues();
			while(bv.hasNext())
				{
				bv.next();
				Q_EMIT message(QString("Mapped - Key: %1, Value: %2").arg(bv.key()).arg(bv.value().toString()));
				}
			Q_EMIT message(QString("Error: %1").arg(insertion.lastError().text()));
			}
		}
	}
void HashDb::addFile(QString _path, QByteArray _hash, bool _generate)
	{
	if (db.isOpen())
		{
		db.transaction();
		QSqlQuery insertion(db);
		if (_generate)
			{
			Q_EMIT message("Mode: Generate");
			insertion = SQL_INSERT_FILE;
			}
		else
			{
			Q_EMIT message("Mode: Validate");
			insertion = SQL_CHECK_INSERT_FILE;
			}
		insertion.bindValue(":hash", _hash);
		insertion.bindValue(":path", _path);
		if (insertion.exec())
			{
			db.commit();
			}
		else
			{
			Q_EMIT message(QString("Failed to insert file %1 with hash %2 into database").arg(_path).arg(QString(_hash)));
			Q_EMIT message(QString("Query: %1").arg(insertion.executedQuery()));
			QMapIterator<QString, QVariant> bv = insertion.boundValues();
			while(bv.hasNext())
				{
				bv.next();
				Q_EMIT message(QString("Mapped - Key: %1, Value: %2").arg(bv.key(), bv.value().toString()));
				}
			Q_EMIT message(QString("Error: %1").arg(insertion.lastError().text()));
			}
		}
	}
void HashDb::generateMissingObjects()
	{
	}
void HashDb::generateNewObjects()
	{
	}
void HashDb::copyMissingObjects()
	{
	}
