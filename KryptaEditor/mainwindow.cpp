#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "PrjSetupDialog.h"
#include "ui_prjsetupdialog.h"
#include "EntBrowserDialog.h"
#include "EnvBrowserDialog.h"
#include "ui_EnvBrowserDialog.h"
#include "LayerBrowserDialog.h"
#include "ConfigDialog.h"
#include "SaveDialog.h"
#include "PrjSettingsDialog.h"
#include "AnimManagerDialog.h"
#include "AudioManagerDialog.h"
#include "ItemManagerDialog.h"
#include "Configuration.h"
#include "Map.h"
#include "Resources.h"
#include "Assets.h"
#include "Tool.h"
//#include "EventSystem.h"
#include <QGraphicsColorizeEffect>
#include <QToolButton>
#include <QFileDialog>
#include <QCloseEvent>
#include <QMessageBox>
#include <QDebug>

static const QString mainTitle("Krypta Map Editor");

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), prjsetupDialog(new PrjSetupDialog(this)),
	entbrowseDialog(new EntBrowserDialog(this)), envbrowseDialog(new EnvBrowserDialog(this)), layerbrowseDialog(new LayerBrowserDialog(this)), configDialog(new ConfigDialog(this)),
	prjsettingsDialog(new PrjSettingsDialog(this)), animmanagerDialog(new AnimManagerDialog(this)), audiomanagerDialog(new AudioManagerDialog(this)), itemmanagerDialog(new ItemManagerDialog(this)), 
	saved(true)
{
	ui->setupUi(this);

	setWindowIcon(QIcon("editor\\kicon.png"));
	ui->bPointer->setIcon(QIcon("editor\\pointer.png"));
	ui->bPaint->setIcon(QIcon("editor\\paint.png"));
	ui->bSelect->setIcon(QIcon("editor\\select.png"));

    ui->layerProperties->horizontalHeader()->resizeSections(QHeaderView::Interactive);
    ui->layerProperties->item(0, 0)->setFlags(ui->layerProperties->item(0, 0)->flags() ^ Qt::ItemIsEditable);
    ui->layerProperties->item(1, 0)->setFlags(ui->layerProperties->item(1, 0)->flags() ^ Qt::ItemIsEditable);
    ui->layerProperties->item(2, 0)->setFlags(ui->layerProperties->item(2, 0)->flags() ^ Qt::ItemIsEditable);
    connect(ui->layerProperties, &QTableWidget::cellChanged, [this](int row, int column)
	{

    });

	ui->statusBar->addWidget((statusMain = new QLabel("Welcome to the Krypta Map Editor.")), 70);
	ui->statusBar->addWidget((statusPos = new QLabel), 15);
	ui->statusBar->addWidget((statusTile = new QLabel), 15);

    connect(ui->miFileNew, SIGNAL(triggered()), this, SLOT(onNewTrigger()));
    connect(ui->miFileOpen, SIGNAL(triggered()), this, SLOT(onOpenTrigger()));
	connect(ui->miFileSave, SIGNAL(triggered()), this, SLOT(onSaveTrigger()));
	connect(ui->miFileSaveAs, SIGNAL(triggered()), this, SLOT(onSaveAsTrigger()));
	connect(ui->miFileExport, SIGNAL(triggered()), this, SLOT(onExportTrigger()));
	connect(ui->miFileExportTo, SIGNAL(triggered()), this, SLOT(onExportToTrigger()));
    connect(ui->miFileExit, SIGNAL(triggered()), this, SLOT(onExitTrigger()));
	connect(ui->miPreferences, &QAction::triggered, [this]()
	{
		if (configDialog->showDialog() == DialogResult::OK)
			Configuration::updateConfig(configDialog->getConfig());
	});
	connect(ui->miViewGrid, &QAction::triggered, [this](bool triggered)
	{
		if (!Map::getMap())
			return;
		/** #TODO(incomplete) show/hide grid */
	});
	connect(ui->miViewWaypoint, &QAction::triggered, [this]()
	{
		if (!Map::getMap())
			return;
		QString message;
		Tool<>::switchTool(ToolType::WAYPOINT, message);
		WaypointData data;
		data.looping = false;
		data.object = nullptr;
		Tool<WaypointData>::getTool()->setData(data);
		ui->glWidget->updateCanvas();
		getStatusMain()->setText("Waypoint viewing mode.");
	});
	connect(ui->miViewCentre, &QAction::triggered, [this]()
	{
		if (!Map::getMap())
			return;
		ui->glWidget->resetCamera();
	});
	connect(ui->miProjectSettings, &QAction::triggered, [this]()
	{
		if (!Map::getMap())
			return;
		if (prjsettingsDialog->showDialog() == DialogResult::OK)
		{

		}
	});
	connect(ui->miProjectAnims, &QAction::triggered, [this]()
	{
		if (!Map::getMap())
			return;
		if (animmanagerDialog->showDialog() == DialogResult::OK)
		{

		}
	});
	connect(ui->miProjectAudio, &QAction::triggered, [this]()
	{
		if (!Map::getMap())
			return;
		if (audiomanagerDialog->showDialog() == DialogResult::OK)
		{

		}
	});
	connect(ui->miProjectItems, &QAction::triggered, [this]()
	{
		if (!Map::getMap())
			return;
		if (itemmanagerDialog->showDialog() == DialogResult::OK)
		{

		}
	});
	connect(ui->bBrowseEntities, &QPushButton::clicked, [this]()
	{
		if (entbrowseDialog->showDialog() == DialogResult::OK)
		{
			ui->lEntityName->setText(entbrowseDialog->getSelectedItem()->text());
			ui->lEntity->setPixmap(entbrowseDialog->getSelectedItem()->icon().pixmap(ui->lEntity->size()));

			ui->lEntity->clicked();
		}
	});
	connect(ui->lEntity, &ClickableLabel::clicked, [this]()
	{
		QGraphicsColorizeEffect* effect = new QGraphicsColorizeEffect(ui->lEntity);
		effect->setColor(QColor(255, 255, 0));
		effect->setStrength(0.5f);
		ui->lEntity->setGraphicsEffect(effect);
		ui->lEnv->setGraphicsEffect(nullptr);

		if (entbrowseDialog->getSelectedItem() != nullptr)
		{
			if (Tool<>::getTool()->getType() == ToolType::PAINT)
				Tool<PaintData>::getTool()->getData().objectasset = entbrowseDialog->getSelectedItem()->object;
			prevObject = entbrowseDialog->getSelectedItem()->object;
		}
	});
    connect(ui->bBrowseEnv, &QPushButton::clicked, [this]()
    {
        if (envbrowseDialog->showDialog() == DialogResult::OK)
        {
            ui->lEnvName->setText(envbrowseDialog->getSelectedItem()->text());
            ui->lEnv->setPixmap(envbrowseDialog->getSelectedItem()->icon().pixmap(ui->lEnv->size()));

			ui->lEnv->clicked();
        }
    });
	connect(ui->lEnv, &ClickableLabel::clicked, [this]()
	{
		QGraphicsColorizeEffect* effect = new QGraphicsColorizeEffect(ui->lEnv);
		effect->setColor(QColor(255, 255, 0));
		effect->setStrength(0.5f);
		ui->lEnv->setGraphicsEffect(effect);
		ui->lEntity->setGraphicsEffect(nullptr);

		if (envbrowseDialog->getSelectedItem() != nullptr)
		{
			if (Tool<>::getTool()->getType() == ToolType::PAINT)
				Tool<PaintData>::getTool()->getData().objectasset = envbrowseDialog->getSelectedItem()->object;
			prevObject = envbrowseDialog->getSelectedItem()->object;
		}
	});
	connect(ui->bLayerMan, &QPushButton::clicked, [this]()
	{
		if (layerbrowseDialog->showDialog() == DialogResult::OK)
		{
			auto map = Map::getMap();
			ui->cbLayers->clear();
			for (auto layer : map->getLayers())
				ui->cbLayers->addItem(QString::number(layer->index) + ':' + layer->description);
			if (layerbrowseDialog->getSelectedIndex() >= 0)
				ui->cbLayers->setCurrentIndex(layerbrowseDialog->getSelectedIndex());
			ui->layerProperties->setItem(0, 1, new QTableWidgetItem(map->getCurrentLayer()->description));
			ui->layerProperties->setItem(1, 1, new QTableWidgetItem(QString::number(map->getCurrentLayer()->size[0])));
			ui->layerProperties->setItem(2, 1, new QTableWidgetItem(QString::number(map->getCurrentLayer()->size[1])));
		}
	});

	void(QComboBox:: *cbLayersSignal)(int) = &QComboBox::currentIndexChanged;
	connect(ui->cbLayers, cbLayersSignal, [this](size_t index)
	{
		if (Map::getMap())
		{
			if (index < 0 || index >= Map::getMap()->getLayers().size())
				return;
		}
		else
			return;
		if (Tool<>::getTool()->getType() != ToolType::POINTER)
		{
			ui->glWidget->handleToolSwitch(nullptr, false);
			for (auto item : toolbarItems)
				ui->toolMain->removeAction(item.action);
			toolbarItems.clear();
			QString message;
			Tool<>::switchTool(ToolType::POINTER, message);
			getStatusMain()->setText(message);
		}
		Map::getMap()->setCurrentLayer(index);
		ui->glWidget->updateCanvas();
	});

	connect(ui->bPointer, &QToolButton::clicked, [this]()
	{
		if (Tool<>::getTool()->getType() == ToolType::POINTER)
			return;
		ui->glWidget->handleToolSwitch(nullptr);
	});
	connect(ui->bPaint, &QToolButton::clicked, [this]()
	{
		if (Tool<>::getTool()->getType() == ToolType::PAINT)
			return;
		ui->glWidget->handleToolSwitch(nullptr, false);
		for (auto item : toolbarItems)
			ui->toolMain->removeAction(item.action);
		toolbarItems.clear();

		QLabel* lSpin = nullptr;
		QSpinBox* spin = nullptr;
		lSpin = new QLabel("Brush size: ", ui->toolMain);
		spin = new QSpinBox(ui->toolMain);
		spin->setMaximum(10);
		spin->setMinimum(1);
		spin->setSuffix(" tiles");
		ToolBarItem item1 { ui->toolMain->addWidget(lSpin), lSpin };
		ToolBarItem item2 { ui->toolMain->addWidget(spin), spin };
		toolbarItems.push_back(item1);
		toolbarItems.push_back(item2);
		
		QString message;
		Tool<>::switchTool(ToolType::PAINT, message);
		if (prevObject)
		{
			PaintData data;
			data.size = spin->value();
			data.objectasset = prevObject;
			Tool<PaintData>::getTool()->setData(data);
		}
		getStatusMain()->setText(message);
	});
	connect(ui->bSelect, &QToolButton::clicked, [this]()
	{
		if (Tool<>::getTool()->getType() == ToolType::SELECTION)
			return;
		ui->glWidget->handleToolSwitch(nullptr, false);
		for (auto item : toolbarItems)
			ui->toolMain->removeAction(item.action);
		toolbarItems.clear();

		QString message;
		Tool<>::switchTool(ToolType::SELECTION, message);
		QLabel* lObjCount = new QLabel("Objects selected: 0", ui->toolMain);
		QToolButton* bEdit = new QToolButton(ui->toolMain);
		bEdit->setText("Edit Selected Objects");
		connect(bEdit, &QToolButton::clicked, [this](bool)
		{
			ui->glWidget->handleSelectionEdit();
		});
		QToolButton* bRemove = new QToolButton(ui->toolMain);
		bRemove->setText("Remove Selected Objects");
		connect(bRemove, &QToolButton::clicked, [this](bool)
		{
			ui->glWidget->handleSelectionRemove();
		});
		ToolBarItem item1 { ui->toolMain->addWidget(lObjCount), lObjCount };
		ToolBarItem item2 { ui->toolMain->addSeparator(), nullptr };
		ToolBarItem item3 { ui->toolMain->addWidget(bEdit), bEdit };
		ToolBarItem item4 { ui->toolMain->addWidget(bRemove), bRemove };
		toolbarItems.push_back(item1);
		toolbarItems.push_back(item2);
		toolbarItems.push_back(item3);
		toolbarItems.push_back(item4);
		getStatusMain()->setText(message);
	});
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init()
{
    try
	{
        if (Configuration::loadFromFile("editor.cfg")["editor"]["maximised"] == "true")
            this->showMaximized();
		configDialog->setConfigData(Configuration::getConfig());
		//EventSystem::createSystem();
    }
    catch (const kry::Util::Exception&)
    {
        QMessageBox::information(this, "Config Error", "Failed to open config file!", QMessageBox::Ok);
    }
}

void MainWindow::clearAndSwitchTool()
{
	for (auto item : toolbarItems)
		ui->toolMain->removeAction(item.action);
	toolbarItems.clear();

	QString message;
	Tool<>::switchTool(ToolType::POINTER, message);
	getStatusMain()->setText(message);
}

void MainWindow::onNewTrigger()
{
    if (prjsetupDialog->showDialog() == DialogResult::OK)
    {
        try
        {
            this->setWindowTitle("Krypta Map Editor - " + prjsetupDialog->getUI()->tbPrjName->text());

            if (!Assets::isLoaded())
			{
                Assets::loadAssets("assets", true);
				Resources::initEditorTextures();
				envbrowseDialog->showDialog();
				entbrowseDialog->showDialog();
			}

            if (Map::getMap())
                Map::getMap()->resetMap();
			Map::setProjectName(prjsetupDialog->getUI()->tbPrjName->text());
			Tile defaulttile;
			defaulttile.asset = Assets::getTiles()[0].get(); /** #TODO(bug) there might not be any assets */
			auto map = Map::createMap(prjsetupDialog->getUI()->tbMapName->text(), defaulttile,
									  prjsetupDialog->getUI()->lbLayers);
			saved = false;
			prjsettingsDialog->resetSettings();
			ui->cbLayers->clear();
			for (auto layer : map->getLayers())
				ui->cbLayers->addItem(QString::number(layer->index) + ':' + layer->description);
			ui->layerProperties->setItem(0, 1, new QTableWidgetItem(map->getCurrentLayer()->description));
            ui->layerProperties->setItem(1, 1, new QTableWidgetItem(QString::number(map->getCurrentLayer()->size[0])));
            ui->layerProperties->setItem(2, 1, new QTableWidgetItem(QString::number(map->getCurrentLayer()->size[1])));
			ui->glWidget->resetCamera();
            ui->glWidget->updateCanvas();

			ui->miFileSave->setEnabled(true);
			ui->miFileSaveAs->setEnabled(true);
			ui->miFileExport->setEnabled(true);
			ui->miFileExportTo->setEnabled(true);
			ui->miViewGrid->setEnabled(true);
			ui->miViewWaypoint->setEnabled(true);
			ui->miViewCentre->setEnabled(true);
			ui->miProjectSettings->setEnabled(true);
			ui->miProjectAnims->setEnabled(true);
			ui->miProjectAudio->setEnabled(true);
			ui->miProjectItems->setEnabled(true);
        }
        catch (const kry::Util::Exception& e)
        {
            QMessageBox::information(this, "Map Creation Error", e.what(), QMessageBox::Ok);
        }
    }
}

void MainWindow::onOpenTrigger()
{
	QString file = QFileDialog::getOpenFileName(this, "Open Project", QApplication::applicationDirPath(), "Project files (*.kryprj)");
	if (file.isNull())
		return;
	int index = file.lastIndexOf('/') + 1;
	auto prjname = file.mid(index, file.lastIndexOf('.') - index);
	this->setWindowTitle(mainTitle + " - " + prjname);

	if (!Assets::isLoaded()) // first time loading the map
	{
		Assets::loadAssets("assets", false);
		Resources::initEditorTextures();
	}
	else // map's been loaded before, clear it all
	{
		//Assets::eraseAll();
		Resources::eraseAll();
		while (envbrowseDialog->getUI()->lbIcons->count() > 0)
			delete envbrowseDialog->getUI()->lbIcons->item(0);
	}
	try
	{
		Map::setProjectName(prjname);
		prjsettingsDialog->resetSettings();
		auto map = Map::loadFromFile(this, file, prjsettingsDialog->getAllSettings());
		saved = false;
		ui->cbLayers->clear();
		for (auto layer : map->getLayers())
			ui->cbLayers->addItem(QString::number(layer->index) + ':' + layer->description);
		ui->layerProperties->setItem(0, 1, new QTableWidgetItem(map->getCurrentLayer()->description));
		ui->layerProperties->setItem(1, 1, new QTableWidgetItem(QString::number(map->getCurrentLayer()->size[0])));
		ui->layerProperties->setItem(2, 1, new QTableWidgetItem(QString::number(map->getCurrentLayer()->size[1])));
		ui->glWidget->resetCamera();
		ui->glWidget->updateCanvas();

		ui->miFileSave->setEnabled(true);
		ui->miFileSaveAs->setEnabled(true);
		ui->miFileExport->setEnabled(true);
		ui->miFileExportTo->setEnabled(true);
		ui->miViewGrid->setEnabled(true);
		ui->miViewWaypoint->setEnabled(true);
		ui->miViewCentre->setEnabled(true);
		ui->miProjectSettings->setEnabled(true);
		ui->miProjectAnims->setEnabled(true);
		ui->miProjectAudio->setEnabled(true);
		ui->miProjectItems->setEnabled(true);
	}
	catch (const kry::Util::Exception& e)
	{
		QMessageBox::information(this, "Map Creation Error", e.what(), QMessageBox::Ok);
	}
}

void MainWindow::onSaveTrigger()
{
	if (!Map::getMap())
		return;
	Map::getMap()->saveToFile(this, Map::getProjectName() + ".kryprj", prjsettingsDialog->getAllSettings());
	saved = true;
}

void MainWindow::onSaveAsTrigger()
{
	if (!Map::getMap())
		return;
	QString file = QFileDialog::getSaveFileName(this, "Save Project", QApplication::applicationDirPath(), "Project files (*.kryprj)");
	if (file.isNull() || file.trimmed().isEmpty())
		return;
	int index = file.lastIndexOf('/') + 1;
	auto prjname = file.mid(index, file.lastIndexOf('.') - index);
	this->setWindowTitle(mainTitle + " - " + prjname);
	Map::setProjectName(prjname);
	Map::getMap()->saveToFile(this, Map::getProjectName() + ".kryprj", prjsettingsDialog->getAllSettings());
	saved = true;
}

void MainWindow::onExportTrigger()
{
	if (!Map::getMap())
		return;

	Map::getMap()->exportToFile(this, Map::getMap()->getName(), prjsettingsDialog->getAllSettings());
}

void MainWindow::onExportToTrigger()
{
	if (!Map::getMap())
		return;

	QString path = QFileDialog::getExistingDirectory(this, "Export Project", QApplication::applicationDirPath());
	if (path.isNull() || path.trimmed().isEmpty())
		return;
	Map::getMap()->exportToFile(this, path + '\\' + Map::getMap()->getName(), prjsettingsDialog->getAllSettings());
}

void MainWindow::onExitTrigger()
{
    close();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	if (saved)
	{
		Configuration::saveToFile("editor.cfg");
		return;
	}

	SaveDialog save(this);
	DialogResult result = save.showDialog();
	if (result == DialogResult::YES)
	{
		Configuration::saveToFile("editor.cfg");

		Map::getMap()->saveToFile(this, Map::getProjectName() + ".kryprj", prjsettingsDialog->getAllSettings());
	}
	else if (result == DialogResult::NO)
	{
		Configuration::saveToFile("editor.cfg");
	}
	else if (result == DialogResult::CANCEL)
		event->ignore();
}
