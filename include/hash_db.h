#ifndef HASH_DB_H__
#define HASH_DB_H__

#include <QtCore/QObject>
#include <QtSql/QSqlDatabase>

class HashDb : public QObject
	{
	Q_OBJECT
	public:
		HashDb(QObject* _parent=NULL);
		~HashDb();

	public Q_SLOTS:
		void addDirectory(QString _path);
		void addFile(int _module, QString _path);

		void setMode(bool _generate);
		void generateMissingObjects();
		void generateNewObjects();

	Q_SIGNALS:
	protected:
		QSqlDatabase db;
		bool generation;

	protected Q_SLOTS:
	private:
	private Q_SLOTS:
	};

#endif //HASH_DB_H__
