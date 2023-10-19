#include "channelfilterdialog.h"
#include "ui_channelfilterdialog.h"

#include "channelfiltermodel.h"
#include "channelfiltersortfilterproxymodel.h"

#include <shv/core/log.h>

#include <QLineEdit>
#include <QMenu>

namespace shv::visu::timeline {

ChannelFilterDialog::ChannelFilterDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ChannelFilterDialog)
{
	ui->setupUi(this);

	m_channelsFilterModel = new ChannelFilterModel(this);

	m_channelsFilterProxyModel = new ChannelFilterSortFilterProxyModel(this);
	m_channelsFilterProxyModel->setSourceModel(m_channelsFilterModel);
	m_channelsFilterProxyModel->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
	m_channelsFilterProxyModel->setRecursiveFilteringEnabled(true);

	ui->tvChannelsFilter->setModel(m_channelsFilterProxyModel);
	ui->tvChannelsFilter->header()->hide();
	ui->tvChannelsFilter->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

	QMenu *view_menu = new QMenu(this);
	m_resetViewAction = view_menu->addAction(tr("Clear"), this, &ChannelFilterDialog::resetView);
	m_saveViewAction = view_menu->addAction(tr("Save"), this, &ChannelFilterDialog::saveView);
	m_saveViewAsAction = view_menu->addAction(tr("Save as"), this, &ChannelFilterDialog::saveViewAs);
	m_revertViewAction = view_menu->addAction(tr("Revert changes"), this, &ChannelFilterDialog::revertView);
	m_deleteViewAction = view_menu->addAction(tr("Delete"), this, &ChannelFilterDialog::deleteView);
	m_exportViewAction = view_menu->addAction(tr("Export"), this, &ChannelFilterDialog::exportView);
	m_importViewAction = view_menu->addAction(tr("Import"), this, &ChannelFilterDialog::importView);
	ui->pbActions->setMenu(view_menu);

	connect(ui->tvChannelsFilter, &QTreeView::customContextMenuRequested, this, &ChannelFilterDialog::onCustomContextMenuRequested);
	connect(ui->leMatchingFilterText, &QLineEdit::textChanged, this, &ChannelFilterDialog::onLeMatchingFilterTextChanged);
	connect(ui->pbClearMatchingText, &QPushButton::clicked, this, &ChannelFilterDialog::onPbClearMatchingTextClicked);
	connect(ui->pbCheckItems, &QPushButton::clicked, this, &ChannelFilterDialog::onPbCheckItemsClicked);
	connect(ui->pbUncheckItems, &QPushButton::clicked, this, &ChannelFilterDialog::onPbUncheckItemsClicked);
}

ChannelFilterDialog::~ChannelFilterDialog()
{
	delete ui;
}

void ChannelFilterDialog::init(const QString &site_path, Graph *graph)
{
	m_sitePath = site_path;
	m_graph = graph;
	m_channelsFilterModel->createNodes(graph->channelPaths());

	for (const QString &view_name : m_graph->savedVisualSettingsNames(m_sitePath)) {
		ui->cbLayouts->addItem(view_name); //, QVariant::fromValue(View{ view_name, false, tl::Graph::VisualSettings() }));
	}
}

QSet<QString> ChannelFilterDialog::selectedChannels()
{
	return m_channelsFilterModel->selectedChannels();
}

void ChannelFilterDialog::setFilter(const QString &filter_name, const QSet<QString> &channels)
{
	ui->cbLayouts->setCurrentText(filter_name);
	m_channelsFilterModel->setSelectedChannels(channels);
}

void ChannelFilterDialog::applyTextFilter()
{
	m_channelsFilterProxyModel->setFilterString(ui->leMatchingFilterText->text());

	if (m_channelsFilterProxyModel->rowCount() == 1){
		ui->tvChannelsFilter->setCurrentIndex(m_channelsFilterProxyModel->index(0, 0));
		ui->tvChannelsFilter->expandRecursively(ui->tvChannelsFilter->currentIndex());
	}
}

