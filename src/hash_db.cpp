#include <hash_db.h>

#include <QtCore/QDebug>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

//#define BIND_BY_NAME 1
#define BIND_BY_NAME 0

QStringList schemas = (
		QStringList() <<
		QString("CREATE TABLE IF NOT EXISTS master_directory (hash TEXT NOT NULL PRIMARY KEY, path TEXT NOT NULL)") <<
		QString("CREATE TEMPORARY TABLE IF NOT EXISTS checked_directory (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, hash TEXT NOT NULL, path TEXT NOT NULL)") <<
		QString("CREATE TABLE IF NOT EXISTS master_files (hash TEXT NOT NULL PRIMARY KEY, path TEXT NOT NULL)") <<
		QString("CREATE TEMPORARY TABLE IF NOT EXISTS checked_files (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, hash TEXT NOT NULL, path TEXT NOT NULL)")
);

#define MAKE_QT_SQL_STATEMENT(name, the_db, the_statement) \
	name = QSqlQuery(the_db); \
    name.prepare(the_statement)

HashDb::HashDb(QObject* _parent) : QObject(_parent)
	{
	db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName(":memory:");
	db.open();
	#if BIND_BY_NAME
		MAKE_QT_SQL_STATEMENT(SQL_INSERT_DIRECTORY, db, "INSERT INTO master_directory (hash, path) VALUES(:hash, :path)");
		MAKE_QT_SQL_STATEMENT(SQL_HAS_DIRECTORY_BY_HASH, db, "SELECT hash, path FROM master_directory WHERE hash = :hash");
		MAKE_QT_SQL_STATEMENT(SQL_CHECK_INSERT_DIRECTORY, db, "INSERT INTO checked_directory (hash, path) VALUES (:hash, :path)");

		MAKE_QT_SQL_STATEMENT(SQL_INSERT_FILE, db, "INSERT INTO master_files (hash, path) VALUES(:hash, :path)");
		MAKE_QT_SQL_STATEMENT(SQL_HAS_FILE_BY_HASH, db, "SELECT hash, path FROM master_files WHERE hash = :hash");
		MAKE_QT_SQL_STATEMENT(SQL_CHECK_INSERT_FILE, db, "INSERT INTO checked_files (hash, path) VALUES(:hash, :path)");
	#else
		MAKE_QT_SQL_STATEMENT(SQL_INSERT_DIRECTORY, db, "INSERT INTO master_directory (hash, path) VALUES(?, ?)");
		MAKE_QT_SQL_STATEMENT(SQL_HAS_DIRECTORY_BY_HASH, db, "SELECT hash, path FROM master_directory WHERE hash = ?");
		MAKE_QT_SQL_STATEMENT(SQL_CHECK_INSERT_DIRECTORY, db, "INSERT INTO checked_directory (hash, path) VALUES (?, ?)");

		MAKE_QT_SQL_STATEMENT(SQL_INSERT_FILE, db, "INSERT INTO master_files (hash, path) VALUES(?, ?)");
		MAKE_QT_SQL_STATEMENT(SQL_HAS_FILE_BY_HASH, db, "SELECT hash, path FROM master_files WHERE hash = ?");
		MAKE_QT_SQL_STATEMENT(SQL_CHECK_INSERT_FILE, db, "INSERT INTO checked_files (hash, path) VALUES(?, ?)");
	#endif

	MAKE_QT_SQL_STATEMENT(SQL_NEW_DIRECTORIES, db, "SELECT hash, path FROM checked_directory WHERE hash not in (SELECT hash FROM master_directory)");
	MAKE_QT_SQL_STATEMENT(SQL_MISSING_DIRECTORIES, db, "SELECT hash, path FROM master_directory WHERE hash NOT IN (SELECT hash FROM check_directory)");

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
		QSqlQuery insertion;
		if (generation)
			{
			//insertion.prepare(SQL_INSERT_DIRECTORY);
			insertion = SQL_INSERT_DIRECTORY;
			}
		else
			{
			//insertion.prepare(SQL_CHECK_INSERT_DIRECTORY);
			insertion = SQL_CHECK_INSERT_DIRECTORY;
			}
		#if BIND_BY_NAME
			insertion.bindValue(":hash", _hash);
			insertion.bindValue(":path", _path);
		#else
			insertion.bindValue(0, _hash);
			insertion.bindValue(1, _path);
		#endif
		if (insertion.exec())
			{
			db.commit();
			}
		else
			{
			qDebug() << "Failed to insert directory " << _path << " with hash " << _hash << " into database";
			qDebug() << "Query: " << insertion.executedQuery();
			#if BIND_BY_NAME
				QMapIterator<QString, QVariant> bv = insertion.boundValues();
				while(bv.hasNext())
					{
					bv.next();
					qDebug() << "Mapped - Key: " << bv.key() << ", Value: "<< bv.value();
					}
			#else
				QList<QVariant> bv = insertion.boundValues().values();
				for (int i = 0; i < bv.size(); ++i)
					{
					qDebug() << "Mapped - Index: " << i << ", Value: " << bv[i];
					}
			#endif
			qDebug() << "Error: " << insertion.lastError();
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
			//insertion.prepare(SQL_INSERT_FILE);
			insertion = SQL_INSERT_FILE;
			}
		else
			{
			qDebug() << "insertion for validation...";
			//insertion.prepare(SQL_CHECK_INSERT_FILE);
			insertion = SQL_CHECK_INSERT_FILE;
			}
		#if BIND_BY_NAME
			insertion.bindValue(":hash", _hash);
			insertion.bindValue(":path", _path);
		#else
			insertion.bindValue(0, _hash);
			insertion.bindValue(1, _path);
		#endif
		if (insertion.exec())
			{
			db.commit();
			}
		else
			{
			qDebug() << "Failed to insert file " << _path << " with hash " << _hash << " into database";
			qDebug() << "Query: " << insertion.executedQuery();
			#if BIND_BY_NAME
				QMapIterator<QString, QVariant> bv = insertion.boundValues();
				while(bv.hasNext())
					{
					bv.next();
					qDebug() << "Mapped - Key: " << bv.key() << ", Value: "<< bv.value();
					}
			#else
				QList<QVariant> bv = insertion.boundValues().values();
				for (int i = 0; i < bv.size(); ++i)
					{
					qDebug() << "Mapped - Index: " << i << ", Value: " << bv[i];
					}
			#endif
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
