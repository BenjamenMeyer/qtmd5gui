#include <hash_coordinator.h>

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QTimer>
#include <QtCore/QString>

HashCoordinator::HashCoordinator(QObject* _parent) : cancelled(false)
	{
	connect(this, SIGNAL(processDirectory(QString)),
	        this, SLOT(startHashing(QString)),
			Qt::QueuedConnection);
	connect(this, SIGNAL(cancelHashing()),
	        this, SLOT(receive_cancelHashing()));
	connect(this, SIGNAL(resetHashing()),
	        this, SLOT(receive_resetHashing()));
	
	for(unsigned int i = 0; i < HASHER_THREAD_COUNT; ++i)
		{
		hashers[i].hasher.setModulo(i);
		connect(this, SIGNAL(cancelHashing()),
				&(hashers[i].hasher), SLOT(receive_cancelHashing()));
		connect(this, SIGNAL(resetHashing()),
				&(hashers[i].hasher), SLOT(receive_resetHashing()));
		connect(this, SIGNAL(processFile(int, QString)),
		        &(hashers[i].hasher), SLOT(processFile(int, QString)));
		connect(&(hashers[i].hasher), SIGNAL(fileData(QString, QByteArray)),
		        this, SLOT(receive_hash(QString, QByteArray)));
		hashers[i].hasher.moveToThread(&(hashers[i].thread));
		hashers[i].thread.start();
		}
	}
HashCoordinator::~HashCoordinator()
	{
	
	for(unsigned int i = 0; i < HASHER_THREAD_COUNT; ++i)
		{
		hashers[i].thread.quit();
		hashers[i].thread.wait();
		}
	}

void HashCoordinator::startHashing(QString _path)
	{
	if (cancelled)
		{
		qDebug() << "Cancelling on Path: " << _path;
		return;
		}

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
		Q_EMIT processDirectory(full_path_to_check);
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
		Q_EMIT processFile(hash_index, full_file_to_check);

		hash_index = (hash_index + 1) % HASHER_THREAD_COUNT;
		}
	}

void HashCoordinator::receive_cancelHashing()
	{
	cancelled = true;
	}

void HashCoordinator::receive_resetHashing()
	{
	cancelled = false;
	}

void HashCoordinator::receive_hash(QString _path, QByteArray _hash_value)
	{
	qDebug() << "Received Hash of " << _hash_value << "on file" << _path;
	}
