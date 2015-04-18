#include <verifier.h>

#include <QtCore/QDir>
#include <QtCore/QString>
#include <QtGui/QFileDialog>
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>


Verifier::Verifier(QWidget* _parent) : QWidget(_parent, 0)
	{
	labelPath = NULL;
	buttonPathSelector = NULL;
	buttonAction = NULL;
	labelLog = NULL;

	setWindowTitle("Qt File Verifier");
	createLayout();
	}

Verifier::~Verifier()
	{
	}

void Verifier::createLayout()
	{
	QVBoxLayout* masterLayout = new QVBoxLayout();
	if (masterLayout != NULL)
		{
		QHBoxLayout* pathLayout = new QHBoxLayout();
		if (pathLayout != NULL)
			{
			labelPath = new QLabel("");
			if (labelPath != NULL)
				{
				labelPath->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
				labelPath->setFrameStyle(15);
				labelPath->setFrameShadow(QFrame::Sunken);
				pathLayout->addWidget(labelPath);
				}

			buttonPathSelector = new QPushButton("Select Path");
			if (buttonPathSelector != NULL)
				{
				connect(buttonPathSelector, SIGNAL(clicked()),
				        this, SLOT(doSelectPath()));
				pathLayout->addWidget(buttonPathSelector);
				}

			masterLayout->addLayout(pathLayout);
			}

		buttonAction = new QPushButton("Start");
		if (buttonAction != NULL)
			{
			buttonAction->setEnabled(false);
			connect(buttonAction, SIGNAL(clicked()),
			        this, SLOT(doAction()));
			masterLayout->addWidget(buttonAction);
			}

		labelLog = new QTextEdit();
		if (labelLog != NULL)
			{
			labelLog->setReadOnly(true);
			masterLayout->addWidget(labelLog);
			}
		setLayout(masterLayout);
		}
	}

void Verifier::doSelectPath()
	{
	QString selectedDirectory = QFileDialog::getExistingDirectory(this, "Select directory", QDir::currentPath(), QFileDialog::ShowDirsOnly);
	labelPath->setText(selectedDirectory);
	buttonAction->setEnabled(!selectedDirectory.isEmpty());
	}

void Verifier::doAction()
	{
	QString currentStatus = buttonAction->text();

	if (currentStatus == "Start")
		{
		emit startHashing(labelPath->text());
		buttonAction->setText("Stop");
		}
	else
		{
		Q_EMIT cancelHashing();
		buttonAction->setText("Start");
		}
	}
