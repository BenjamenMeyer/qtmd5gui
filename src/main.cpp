#include <QApplication>

#include <verifier.h>

int main(int argc, char* argv[])
	{
	QApplication theApplication(argc, argv);
	Verifier main(NULL);
	main.show();
	return theApplication.exec();
	}
