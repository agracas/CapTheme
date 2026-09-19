/*
    SPDX-FileCopyrightText: 2026 CapTheme contributors
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "core.h"
#include "mainwindow.h"

#include <KActionCollection>
#include <KColorScheme>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KToolBar>
#include <QApplication>
#include <QFile>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QPointer>
#include <QToolButton>
#include <kddockwidgets/core/DockWidget.h>

void MainWindow::resetCapThemeLayout()
{
    // Close views, never delete their editing models or project data. In particular,
    // include floating and secondary bins, which are not children of this window.
    QList<QPointer<KDDockWidgets::QtWidgets::DockWidget>> docks;
    for (QWidget *widget : QApplication::allWidgets()) {
        if (auto *dock = qobject_cast<KDDockWidgets::QtWidgets::DockWidget *>(widget)) {
            docks.append(dock);
        }
    }
    for (const auto &dock : docks) {
        if (dock) {
            dock->close();
        }
    }
    using namespace KDDockWidgets;
    const int w = qMax(width(), 1000);
    const int h = qMax(height(), 700);
    mainDockWindow->addDockWidget(m_projectMonitorDock, Location_OnTop, nullptr, QSize(w / 2, h * 3 / 5));
    mainDockWindow->addDockWidget(m_projectBinDock, Location_OnLeft, m_projectMonitorDock, QSize(w / 4, h * 3 / 5));
    mainDockWindow->addDockWidget(m_effectStackDock, Location_OnRight, m_projectMonitorDock, QSize(w / 4, h * 3 / 5));
    mainDockWindow->addDockWidget(m_timelineDock, Location_OnBottom, nullptr, QSize(w, h * 2 / 5));
    m_projectBinDock->addDockWidgetAsTab(m_effectListDock);
    m_projectBinDock->addDockWidgetAsTab(m_compositionListDock);
    m_effectStackDock->addDockWidgetAsTab(m_mixerDock);
    m_projectMonitorDock->addDockWidgetAsTab(m_clipMonitorDock);
    m_projectBinDock->setAsCurrentTab();
    m_projectMonitorDock->setAsCurrentTab();
    m_effectStackDock->setAsCurrentTab();
    m_timelineDock->open();
    auto *bin = m_projectBinDock->asDockWidgetController();
    auto *details = m_effectStackDock->asDockWidgetController();
    bin->resizeInLayout(0, 0, mainDockWindow->width() / 4 - bin->sizeInLayout().width(), 0);
    details->resizeInLayout(mainDockWindow->width() / 4 - details->sizeInLayout().width(), 0, 0, 0);
}

void MainWindow::setupCapTheme()
{
    if (qEnvironmentVariableIsSet("CAPTHEME_CLASSIC")) {
        return;
    }
    const auto colors = KSharedConfig::openConfig(QStringLiteral(":/captheme/CapTheme.colors"), KConfig::SimpleConfig);
    qApp->setProperty("KDE_COLOR_SCHEME_PATH", QStringLiteral(":/captheme/CapTheme.colors"));
    qApp->setPalette(KColorScheme::createApplicationPalette(colors));
    QFile stylesheet(QStringLiteral(":/captheme/captheme.qss"));
    if (stylesheet.open(QIODevice::ReadOnly)) {
        qApp->setStyleSheet(QString::fromUtf8(stylesheet.readAll()));
    }
    Q_EMIT pCore->updatePalette();

    // Retain the complete menus in a compact menu button. Timeline editing
    // actions remain available in their own toolbar.
    toolBar()->hide();
    toolBar(QStringLiteral("extraToolBar"))->hide();
    auto *header = new QToolBar(i18n("CapTheme"), this);
    header->setObjectName(QStringLiteral("capthemeHeader"));
    header->setMovable(false);
    header->setFloatable(false);
    header->setIconSize(QSize(20, 20));
    header->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    addToolBar(Qt::TopToolBarArea, header);

    auto *brand = new QLabel(QStringLiteral("CapTheme"), header);
    brand->setObjectName(QStringLiteral("capthemeBrand"));
    header->addWidget(brand);

    auto *menuButton = new QToolButton(header);
    menuButton->setIcon(QIcon::fromTheme(QStringLiteral("application-menu")));
    menuButton->setToolTip(i18n("Menu"));
    menuButton->setAccessibleName(i18n("Menu"));
    menuButton->setPopupMode(QToolButton::InstantPopup);
    auto *mainMenu = new QMenu(menuButton);
    for (QAction *action : menuBar()->actions()) {
        mainMenu->addAction(action);
    }
    menuButton->setMenu(mainMenu);
    header->addWidget(menuButton);
    menuBar()->hide();

    m_projectBinDock->setTitle(i18n("Media"));
    m_projectMonitorDock->setTitle(i18n("Player"));
    m_effectStackDock->setTitle(i18n("Details"));

    // Proxies follow the real QAction's enabled state and invoke the same editing
    // command, without changing its name, shortcuts or labels elsewhere.
    auto addCommand = [this](QToolBar *bar, const QString &id, const QString &label, const QString &objectName = QString()) {
        QAction *command = actionCollection()->action(id);
        if (!command) {
            return;
        }
        auto *button = new QToolButton(bar);
        button->setObjectName(objectName);
        button->setDefaultAction(command);
        button->setToolButtonStyle(objectName == QStringLiteral("capthemeExport") ? Qt::ToolButtonTextOnly : Qt::ToolButtonTextBesideIcon);
        button->setText(label);
        connect(command, &QAction::changed, button, [button, label]() { button->setText(label); });
        bar->addWidget(button);
    };
    addCommand(header, QStringLiteral("add_clip"), i18n("Import"));
    header->addSeparator();
    addCommand(header, QStringLiteral("edit_undo"), i18n("Undo"));
    addCommand(header, QStringLiteral("edit_redo"), i18n("Redo"));

    auto *navigation = new QToolBar(i18n("Editing tools"), this);
    navigation->setObjectName(QStringLiteral("capthemeNavigation"));
    navigation->setMovable(false);
    navigation->setFloatable(false);
    navigation->setIconSize(QSize(22, 22));
    navigation->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    addToolBarBreak(Qt::TopToolBarArea);
    addToolBar(Qt::TopToolBarArea, navigation);

    auto addPanel = [this, navigation](const QString &label, const QString &icon, KDDockWidgets::QtWidgets::DockWidget *dock) {
        QAction *action = navigation->addAction(QIcon::fromTheme(icon), label);
        action->setCheckable(true);
        action->setChecked(dock->asDockWidgetController()->isCurrentTab());
        connect(action, &QAction::triggered, this, [dock]() {
            dock->open();
            dock->setAsCurrentTab();
        });
        connect(dock, &KDDockWidgets::QtWidgets::DockWidget::isCurrentTabChanged, action, [action](bool current) { action->setChecked(current); });
        connect(dock, &KDDockWidgets::QtWidgets::DockWidget::isOpenChanged, action, [action](bool open) {
            if (!open) {
                action->setChecked(false);
            }
        });
    };
    addPanel(i18n("Media"), QStringLiteral("folder-videos"), m_projectBinDock);
    addPanel(i18n("Audio"), QStringLiteral("audio-volume-high"), m_mixerDock);
    addCommand(navigation, QStringLiteral("add_text_clip"), i18n("Text"));
    addPanel(i18n("Effects"), QStringLiteral("tools-wizard"), m_effectListDock);
    addPanel(i18n("Compositions"), QStringLiteral("view-filter"), m_compositionListDock);
    // Reuse the native recognition action: it creates the subtitle track and
    // opens SpeechDialog with the user's engine, models and language settings.
    if (auto *recognition = actionCollection()->action(QStringLiteral("audio_recognition"))) {
        auto *captions = new QToolButton(navigation);
        captions->setObjectName(QStringLiteral("capthemeCaptions"));
        captions->setDefaultAction(recognition);
        captions->setText(i18n("Captions"));
        captions->setToolTip(i18n("Generate automatic captions from timeline audio"));
        captions->setPopupMode(QToolButton::MenuButtonPopup);
        auto *menu = new QMenu(captions);
        menu->addSection(i18n("Automatic captions"));
        menu->addAction(recognition);
        menu->addSeparator();
        for (const auto &id : {"add_subtitle", "import_subtitle", "export_subtitle", "manage_subtitle"}) {
            if (auto *command = actionCollection()->action(QString::fromLatin1(id))) {
                menu->addAction(command);
            }
        }
        captions->setMenu(menu);
        connect(recognition, &QAction::changed, captions, [captions]() {
            captions->setText(i18n("Captions"));
            captions->setToolTip(i18n("Generate automatic captions from timeline audio"));
        });
        navigation->addWidget(captions);
    }
    for (auto *button : navigation->findChildren<QToolButton *>()) {
        button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    }

    auto *spacer = new QWidget(header);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    header->addWidget(spacer);
    auto *layout = header->addAction(QIcon::fromTheme(QStringLiteral("view-restore")), i18n("Reset Workspace"));
    layout->setToolTip(i18n("Restore the CapTheme panel arrangement"));
    connect(layout, &QAction::triggered, this, &MainWindow::resetCapThemeLayout);
    addCommand(header, QStringLiteral("project_render"), i18n("Export"), QStringLiteral("capthemeExport"));

    KConfigGroup config(KSharedConfig::openConfig(), QStringLiteral("CapTheme"));
    if (!config.readEntry("WorkspaceInitialized", false)) {
        resetCapThemeLayout();
        config.writeEntry("WorkspaceInitialized", true);
        config.sync();
    }
}
