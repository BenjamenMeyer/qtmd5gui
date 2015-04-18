#ifndef VERIFIER_H__
#define VERIFIER_H__

#include <QtCore/QString>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QTextEdit>
#include <QtGui/QWidget>

class Verifier: public QWidget
	{
	Q_OBJECT
	public:
		Verifier(QWidget* _parent);
		virtual ~Verifier();

	public slots:
	signals:
		void startHashing(QString _path);
		void cancelHashing();

	protected:
		void createLayout();


	protected slots:
		void doSelectPath();
		void doAction();

	private:
		QLabel* labelPath;
		QPushButton* buttonPathSelector;
		QPushButton* buttonAction;
		QTextEdit* labelLog;
	
	private slots:
	};

#endif //VERIFIER_H__
