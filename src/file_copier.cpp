#include <file_copier.h>

#include <QtCore/QCryptographicHash>
#include <QtCore/QFile>

FileCopier::FileCopier(QObject* _parent) : QObject(_parent), modulo(0)
	{
	}
FileCopier::~FileCopier()
	{
	}

int FileCopier::getModulo() const
	{
	return modulo;
	}
void FileCopier::setModulo(int _m)
	{
	modulo = _m;
	}

void FileCopier::copyFile(int _modulo, QString _source, QString _destination)
	{
	if (_modulo != modulo)
		{
		return;
		}
	QFile::copy(_source, _destination);
	}
