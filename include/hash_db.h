#ifndef HASH_DB_H__
#define HASH_DB_H__

#include <QtCore/QObject>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

class HashDb : public QObject
	{
	Q_OBJECT
	public:
		HashDb(QObject* _parent=NULL);
		~HashDb();

	public Q_SLOTS:
		void addDirectory(QString _path, QByteArray _hash);
		void addFile(QString _path, QByteArray _hash);

		void setMode(bool _generate);
		void generateMissingObjects();
		void generateNewObjects();

	Q_SIGNALS:
	protected:
		QSqlDatabase db;
		bool generation;

		QSqlQuery SQL_INSERT_DIRECTORY;
		QSqlQuery SQL_HAS_DIRECTORY_BY_HASH;
		QSqlQuery SQL_CHECK_INSERT_DIRECTORY;
		QSqlQuery SQL_NEW_DIRECTORIES;
		QSqlQuery SQL_MISSING_DIRECTORIES;

		QSqlQuery SQL_INSERT_FILE;
		QSqlQuery SQL_HAS_FILE_BY_HASH;
		QSqlQuery SQL_CHECK_INSERT_FILE;
		QSqlQuery SQL_NEW_FILES;
		QSqlQuery SQL_MISSING_FILES;

		void init_database();

	protected Q_SLOTS:
	private:
	private Q_SLOTS:
	};

#endif //HASH_DB_H__