void ChannelFilterDialog::deleteView()
{
/*	View current_view = ui->cbViews->currentData().value<View>();
	m_graph->deleteVisualSettings(QString::fromStdString(m_site->shvPath()), current_view.name);
	ui->cbViews->removeItem(index);
	ui->cbViews->setCurrentIndex(VIEW_COMBO_NO_VIEW_INDEX);
	onViewSelected(VIEW_COMBO_NO_VIEW_INDEX);
	*/
}

void ChannelFilterDialog::exportView()
{
/*	int index = ui->cbViews->currentIndex();
	if (index == VIEW_COMBO_NO_VIEW_INDEX) {
		return;
	}
	QString file_name = QFileDialog::getSaveFileName(this, tr("Input file name"), RECENT_SETTINGS_DIR, "*" + FLATLINE_VIEW_SETTINGS_FILE_EXTENSION);
	if (!file_name.isEmpty()) {
		if (!file_name.endsWith(FLATLINE_VIEW_SETTINGS_FILE_EXTENSION)) {
			file_name.append(FLATLINE_VIEW_SETTINGS_FILE_EXTENSION);
		}
		View current_view = ui->cbViews->currentData().value<View>();
		m_graph->saveVisualSettings(QString::fromStdString(m_site->shvPath()), current_view.name);
		QSettings settings(file_name, QSettings::Format::IniFormat);
		settings.setValue("fileType", FLATLINE_VIEW_SETTINGS_FILE_TYPE);
		settings.setValue("settings", m_graph->visualSettings().toJson());
		RECENT_SETTINGS_DIR = QFileInfo(file_name).path();
	}
	*/
}

void ChannelFilterDialog::importView()
{
/*	QString file_name = QFileDialog::getOpenFileName(this, tr("Input file name"), RECENT_SETTINGS_DIR, "*" + FLATLINE_VIEW_SETTINGS_FILE_EXTENSION);
	if (!file_name.isEmpty()) {
		QString view_name = QInputDialog::getText(this, tr("Import as"), tr("Input view name"));
		if (!view_name.isEmpty()) {
			for (int i = 0; i < ui->cbViews->count(); ++i) {
				if (ui->cbViews->itemData(i).value<View>().name == view_name) {
					QMessageBox::warning(this, tr("Error"), tr("This view already exists"));
					return;
				}
			}
			QSettings settings_file(file_name, QSettings::Format::IniFormat);
			if (settings_file.value("fileType").toString() != FLATLINE_VIEW_SETTINGS_FILE_TYPE) {
				QMessageBox::warning(this, tr("Error"), tr("This file is not flatline view setting file"));
				return;
			}
			tl::Graph::VisualSettings settings = tl::Graph::VisualSettings::fromJson(settings_file.value("settings").toString());
			QSet<QString> graph_channels = m_graph->channelPaths();
			for (int i = 0; i < settings.channels.count(); ++i) {
				if (!graph_channels.contains(settings.channels[i].shvPath)) {
					settings.channels.removeAt(i--);
				}
			}
			int index = ui->cbViews->count();
			ui->cbViews->addItem(view_name + "*", QVariant::fromValue(View{ view_name, true, settings }));
			ui->cbViews->setCurrentIndex(index);
			onViewSelected(index);
			RECENT_SETTINGS_DIR = QFileInfo(file_name).path();
		}
	}
	*/
}

void ChannelFilterDialog::saveView()
{
	m_graph->saveVisualSettings(m_sitePath, ui->cbLayouts->currentText());
}

void ChannelFilterDialog::saveViewAs()
{
/*	QString view_name = QInputDialog::getText(this, tr("Save as"), tr("Input view name"));
	if (!view_name.isEmpty()) {
		for (int i = 0; i < ui->cbViews->count(); ++i) {
			if (ui->cbViews->itemData(i).value<View>().name == view_name) {
				QMessageBox::warning(this, tr("Error"), tr("This view already exists"));
				return;
			}
		}
		int index = ui->cbViews->currentIndex();
		View current_view = ui->cbViews->itemData(index).value<View>();
		current_view.edited = false;
		current_view.settings = tl::Graph::VisualSettings();
		ui->cbViews->setItemText(index, current_view.name);
		ui->cbViews->setItemData(index, QVariant::fromValue(current_view));

		m_graph->saveVisualSettings(QString::fromStdString(m_site->shvPath()), view_name);
		index = ui->cbViews->count();
		ui->cbViews->addItem(view_name, QVariant::fromValue(View{ view_name, false, tl::Graph::VisualSettings() }));
		ui->cbViews->setCurrentIndex(index);
		onViewSelected(index);
	}
	*/
}

