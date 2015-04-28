#include <hash_coordinator.h>

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QTimer>
#include <QtCore/QString>

HashCoordinator::HashCoordinator(QObject* _parent) : cancelled(false), generate(true)
	{
	connect(this, SIGNAL(processDirectory(QString, bool)),
	        this, SLOT(startHashing(QString, bool)),
			Qt::QueuedConnection);
	connect(this, SIGNAL(cancelHashing()),
	        this, SLOT(receive_cancelHashing()));
	connect(this, SIGNAL(resetHashing()),
	        this, SLOT(receive_resetHashing()));
	connect(&db, SIGNAL(message(QString)),
	        this, SIGNAL(message(QString)));
	
	for(unsigned int i = 0; i < HASHER_THREAD_COUNT; ++i)
		{
		hashers[i].hasher.setModulo(i);
		connect(this, SIGNAL(cancelHashing()),
				&(hashers[i].hasher), SLOT(receive_cancelHashing()));
		connect(this, SIGNAL(resetHashing()),
				&(hashers[i].hasher), SLOT(receive_resetHashing()));
		connect(this, SIGNAL(processFile(int, QString, bool)),
		        &(hashers[i].hasher), SLOT(processFile(int, QString, bool)));
		connect(&(hashers[i].hasher), SIGNAL(fileData(QString, QByteArray, bool)),
		        this, SLOT(receive_hash(QString, QByteArray, bool)));

		hashers[i].hasher.moveToThread(&(hashers[i].thread));
		hashers[i].thread.start();
		}

	for (unsigned int i = 0; i < COPIER_THREAD_COUNT; ++i)
		{
		copiers[i].copier.setModulo(i);
		connect(this, SIGNAL(copyFile(int, QString, QString)),
		        &(copiers[i].copier), SLOT(copyFile(int, QString, QString)));
		copiers[i].copier.moveToThread(&(copiers[i].thread));
		copiers[i].thread.start();
		}
	}
HashCoordinator::~HashCoordinator()
	{
	qDebug() << "Stopping Hashers";
	for(unsigned int i = 0; i < HASHER_THREAD_COUNT; ++i)
		{
		hashers[i].thread.quit();
		hashers[i].thread.wait();
		}
	qDebug() << "Stopping Copiers";
	for (unsigned int i = 0; i < COPIER_THREAD_COUNT; ++i)
		{
		copiers[i].thread.quit();
		copiers[i].thread.wait();
		}
	qDebug() << "All tasks stopped";
	}

void HashCoordinator::startHashing(QString _path)
	{
	Q_EMIT hashing_started();
	hash_directory(_path);
	Q_EMIT hashing_pending();
	}

void HashCoordinator::startHashing(QString _path, bool _mode)
	{
	if (cancelled)
		{
		qDebug() << "Cancelling on Path: " << _path;
		return;
		}
	hash_directory(_path);
	}

void HashCoordinator::hash_directory(QString _path)
	{
	qDebug() << "Processing Path: " << _path;

	QDir directories(_path);
	QDir files(_path);

	directories.setFilter(QDir::Dirs|
							QDir::NoDotAndDotDot|
							QDir::Hidden|QDir::System);
	directories.setSorting(QDir::Name);
	QStringList dirList = directories.entryList();
	for (QStringList::iterator iter = dirList.begin(); iter != dirList.end(); ++iter)
		{
		QString full_path_to_check = _path + directories.separator() + (*iter);
		qDebug() << "Found sub-directory: " << (*iter) << " -> " << full_path_to_check;
		Q_EMIT processDirectory(full_path_to_check, generate);
		}

	int hash_index = 0;
	files.setFilter(QDir::Files|
					QDir::Hidden|QDir::System|
					QDir::NoSymLinks);
	files.setSorting(QDir::Name);
	QStringList fileList = files.entryList();
	for (QStringList::iterator iter = fileList.begin(); iter != fileList.end(); ++iter)
		{
		QString full_file_to_check = _path + directories.separator() + (*iter);
		qDebug() << "Found file: " << (*iter) << " -> " << full_file_to_check;
		Q_EMIT processFile(hash_index, full_file_to_check, generate);

		hash_index = (hash_index + 1) % HASHER_THREAD_COUNT;
		}
	}

void HashCoordinator::receive_cancelHashing()
	{
	qDebug() << "Cancel Hashing";
	cancelled = true;
	}

void HashCoordinator::receive_resetHashing()
	{
	qDebug() << "Reset Hashing";
	cancelled = false;
	}

void HashCoordinator::changeMode(bool _generate)
	{
	qDebug() << "Generate changing from " << generate << " to " << _generate;
	generate = _generate;
	}

void HashCoordinator::receive_hash(QString _path, QByteArray _hash_value, bool _generate)
	{
	qDebug() << "Received Hash of " << _hash_value << "on file" << _path << " - generate: " << _generate;
	db.addFile(_path, _hash_value, _generate);
	}

void HashCoordinator::getMissing()
	{
	}

void HashCoordinator::getNew()
	{
	}

void HashCoordinator::copyMissing()
	{
	}

void HashCoordinator::resetDatabase()
	{
	}
