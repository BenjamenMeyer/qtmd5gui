#include <verifier.h>

#include <QtCore/QtCore>
#include <QtCore/QtGlobal>
#include <QtCore/QDir>
#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtGui/QCheckBox>
#include <QtGui/QFileDialog>
#include <QtGui/QFrame>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>

Verifier* instance = NULL;

void messageCapture(QtMsgType _type, const char* _msg)
	{
	QString msg(_msg);
	if (instance != NULL)
		{
		instance->logMessage(_type, msg);
		}
	}

Verifier::Verifier(QWidget* _parent) : QWidget(_parent, 0),
		boxSource(NULL), labelSourcePath(NULL), buttonSourceSelect(NULL), editSource(NULL),
		boxDestination(NULL), labelDestinationPath(NULL), buttonDestinationSelect(NULL), editDestination(NULL),
		buttonCompare(NULL), buttonGetResults(NULL), buttonCopy(NULL),
		labelLog(NULL)
	{
	instance = this;
	qInstallMsgHandler(messageCapture);

	setWindowTitle("Qt File Verifier");
	createLayout();

	connect(this, SIGNAL(startHashing(QString, bool)),
	        &hasher, SLOT(startHashing(QString, bool)));
	connect(this, SIGNAL(resetHashing()),
	        &hasher, SIGNAL(resetHashing()));

	connect(this, SIGNAL(getMissing()),
	        &hasher, SIGNAL(getMissing()));
	connect(this, SIGNAL(getNew()),
	        &hasher, SIGNAL(getNew()));
	connect(this, SIGNAL(copyMissing()),
	        &hasher, SIGNAL(copyMissing()));
	connect(this, SIGNAL(resetDatabase()),
	        &hasher, SIGNAL(resetDatabase()));
	connect(&hasher, SIGNAL(send_message(QString)),
	        this, SLOT(receive_info(QString)));
	connect(&hasher, SIGNAL(send_missing(QString)),
	        this, SLOT(receive_missing(QString)));
	connect(&hasher, SIGNAL(send_new(QString)),
	        this, SLOT(receive_new(QString)));

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
		QHBoxLayout* comparisonLayout = new QHBoxLayout();
		if (comparisonLayout != NULL)
			{
			QVBoxLayout* sourceLayout = new QVBoxLayout();
			if (sourceLayout != NULL)
				{
				if (boxSource == NULL)
					{
					boxSource = new QGroupBox("Source");
					}
				if (boxSource != NULL)
					{
					QVBoxLayout* boxLayout = new QVBoxLayout();
					if (boxLayout != NULL)
						{
						QHBoxLayout* selectionLayout = new QHBoxLayout();
						if (selectionLayout != NULL)
							{
							if (labelSourcePath == NULL)
								{
								labelSourcePath = new QLabel();
								}
							if (labelSourcePath != NULL)
								{
								labelSourcePath->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
								labelSourcePath->setFrameStyle(15);
								labelSourcePath->setFrameShadow(QFrame::Sunken);

								selectionLayout->addWidget(labelSourcePath);
								}

							if (buttonSourceSelect == NULL)
								{
								buttonSourceSelect = new QPushButton("Select");
								}
							if (buttonSourceSelect != NULL)
								{
								connect(buttonSourceSelect, SIGNAL(clicked()),
								        this, SLOT(doSelectSource()));

								selectionLayout->addWidget(buttonSourceSelect);
								}

							boxLayout->addLayout(selectionLayout);
							}

						if (editSource == NULL)
							{
							editSource = new QTextEdit();
							}
						if (editSource != NULL)
							{
							boxLayout->addWidget(editSource);
							}
						boxSource->setLayout(boxLayout);
						}

					sourceLayout->addWidget(boxSource);
					}

				comparisonLayout->addLayout(sourceLayout);
				}

			QVBoxLayout* destinationLayout = new QVBoxLayout();
			if (destinationLayout != NULL)
				{
				if (boxDestination == NULL)
					{
					boxDestination = new QGroupBox("Destination");
					}
				if (boxDestination != NULL)
					{
					QVBoxLayout* boxLayout = new QVBoxLayout();
					if (boxLayout != NULL)
						{
						QHBoxLayout* selectionLayout = new QHBoxLayout();
						if (selectionLayout != NULL)
							{
							if (labelDestinationPath == NULL)
								{
								labelDestinationPath = new QLabel();
								}
							if (labelDestinationPath != NULL)
								{
								labelDestinationPath->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
								labelDestinationPath->setFrameStyle(15);
								labelDestinationPath->setFrameShadow(QFrame::Sunken);

								selectionLayout->addWidget(labelDestinationPath);
								}

							if (buttonDestinationSelect == NULL)
								{
								buttonDestinationSelect = new QPushButton("Select");
								}
							if (buttonDestinationSelect != NULL)
								{
								connect(buttonDestinationSelect, SIGNAL(clicked()),
								        this, SLOT(doSelectDestination()));

								selectionLayout->addWidget(buttonDestinationSelect);
								}

							boxLayout->addLayout(selectionLayout);
							}

						if (editDestination == NULL)
							{
							editDestination = new QTextEdit();
							}
						if (editDestination != NULL)
							{
							boxLayout->addWidget(editDestination);
							}

						boxDestination->setLayout(boxLayout);
						}

					destinationLayout->addWidget(boxDestination);
					}

				comparisonLayout->addLayout(destinationLayout);
				}

			masterLayout->addLayout(comparisonLayout);
			}

		QHBoxLayout* commandLayout = new QHBoxLayout();
		if (commandLayout != NULL)
			{
			if (buttonCompare == NULL)
				{
				buttonCompare = new QPushButton("Compare");
				}
			if (buttonCompare != NULL)
				{
				connect(buttonCompare, SIGNAL(clicked()),
				        this, SLOT(doCompare()));
				commandLayout->addWidget(buttonCompare);
				}

			if (buttonGetResults == NULL)
				{
				buttonGetResults = new QPushButton("Results");
				}
			if (buttonGetResults != NULL)
				{
				connect(buttonGetResults, SIGNAL(clicked()),
				        this, SLOT(doGetResults()));
				commandLayout->addWidget(buttonGetResults);
				}

			if (buttonCopy == NULL)
				{
				buttonCopy = new QPushButton("Copy");
				}
			if (buttonCopy != NULL)
				{
				connect(buttonCopy, SIGNAL(clicked()),
				        this, SLOT(doCopy()));
				commandLayout->addWidget(buttonCopy);
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

void Verifier::logMessage(QtMsgType _type, QString _msg)
	{
	switch (_type)
		{
		case QtDebugMsg:	_msg.prepend("Debug   :");	break;
		case QtWarningMsg:	_msg.prepend("Warning :");	break;
		case QtCriticalMsg:	_msg.prepend("Critical:");	break;
		case QtFatalMsg:	_msg.prepend("Fatal   :");	break;
		default:			_msg.prepend("Unknown :");	break;
		};
	receive_info(_msg);
	}
void Verifier::receive_info(QString _message)
	{
	if (labelLog != NULL)
		{
		labelLog->append(_message);
		}
	}

void Verifier::doSelectSource()
	{
	QString selectedDirectory = QFileDialog::getExistingDirectory(this, "Select Source directory", QDir::currentPath(), QFileDialog::ShowDirsOnly);
	labelSourcePath->setText(selectedDirectory);
	doCheckState();
	}
void Verifier::doSelectDestination()
	{
	QString selectedDirectory = QFileDialog::getExistingDirectory(this, "Select Destination directory", QDir::currentPath(), QFileDialog::ShowDirsOnly);
	labelDestinationPath->setText(selectedDirectory);
	doCheckState();
	}
void Verifier::doCheckState()
	{
	QString source;
	QString destination;
	if (labelSourcePath != NULL)
		{
		source = labelSourcePath->text();
		}
	if (labelDestinationPath != NULL)
		{
		destination = labelDestinationPath->text();
		}

	if (source.isEmpty() == false && destination.isEmpty() == false)
		{
		// enable compare button
		}
	}

void Verifier::doCompare()
	{
	QString source;
	QString destination;
	if (labelSourcePath != NULL)
		{
		source = labelSourcePath->text();
		}
	if (labelDestinationPath != NULL)
		{
		destination = labelDestinationPath->text();
		}

	Q_EMIT resetDatabase();
	Q_EMIT startHashing(source, true);
	Q_EMIT startHashing(destination, false);
	}
void Verifier::doGetResults()
	{
	Q_EMIT getMissing();
	Q_EMIT getNew();
	}
void Verifier::receive_missing(QString _message)
	{
	}
void Verifier::receive_new(QString _new)
	{
	}
void Verifier::doCopy()
	{
	}