void ChannelFilterDialog::revertView()
{
/*	int index = ui->cbViews->currentIndex();
	View current_view = ui->cbViews->itemData(index).value<View>();

	if (index == VIEW_COMBO_NO_VIEW_INDEX) {
		m_graph->reset();
	}
	else {
		m_graph->loadVisualSettings(QString::fromStdString(m_site->shvPath()), current_view.name);
	}

	current_view.edited = false;
	current_view.settings = tl::Graph::VisualSettings();
	ui->cbViews->setItemText(index, current_view.name);
	ui->cbViews->setItemData(index, QVariant::fromValue(current_view));
	m_revertViewAction->setEnabled(false);
	*/
}

void ChannelFilterDialog::resetView()
{
/*	int index = ui->cbViews->currentIndex();
	View current_view = ui->cbViews->itemData(index).value<View>();
	current_view.edited = (index != VIEW_COMBO_NO_VIEW_INDEX);
	m_graph->reset();
	current_view.settings = tl::Graph::VisualSettings();
	ui->cbViews->setItemText(index, current_view.name + (current_view.edited ? "*" : QString()));
	ui->cbViews->setItemData(index, QVariant::fromValue(current_view));
	m_revertViewAction->setEnabled(current_view.edited);
	*/
}

void ChannelFilterDialog::onCustomContextMenuRequested(QPoint pos)
{
	QModelIndex ix = ui->tvChannelsFilter->indexAt(pos);

	if (ix.isValid()) {
		QMenu menu(this);

		menu.addAction(tr("Expand"), this, [this, ix]() {
			ui->tvChannelsFilter->expandRecursively(ix);
		});
		menu.addAction(tr("Collapse"), this, [this, ix]() {
			ui->tvChannelsFilter->collapse(ix);
		});

		menu.addSeparator();

		menu.addAction(tr("Expand all nodes"), this, [this]() {
			ui->tvChannelsFilter->expandAll();
		});

		menu.exec(ui->tvChannelsFilter->mapToGlobal(pos));
	}
}

void ChannelFilterDialog::onPbCheckItemsClicked()
{
	setVisibleItemsCheckState(Qt::Checked);
}

void ChannelFilterDialog::onPbUncheckItemsClicked()
{
	setVisibleItemsCheckState(Qt::Unchecked);
}

void ChannelFilterDialog::onPbClearMatchingTextClicked()
{
	ui->leMatchingFilterText->setText(QString());
}

void ChannelFilterDialog::onLeMatchingFilterTextChanged(const QString &text)
{
	Q_UNUSED(text);
	applyTextFilter();
}

void ChannelFilterDialog::onChbFindRegexChanged(int state)
{
	Q_UNUSED(state);
	applyTextFilter();
}

void ChannelFilterDialog::setVisibleItemsCheckState(Qt::CheckState state)
{
	for (int row = 0; row < m_channelsFilterProxyModel->rowCount(); row++) {
		setVisibleItemsCheckState_helper(m_channelsFilterProxyModel->index(row, 0), state);
	}

	m_channelsFilterModel->fixCheckBoxesIntegrity();
}

void ChannelFilterDialog::setVisibleItemsCheckState_helper(const QModelIndex &mi, Qt::CheckState state)
{
	if (!mi.isValid()) {
		return;
	}

	m_channelsFilterModel->setItemCheckState(m_channelsFilterProxyModel->mapToSource(mi), state);

	for (int row = 0; row < m_channelsFilterProxyModel->rowCount(mi); row++) {
		setVisibleItemsCheckState_helper(m_channelsFilterProxyModel->index(row, 0, mi), state);
	}
}

}
