#include <file_hasher.h>

#include <QtCore/QCryptographicHash>
#include <QtCore/QFile>

FileHasher::FileHasher(QObject* _parent) : QObject(_parent), modulo(0)
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
	modulo = _m;
	}

void FileHasher::processFile(int _modulo, QString _path)
	{
	if (cancelled)
		{
		return;
		}

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
		Q_EMIT fileData(_path, hash_value.toHex());
		}
	}
void FileHasher::receive_cancelHashing()
	{
	cancelled = true;
	}
void FileHasher::receive_resetHashing()
	{
	cancelled = false;
	}
