// SPDX-License-Identifier: GPL-3.0-or-later
// Loaded only by tests/ui-smoke.sh. No test hooks are built into the application.
#include <QApplication>
#include <QAction>
#include <QFile>
#include <QMainWindow>
#include <QMenuBar>
#include <QScreen>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <cstdio>
#include <cstdlib>

static void require(bool condition, const char *message)
{
    if (!condition) {
        std::fprintf(stderr, "CAPTHEME FAIL: %s\n", message);
        std::exit(2);
    }
    std::fprintf(stderr, "CAPTHEME PASS: %s\n", message);
}

static QWidget *widget(const QString &name)
{
    for (QWidget *candidate : QApplication::allWidgets()) {
        if (candidate->objectName() == name) {
            return candidate;
        }
    }
    return nullptr;
}

static void checkInterface()
{
    auto *header = qobject_cast<QToolBar *>(widget(QStringLiteral("capthemeHeader")));
    require(header && header->isVisible(), "the real editor opens with the CapTheme toolbar");
    auto *window = qobject_cast<QMainWindow *>(header->window());
    require(window != nullptr, "toolbar belongs to the editor window");
    window->resize(1600, 960);

    QTimer::singleShot(1500, window, [header, window]() {
        QWidget *bin = widget(QStringLiteral("project_bin"));
        QWidget *monitor = widget(QStringLiteral("projectmonitor"));
        QWidget *stack = widget(QStringLiteral("effect_stack"));
        QWidget *timeline = widget(QStringLiteral("timeline"));
        require(bin && monitor && stack && timeline, "all four editing panels exist");
        require(bin->isVisible() && monitor->isVisible() && stack->isVisible() && timeline->isVisible(), "primary panels are visible");
        require(bin->mapToGlobal(QPoint()).x() < monitor->mapToGlobal(QPoint()).x(), "media is left of preview");
        require(monitor->mapToGlobal(QPoint()).x() < stack->mapToGlobal(QPoint()).x(), "inspector is right of preview");
        require(timeline->mapToGlobal(QPoint()).y() > monitor->mapToGlobal(QPoint()).y(), "timeline is below preview");
        require(!window->menuBar()->isVisible(), "compact menu remains active after restoring settings");
        require(qApp->palette().color(QPalette::Window).lightness() < 50, "dark palette reaches QApplication and QML");
        require(qApp->palette().color(QPalette::Highlight) == QColor(118, 242, 228), "mint selection palette loaded from resource");

        auto *exportButton = qobject_cast<QToolButton *>(widget(QStringLiteral("capthemeExport")));
        require(exportButton && exportButton->defaultAction(), "export is bound to a real QAction");
        QAction *command = exportButton->defaultAction();
        const bool enabled = command->isEnabled();
        command->setEnabled(false);
        require(!exportButton->isEnabled(), "export follows disabled command state");
        command->setEnabled(enabled);

        QAction *effects = nullptr;
        QAction *reset = nullptr;
        for (QAction *action : header->actions()) {
            if (action->text() == QStringLiteral("Effects")) effects = action;
            if (action->text() == QStringLiteral("Reset Workspace")) reset = action;
        }
        require(effects && reset, "navigation and workspace reset are present");
        effects->trigger();
        require(widget(QStringLiteral("effect_list"))->isVisible(), "Effects opens the real effects panel");
        reset->trigger();
        reset->trigger();
        require(bin->isVisible() && monitor->isVisible(), "repeated reset restores primary panels without deleting them");
        require(bin->width() >= window->width() / 5, "media panel retains a usable width after reset");
        QTimer::singleShot(1000, window, [window]() {
            const QString path = qEnvironmentVariable("CAPTHEME_SCREENSHOT");
            require(!path.isEmpty() && window->screen()->grabWindow(window->winId()).save(path), "screenshot saved from running editor");
            require(window->close(), "editor closes and saves workspace normally");
            qApp->quit();
        });
    });
}

static void scheduleTest()
{
    if (!qEnvironmentVariableIsSet("CAPTHEME_UI_TEST")) return;
    QTimer::singleShot(15000, qApp, checkInterface);
}
Q_COREAPP_STARTUP_FUNCTION(scheduleTest)
