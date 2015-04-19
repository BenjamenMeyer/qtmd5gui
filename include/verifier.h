#ifndef VERIFIER_H__
#define VERIFIER_H__

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QThread>
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

	protected:
		void createLayout();


	protected Q_SLOTS:
		void doSelectPath();
		void doAction();

	private:
		QLabel* labelPath;
		QPushButton* buttonPathSelector;
		QPushButton* buttonAction;
		QTextEdit* labelLog;

		HashCoordinator hasher;
		QThread hashThread;
	private Q_SLOTS:
	};

#endif //VERIFIER_H__
