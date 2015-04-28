#include <verifier.h>

#include <QtCore/QtCore>
#include <QtCore/QDir>
#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtGui/QCheckBox>
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
	buttonGetMissing = NULL;
	buttonGetNew = NULL;
	buttonCopyMissing = NULL;
	labelLog = NULL;
	checkGeneration = NULL;

	setWindowTitle("Qt File Verifier");
	createLayout();

	connect(this, SIGNAL(startHashing(QString)),
	        &hasher, SLOT(startHashing(QString)));
	connect(this, SIGNAL(cancelHashing()),
	        &hasher, SIGNAL(cancelHashing()));
	connect(this, SIGNAL(resetHashing()),
	        &hasher, SIGNAL(resetHashing()));
	connect(this, SIGNAL(changeMode(bool)),
	        &hasher, SLOT(changeMode(bool)));

	connect(this, SIGNAL(getMissing()),
	        &hasher, SLOT(getMissing()));
	connect(this, SIGNAL(getNew()),
	        &hasher, SLOT(getNew()));
	connect(this, SIGNAL(copyMissing()),
	        &hasher, SLOT(copyMissing()));
	connect(this, SIGNAL(resetDatabase()),
	        &hasher, SLOT(resetDatabase()));
	connect(&hasher, SIGNAL(send_message(QString)),
	        this, SLOT(receive_info(QString)));

	hasher.moveToThread(&hashThread);
	hashThread.start();
	}

Verifier::~Verifier()
	{
	hashThread.quit();
	hashThread.wait();
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

		checkGeneration = new QCheckBox("Validate");
		if (checkGeneration != NULL)
			{
			checkGeneration->setChecked(false);
			connect(checkGeneration, SIGNAL(stateChanged(int)),
			        this, SLOT(doChangeMode(int)));
			masterLayout->addWidget(checkGeneration);
			}

		QHBoxLayout* commandLayout = new QHBoxLayout();
		if (commandLayout != NULL)
			{
			buttonAction = new QPushButton("Start");
			if (buttonAction != NULL)
				{
				buttonAction->setEnabled(false);
				connect(buttonAction, SIGNAL(clicked()),
						this, SLOT(doAction()));
				commandLayout->addWidget(buttonAction);
				}

			buttonGetMissing = new QPushButton("Report Missing");
			if (buttonGetMissing != NULL)
				{
				buttonGetMissing->setEnabled(false);
				connect(buttonGetMissing, SIGNAL(clicked()),
				        this, SIGNAL(getMissing()));
				commandLayout->addWidget(buttonGetMissing);
				}

			buttonGetNew = new QPushButton("Report New");
			if (buttonGetNew != NULL)
				{
				buttonGetNew->setEnabled(false);
				connect(buttonGetNew, SIGNAL(clicked()),
				        this, SIGNAL(getNew()));
				commandLayout->addWidget(buttonGetNew);
				}

			buttonCopyMissing = new QPushButton("Copy Missing");
			if (buttonCopyMissing != NULL)
				{
				buttonCopyMissing->setEnabled(false);
				connect(buttonCopyMissing, SIGNAL(clicked()),
				        this, SIGNAL(copyMissing()));
				}

			masterLayout->addLayout(commandLayout);
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

	bool generate = true;
	if (checkGeneration != NULL)
		{
		switch (checkGeneration->checkState())
			{
			case Qt::Unchecked:
				generate = false;
				break;

			default:
			case Qt::Checked:
				generate = true;
				break;
			};
		}

	if (currentStatus == "Start")
		{
		Q_EMIT startHashing(labelPath->text());
		buttonAction->setText("Stop");

		buttonGetMissing->setEnabled(false);
		buttonGetNew->setEnabled(false);
		buttonCopyMissing->setEnabled(false);
		}
	else if (currentStatus == "Stop")
		{
		Q_EMIT cancelHashing();
		buttonAction->setText("Reset");

		buttonGetMissing->setEnabled(false);
		buttonGetNew->setEnabled(false);
		buttonCopyMissing->setEnabled(false);
		}
	else
		{
		Q_EMIT resetHashing();
		buttonAction->setText("Start");

		buttonGetMissing->setEnabled(!generate);
		buttonGetNew->setEnabled(!generate);
		buttonCopyMissing->setEnabled(!generate);

		if (generate)
			{
			Q_EMIT resetDatabase();
			}
		}
	}

void Verifier::doChangeMode(int _state)
	{
	switch (_state)
		{
		case Qt::Unchecked:
			qDebug() << "Mode: Generate";
			Q_EMIT changeMode(true);
			break;

		default:
		case Qt::Checked:
			qDebug() << "Mode: Validate";
			Q_EMIT changeMode(false);
			break;
		};
	}

void Verifier::receive_info(QString _message)
	{
	if (labelLog != NULL)
		{
		labelLog->append(_message);
		}
	}
