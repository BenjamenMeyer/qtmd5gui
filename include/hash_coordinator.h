#ifndef HASH_COORDINATOR_H__
#define HASH_COORDINATOR_H__

#include <QtCore/QObject>
#include <QtCore/QThread>

#include <file_copier.h>
#include <file_hasher.h>
#include <hash_db.h>

#define COPIER_THREAD_COUNT		20
#define HASHER_THREAD_COUNT		10

class HashCoordinator : public QObject
	{
	Q_OBJECT
	public:
		HashCoordinator(QObject* _parent=NULL);
		~HashCoordinator();

	public Q_SLOTS:
		void startHashing(QString _path);
		void changeMode(bool _generate);

		void getMissing();
		void getNew();
		void copyMissing();

		void resetDatabase();

	Q_SIGNALS:
		void processDirectory(QString _path, bool _mode);
		void processFile(int _modulo, QString _path, bool _mode);
		void copyFile(int _modulo, QString _source, QString _destination);
		void cancelHashing();
		void resetHashing();

		void hashing_started();
		void hashing_pending();

		void send_message(QString _message);

	protected:
		bool cancelled;

	protected Q_SLOTS:
		void startHashing(QString _path, bool _mode);

		void receive_hash(QString _path, QByteArray _hash_value, bool _generate);
		void receive_cancelHashing();
		void receive_resetHashing();

		void hash_directory(QString _path);

	private:
		struct ht
			{
			FileHasher hasher;
			QThread thread;
			};
		struct ht hashers[HASHER_THREAD_COUNT];

		struct hc
			{
			FileCopier copier;
			QThread thread;
			};
		struct hc copiers[COPIER_THREAD_COUNT];

		HashDb db;
		bool generate;

	private Q_SLOTS:
	};

#endif //HASH_COORDINATOR_H__
