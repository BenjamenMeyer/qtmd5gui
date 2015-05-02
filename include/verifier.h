#ifndef VERIFIER_H__
#define VERIFIER_H__

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QThread>
#include <QtGui/QCheckBox>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QTextEdit>
#include <QtGui/QWidget>

#include <hash_coordinator.h>
#include <logger.h>

class Verifier: public QWidget
	{
	Q_OBJECT
	public:
		Verifier(QWidget* _parent);
		virtual ~Verifier();

	public Q_SLOTS:
		void logMessage(QtMsgType _type, QString _msg);

	Q_SIGNALS:
		void startHashing(QString _path, bool _generate);
		void resetHashing();

		void getMissing();
		void getNew();
		void copyMissing(QString _source_path, QString _destination_path);

		void resetDatabase();
		void clearResultDisplay();

		void send_message(QString _message);

	protected:
		void createLayout();

		void doCheckState();

	protected Q_SLOTS:
		void doSelectSource();
		void doSelectDestination();
		void doCompare();
		void doGetResults();
		void doCopy();

		void receive_info(QString _message);
		void receive_missing(QString _message);
		void receive_new(QString _new);

	private:
		QGroupBox* boxSource;
		QLabel* labelSourcePath;
		QPushButton* buttonSourceSelect;
		QTextEdit* editSource;

		QGroupBox* boxDestination;
		QLabel* labelDestinationPath;
		QPushButton* buttonDestinationSelect;
		QTextEdit* editDestination;

		QPushButton* buttonCompare;
		QPushButton* buttonGetResults;
		QPushButton* buttonCopy;
		QPushButton* buttonReset;

		QTextEdit* labelLog;
		
		HashCoordinator hasher;
		QThread hashThread;

		Logger theLog;
		QThread logThread;
	private Q_SLOTS:
	};

#endif //VERIFIER_H__
