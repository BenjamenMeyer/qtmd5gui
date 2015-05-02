#include <hash_db.h>

#include <QtCore/QDebug>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtSql/QSqlRecord>

QStringList schemas = (
	QStringList() <<
		QString("CREATE TABLE IF NOT EXISTS master_directory (hash TEXT NOT NULL PRIMARY KEY, path TEXT NOT NULL)") <<
		QString("CREATE TABLE IF NOT EXISTS checked_directory (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, hash TEXT NOT NULL, path TEXT NOT NULL)") <<
		QString("CREATE TABLE IF NOT EXISTS master_files (hash TEXT NOT NULL PRIMARY KEY, path TEXT NOT NULL)") <<
		QString("CREATE TABLE IF NOT EXISTS checked_files (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, hash TEXT NOT NULL, path TEXT NOT NULL)")
);

QStringList resetSchemas = (
	QStringList() <<
		QString("DROP TABLE IF EXISTS checked_files") <<
		QString("DROP TABLE IF EXISTS master_files") <<
		QString("DROP TABLE IF EXISTS checked_directory") <<
		QString("DROP TABLE IF EXISTS master_directory")
);

#define MAKE_QT_SQL_STATEMENT(name, the_db, the_statement)					\
	name = QSqlQuery(the_db); 												\
    if (!name.prepare(the_statement))										\
		{																	\
		qDebug() << "Failed to prepare " << the_statement;					\
		qDebug() << "Error: " << name.lastError();							\
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
	SQL_NEW_DIRECTORIES.setForwardOnly(true);

	MAKE_QT_SQL_STATEMENT(SQL_MISSING_DIRECTORIES, db, "SELECT hash, path FROM master_directory WHERE hash NOT IN (SELECT hash FROM checked_directory)");
	SQL_MISSING_DIRECTORIES.setForwardOnly(true);

	MAKE_QT_SQL_STATEMENT(SQL_NEW_FILES, db, "SELECT hash, path FROM checked_files WHERE hash NOT IN (SELECT hash FROM master_files)");
	SQL_NEW_FILES.setForwardOnly(true);

	MAKE_QT_SQL_STATEMENT(SQL_MISSING_FILES, db, "SELECT hash, path FROM master_files WHERE hash NOT IN (SELECT hash FROM checked_files)");
	SQL_MISSING_FILES.setForwardOnly(true);
	}

HashDb::~HashDb()
	{
	db.close();
	}

void HashDb::init_database()
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

				qDebug() << "Failed to drop table...";
				qDebug() << "Query: " << setup.executedQuery();
				qDebug() << "Error: " << setup.lastError();
				}
	  		 setup.clear();
			 db.commit();
			 }
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

				qDebug() << "Failed to create table...";
				qDebug() << "Query: " << setup.executedQuery();
				qDebug() << "Error: " << setup.lastError();
				}
			 setup.clear();
			 db.commit();
			 }
		 }
	}
void HashDb::resetDatabase()
	{
	Q_EMIT message("Resetting database...");
	init_database();
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
			db.rollback();
			}
		}
	}
void HashDb::generateMissingObjects()
	{
	if (db.isOpen())
		{
		int col_path = 1;
		if (SQL_MISSING_FILES.exec())
			{
			while (SQL_MISSING_FILES.next())
				{
				QString path = SQL_MISSING_FILES.value(col_path).toString();
				Q_EMIT message_missing(path);
				}
			}
		else
			{
			Q_EMIT message(QString("Failed to retrieve missing records from the database"));
			Q_EMIT message(QString("Query: %1").arg(SQL_MISSING_FILES.executedQuery()));
			Q_EMIT message(QString("Error: %1").arg(SQL_MISSING_FILES.lastError().text()));
			}
		}
	}
void HashDb::generateNewObjects()
	{
	if (db.isOpen())
		{
		int col_path = 1;
		if (SQL_NEW_FILES.exec())
			{
			while (SQL_NEW_FILES.next())
				{
				QString path = SQL_NEW_FILES.value(col_path).toString();
				Q_EMIT message_new(path);
				}
			}
		else
			{
			Q_EMIT message(QString("Failed to retrieve new records from the database"));
			Q_EMIT message(QString("Query: %1").arg(SQL_NEW_FILES.executedQuery()));
			Q_EMIT message(QString("Error: %1").arg(SQL_NEW_FILES.lastError().text()));
			}
		}
	}
void HashDb::copyMissingObjects()
	{
	}
