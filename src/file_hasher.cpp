#include <file_hasher.h>

#include <QtCore/QCryptographicHash>
#include <QtCore/QDebug>
#include <QtCore/QFile>

FileHasher::FileHasher(QObject* _parent) : QObject(_parent), modulo(0), cancelled(false)
	{
	}
FileHasher::~FileHasher()
	{
	}

int FileHasher::getModulo() const
	{
	return modulo;
	}
void FileHasher::setModulo(int _m)
	{
	qDebug() << " Hashing Thread [ " << modulo << "] -> " << _m;
	modulo = _m;
	qDebug() << " Hashing Thread [ " << modulo << "] configured";
	}

void FileHasher::processFile(int _modulo, QString _path, bool _generate)
	{
	qDebug() << "Hashing Thread [" << modulo << "] - Received (" << _modulo << ", " << _path << ", " << _generate << ")";
	if (cancelled)
		{
		qDebug() << "Hashing Thread [" << modulo << "] - all jobs cancelled; ignoring - (" << _modulo << ", " << _path << ", " << _generate << ")";
		return;
		}

	if (_modulo != modulo)
		{
		qDebug() << "Hashing Thread [" << modulo << "] - job not mine; ignoring - (" << _modulo << ", " << _path << ", " << _generate << ")";
		return;
		}

	qDebug() << "Hashing Thread [" << modulo << "] - job accepted - (" << _modulo << ", " << _path << ", " << _generate << ")";

	QCryptographicHash hasher(QCryptographicHash::Sha1);

	QFile theFile(_path);
	if (theFile.open(QIODevice::ReadOnly))
		{
		const qint64 read_length = 4096;
		// 4 byte aligntment, 4 bytes extra
		char buffer[read_length + 4];
		qint64 read_count;
		while (!theFile.atEnd())
			{
			// only read 4k at a time
			memset(buffer, 0, sizeof(buffer));
			read_count = theFile.read(buffer, read_length);
			if (read_count > 0)
				{
				hasher.addData(buffer, read_count);
				}
			}

		QByteArray hash_value = hasher.result();
		Q_EMIT fileData(_path, hash_value.toHex(), _generate);
		}

	qDebug() << "Hashing Thread [" << modulo << "] - job completed - (" << _modulo << ", " << _path << ", " << _generate << ")";
	}
void FileHasher::receive_cancelHashing()
	{
	cancelled = true;
	}
void FileHasher::receive_resetHashing()
	{
	cancelled = false;
	}
