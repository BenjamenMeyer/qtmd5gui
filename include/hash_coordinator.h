#ifndef HASH_COORDINATOR_H__
#define HASH_COORDINATOR_H__

#include <QtCore/QObject>
#include <QtCore/QThread>

#include <file_hasher.h>
#include <hash_db.h>

#define HASHER_THREAD_COUNT		10

class HashCoordinator : public QObject
	{
	Q_OBJECT
	public:
		HashCoordinator(QObject* _parent=NULL);
		~HashCoordinator();

	public Q_SLOTS:
		void startHashing(QString _path);

	Q_SIGNALS:
		void processDirectory(QString _path);
		void processFile(int _modulo, QString _path);
		void cancelHashing();
		void resetHashing();

	protected:
		bool cancelled;

	protected Q_SLOTS:
		void receive_hash(QString _path, QByteArray _hash_value);
		void receive_cancelHashing();
		void receive_resetHashing();

	private:
		struct ht
			{
			FileHasher hasher;
			QThread thread;
			};
		struct ht hashers[HASHER_THREAD_COUNT];
		HashDb db;

	private Q_SLOTS:
	};

#endif //HASH_COORDINATOR_H__
