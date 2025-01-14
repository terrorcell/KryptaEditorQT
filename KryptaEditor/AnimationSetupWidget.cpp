#include "AnimationSetupWidget.h"
#include "ui_AnimationSetupWidget.h"
#include "AnimManagerDialog.h"
#include "ui_AnimManagerDialog.h"
#include "Resources.h"
#include "Utilities.h"
#include <QListWidgetItem>
#include <QFileDialog>
#include <QTimer>
#include <QImageReader>
#include <QInputDialog>
#include <QComboBox>
#include <QDebug>

namespace
{
	QTableWidget* initTable(QTableWidget* table)
	{
		table->setVerticalHeader(new QHeaderView(Qt::Vertical, table));
		table->verticalHeader()->setVisible(false);
		table->setHorizontalHeader(new QHeaderView(Qt::Horizontal, table));
		table->horizontalHeader()->setVisible(false);
		table->horizontalHeader()->setStretchLastSection(true);
		table->insertColumn(0);
		table->insertColumn(0);

		return table;
	}
}

AnimationSetupWidget::AnimationSetupWidget(QWidget *parent) : QWidget(parent), ui(new Ui::AnimationSetupWidget),
	timer(new QTimer(this))
{
	ui->setupUi(this);

	ui->lbImages->setMovement(QListView::Snap);
	ui->lbImages->setIconSize(QSize(130, 65));
	ui->lbImages->setGridSize(ui->lbImages->iconSize());

	connect(timer, &QTimer::timeout, [this]()
	{
		int index = ui->lbImages->currentRow();
		ui->lbImages->setCurrentRow(index < 0 || index >= (ui->lbImages->count() - 1) ? 0 : index + 1);
	});
	auto updateFramesKey = [this](QTableWidget* search, unsigned count)
	{
		for (unsigned i = 0; i < search->rowCount(); ++i)
		{
			if (search->item(i, 0)->text() == "frames")
			{
				search->item(i, 1)->setText(QString::number(count));
				break;
			}
		}
	};

	connect(ui->lbImages, &QListWidget::currentItemChanged, [this](QListWidgetItem*, QListWidgetItem*)
	{
		/** #TODO(incomplete) handle image reordering (reorder frame tabs as well) */
		if (ui->lbImages->currentRow() < 0 || ui->lbImages->currentItem() == nullptr)
		{
			ui->lbImages->clearSelection();
			return;
		}
		if (!timer->isActive())
			ui->tabs->setCurrentIndex(ui->lbImages->currentRow());

		int row = 0;
		auto table = dynamic_cast<QTableWidget*>(ui->tabs->currentWidget());
		for (; row < table->rowCount() && table->item(row, 0)->text() != "image"; ++row);

		QString path;
		if (row < table->rowCount() && !table->item(row, 1)->text().isEmpty())
			path = table->item(row, 1)->text().trimmed();
		else
		{
			row = 0;
			table = ui->tSheetProps;
			for (; row < table->rowCount() && table->item(row, 0)->text() != "sheetImage"; ++row);
			path = table->item(row, 1)->text().trimmed();
		}

		if (path.isEmpty())
			return;
		QImageReader reader(path);
		QSize iconsize = reader.size();
		if (iconsize.width() == 0) // prevent a divide by 0
			iconsize.setWidth(1);
		if (iconsize.height() == 0)
			iconsize.setHeight(1);
		QString strpivot;
		table = dynamic_cast<QTableWidget*>(ui->tabs->currentWidget());
		for (row = 0; row < table->rowCount() && table->item(row, 0)->text() != "image"; ++row);
		if (table->item(row, 1)->text().isEmpty())
		{
			for (row = 0; row < ui->tSheetProps->rowCount() && ui->tSheetProps->item(row, 0)->text() != "sheetImage"; ++row);
			if (ui->tSheetProps->item(row, 1)->text().trimmed().isEmpty())
				strpivot = "{ 0, 0 }";
			else
			{
				for (row = 0; row < ui->tSheetProps->rowCount() && ui->tSheetProps->item(row, 0)->text() != "framePivot"; ++row);
				strpivot = ui->tSheetProps->item(row, 1)->text().trimmed();
			}
		}
		else
		{
			for (row = 0; row < table->rowCount() && table->item(row, 0)->text() != "pivot"; ++row);
			strpivot = table->item(row, 1)->text().trimmed().isEmpty() ? QString("{ 0, 0 }") : table->item(row, 1)->text().trimmed();
		}
		auto pivot = kry::Util::Vector2f::Vector(qToKString(strpivot.isEmpty() ? QString("{ 0, 0 }") : strpivot));
		ui->gvPivot->setup(ui->lbImages->currentItem()->icon().pixmap(iconsize), pivot[0] / iconsize.width(), pivot[1] / iconsize.height());
	});
	connect(ui->lbImages, &QListWidget::itemClicked, [this](QListWidgetItem* item)
	{
		//if (item->isSelected())
		//	item->setSelected(false);
		//else
		//{
			//item->setSelected(true);
			ui->lbImages->currentItemChanged(item, item);
		//}
	});
	connect(ui->bAdd, &QPushButton::clicked, [this, &updateFramesKey](bool)
	{
		QStringList files = QFileDialog::getOpenFileNames(this, "Create New Frame", QApplication::applicationDirPath() + "\\assets\\resources\\images", "Image files (*.jpg *.png *.bmp)");
		if (!files.empty())
		{
			for (QString& file : files)
			{
				int currentindex = dynamic_cast<AnimManagerDialog*>(this->parent()->parent()->parent()->parent())->getUI()->cbAnims->currentIndex();
				Animation<>* currentanim = Resources::getAnimations()[currentindex].get();

				QString section = kryToQString(currentanim->properties[currdir]["Skins"]["name"]);
				auto ksection = qToKString(section);
				kry::Media::Config framesettings;
				framesettings["Skins"]["name"] = ksection;
				framesettings[ksection]["image"] = qToKString(file);
				framesettings[ksection]["pivot"] = "";
				framesettings[ksection]["dimensions"] = "";
				framesettings[ksection]["RGBA"] = "";
				framesettings[ksection]["rotation"] = "";
				framesettings[ksection]["UVposition"] = "";
				framesettings[ksection]["UVdimensions"] = "";
				framesettings[ksection]["linearFilter"] = "";
				framesettings[ksection]["sortDepth"] = "";
				framesettings[ksection]["sortPivotOffset"] = "";
				currentanim->frames[currdir].push_back(framesettings);

				QTableWidget* table = initTable(new QTableWidget(ui->tabs));
				for (auto& key : framesettings[ksection].getKeyNames())
				{
					int index = table->rowCount();
					table->insertRow(index);
					table->setItem(index, 0, new QTableWidgetItem(kryToQString(key)));
					table->item(index, 0)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
					table->setItem(index, 1, new QTableWidgetItem(kryToQString(framesettings[ksection][key])));
				}
				connect(table, &QTableWidget::itemChanged, [this, table, currentanim, ksection](QTableWidgetItem* item)
				{
					if (item->column() == 0)
						return;
					unsigned index = 0;
					for (unsigned i = 0; i < ui->tabs->count(); ++i)
					{
						if (ui->tabs->widget(i) == table)
						{
							index = i;
							break;
						}
					}
					/** #TODO(incomplete) if the key here is "image", set the icon image for this frame */
					currentanim->frames[currdir][index][ksection][qToKString(table->item(item->row(), 0)->text())] = qToKString(item->text().trimmed());
				});
				ui->tabs->addTab(table, "Frame: " + QString::number(ui->tabs->count()));

				ui->lbImages->addItem(new QListWidgetItem(QIcon(file), ""));
			}
			if (ui->tabs->count() > 1)
			{
				bool found = false;
				auto table = dynamic_cast<QTableWidget*>(ui->tabs->widget(0));
				for (int i = 0; i < table->rowCount() && !found; ++i)
				{
					if (table->item(i, 0)->text() == "image" && table->item(i, 1)->text().isEmpty())
					{
						QTableWidget* props = ui->tSheetProps;
						for (int j = 0; j < props->rowCount(); ++j)
						{
							if (props->item(j, 0)->text() == "sheetImage")
							{
								table->item(i, 1)->setText(props->item(j, 1)->text());
								found = true;
								break;
							}
						}
					}
				}

				ui->bRemove->setEnabled(true);
			}
			updateFramesKey(ui->tSheetProps, ui->tabs->count());
		}
	});
	connect(ui->bRemove, &QPushButton::clicked, [this, &updateFramesKey](bool)
	{
		int index = ui->lbImages->currentRow();
		QString toremove = ui->tabs->tabText(index);
		delete ui->lbImages->takeItem(index);
		ui->tabs->removeTab(index);
		if (ui->tabs->count() <= 1)
			ui->bRemove->setEnabled(false);
		for (unsigned i = 0; i < ui->tabs->count(); ++i)
			ui->tabs->tabBar()->setTabText(i, "Frame: " + QString::number(i));

		auto cindex = dynamic_cast<AnimManagerDialog*>(this->parent()->parent()->parent()->parent())->getUI()->cbAnims->currentIndex();
		Resources::getAnimations()[cindex]->frames[currdir].erase(Resources::getAnimations()[cindex]->frames[currdir].begin() + index);
		updateFramesKey(ui->tSheetProps, ui->tabs->count());
	});
	connect(ui->bAnimate, &QPushButton::clicked, [this](bool)
	{
		int row = 0;
		for (; row < ui->tSheetProps->rowCount() && ui->tSheetProps->item(row, 0)->text() != "fps"; ++row);
		auto strfps = qToKString(ui->tSheetProps->item(row, 1)->text());
		int fps = strfps.trim().isEmpty() ? 0 : kry::Util::toIntegral<int>(strfps);

		if (timer->isActive())
		{
			timer->stop();
			ui->bAnimate->setText("Animate");
			ui->tabs->setCurrentIndex(ui->lbImages->currentRow());
		}
		else
		{
			timer->start(1000 / fps);
			ui->bAnimate->setText("Stop Animating");
		}
	});
	ui->gvPivot->setClickCallback([this](float x, float y)
	{
		auto pivot = kry::Util::Vector2f(x, y);
		int row = 0;
		for (; row < ui->tSheetProps->rowCount() && ui->tSheetProps->item(row, 0)->text() != "framePivot"; ++row);
		ui->tSheetProps->item(row, 1)->setText(kryToQString(pivot.toString()));
		auto table = dynamic_cast<QTableWidget*>(ui->tabs->widget(ui->lbImages->currentRow()));
		for (row = 0; row < table->rowCount() && table->item(row, 0)->text() != "image"; ++row);
		if (!table->item(row, 1)->text().isEmpty())
		{
			for (row = 0; row < table->rowCount() && table->item(row, 0)->text() != "pivot"; ++row);
			table->item(row, 1)->setText(kryToQString(pivot.toString()));
		}
	});
	connect(ui->bSetPivot, &QPushButton::clicked, [this](bool)
	{
		ui->gvPivot->setPivoting(true);
	});
	connect(ui->bCancelPivot, &QPushButton::clicked, [this](bool)
	{
		ui->gvPivot->setPivoting(false);
	});
}

