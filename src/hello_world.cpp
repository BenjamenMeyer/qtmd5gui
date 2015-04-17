#include <QApplication>
#include <QtGui/QPushButton>

int main(int argc, char* argv[])
	{
	QApplication theApplication(argc, argv);

	QPushButton hello("Hello");
	hello.show();

	return theApplication.exec();
	}
