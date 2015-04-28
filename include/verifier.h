#ifndef VERIFIER_H__
#define VERIFIER_H__

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QThread>
#include <QtGui/QCheckBox>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QTextEdit>
#include <QtGui/QWidget>

#include <hash_coordinator.h>

class Verifier: public QWidget
	{
	Q_OBJECT
	public:
		Verifier(QWidget* _parent);
		virtual ~Verifier();

	public Q_SLOTS:
	Q_SIGNALS:
		void startHashing(QString _path);
		void cancelHashing();
		void resetHashing();
		void changeMode(bool _generate);

		void getMissing();
		void getNew();
		void copyMissing();

		void resetDatabase();

	protected:
		void createLayout();


	protected Q_SLOTS:
		void doSelectPath();
		void doAction();
		void doChangeMode(int _mode);

		void receive_info(QString _message);

	private:
		QLabel* labelPath;
		QPushButton* buttonPathSelector;
		QPushButton* buttonAction;
		QPushButton* buttonGetMissing;
		QPushButton* buttonGetNew;
		QPushButton* buttonCopyMissing;

		QTextEdit* labelLog;
		QCheckBox* checkGeneration;

		HashCoordinator hasher;
		QThread hashThread;
	private Q_SLOTS:
	};

#endif //VERIFIER_H__