AnimationSetupWidget::~AnimationSetupWidget()
{
	delete ui;
}

void AnimationSetupWidget::setup(std::shared_ptr<Animation<kry::Graphics::Texture> > animation, int tabindex)
{
	currdir = tabindex;
	auto sectionname = animation->properties[currdir]["Skins"]["name"];

	QTableWidget* table = ui->tSheetProps;
	while (table->rowCount() > 0)
		table->removeRow(0);
	for (auto& key : animation->properties[currdir][sectionname].getKeyNames()) // main section
	{
		int index = table->rowCount();
		table->insertRow(index);
		table->setItem(index, 0, new QTableWidgetItem(kryToQString(key)));
		table->item(index, 0)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		table->setItem(index, 1, new QTableWidgetItem(kryToQString(animation->properties[currdir][sectionname][key])));
	}
	
	connect(table, &QTableWidget::itemChanged, [this, table, animation, sectionname, tabindex](QTableWidgetItem* item)
	{
		if (item->column() == 0)
			return;
		animation->properties[tabindex][sectionname][qToKString(table->item(item->row(), 0)->text())] = qToKString(item->text().trimmed());
	});

	ui->lbImages->clear();
	while (ui->tabs->count() > 0)
		delete ui->tabs->widget(0);
	for (auto& frame : animation->frames[currdir]) // frames
	{
		auto section = frame["Skins"]["name"];

		table = initTable(new QTableWidget(ui->tabs));
		for (auto& key : frame[section].getKeyNames()) // frame table
		{
			int index = table->rowCount();
			table->insertRow(index);
			table->setItem(index, 0, new QTableWidgetItem(kryToQString(key)));
			table->item(index, 0)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			table->setItem(index, 1, new QTableWidgetItem(kryToQString(frame[section][key])));
		}
		unsigned tableindex = ui->tabs->count();
		connect(table, &QTableWidget::itemChanged, [this, table, animation, tableindex, section, tabindex](QTableWidgetItem* item)
		{
			if (item->column() == 0)
				return;
			animation->frames[tabindex].at(tableindex)[section][qToKString(table->item(item->row(), 0)->text())] = qToKString(item->text().trimmed());
		});
		ui->tabs->addTab(table, "Frame: " + QString::number(ui->tabs->count()));
		ui->lbImages->addItem(new QListWidgetItem(QIcon(frame[section]["image"].isEmpty() ? animation->path : kryToQString(frame[section]["image"])), ""));
	}
	if (ui->lbImages->count() > 0)
		ui->lbImages->setCurrentRow(0);
}

void AnimationSetupWidget::saveTo(std::shared_ptr<Animation<kry::Graphics::Texture> > animation, int tabindex)
{
	auto sectionname = animation->properties[tabindex]["Skins"]["name"];
	for (int row = 0; row < ui->tSheetProps->rowCount(); ++row)
		animation->properties[tabindex][sectionname][qToKString(ui->tSheetProps->item(row, 0)->text())] = qToKString(ui->tSheetProps->item(row, 1)->text());
	/*
	for (int i = 0; i < ui->tabs->count(); ++i)
	{
		auto table = dynamic_cast<QTableWidget*>(ui->tabs->widget(i));
		auto framename = sectionname + ": " + kry::Util::toString(i);
		for (int row = 0; row < table->rowCount(); ++row)
			animation->properties[tabindex][framename][qToKString(table->item(row, 0)->text())] = qToKString(table->item(row, 1)->text());
	}
	*/
}

void AnimationSetupWidget::release()
{
	timer->stop();
	ui->bAnimate->setText("Animate");
	ui->gvPivot->reset();
}

void AnimationSetupWidget::clearAll()
{
	ui->gvPivot->clear();
	while (ui->tSheetProps->rowCount() > 0)
		ui->tSheetProps->removeRow(0);
	ui->lbImages->clear();
	while (ui->tabs->count() > 0)
		delete ui->tabs->widget(0);
}
