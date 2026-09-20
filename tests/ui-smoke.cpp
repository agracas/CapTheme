// SPDX-License-Identifier: GPL-3.0-or-later
// Loaded only by tests/ui-smoke.sh. No test hooks are built into the application.
#include <QApplication>
#include <QAction>
#include <QFile>
#include <QDialog>
#include <QMenu>
#include <QMainWindow>
#include <QMenuBar>
#include <QScreen>
#include <QTimer>
#include <QTabWidget>
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
        auto *transform = widget(QStringLiteral("capthemeTransform"));
        auto *volume = widget(QStringLiteral("capthemeVolume"));
        require(transform && volume, "clip inspector shortcuts are present");
        require(!transform->isEnabled() && !volume->isEnabled(), "clip controls are disabled without a selection");
        auto *categories = qobject_cast<QTabWidget *>(widget(QStringLiteral("capthemeCategories")));
        require(categories && categories->count() == 3, "inspector has three media categories");
        for (int i = 0; i < categories->count(); ++i) require(!categories->isTabEnabled(i), "categories are disabled without a selection");

        require(qApp->palette().color(QPalette::Window).lightness() < 50, "dark palette reaches QApplication and QML");
        require(qApp->palette().color(QPalette::Highlight) == QColor(118, 242, 228), "mint selection palette loaded from resource");

        auto *exportButton = qobject_cast<QToolButton *>(widget(QStringLiteral("capthemeExport")));
        require(exportButton && exportButton->defaultAction(), "export is bound to a real QAction");
        QAction *command = exportButton->defaultAction();
        const bool enabled = command->isEnabled();
        command->setEnabled(false);
        require(!exportButton->isEnabled(), "export follows disabled command state");
        command->setEnabled(enabled);

        auto *navigation = qobject_cast<QToolBar *>(widget(QStringLiteral("capthemeNavigation")));
        require(navigation && navigation->isVisible(), "editing categories have a separate navigation strip");
        auto *captions = qobject_cast<QToolButton *>(widget(QStringLiteral("capthemeCaptions")));
        require(captions && captions->defaultAction(), "captions uses the native recognition action");
        require(captions->defaultAction()->objectName() == QStringLiteral("audio_recognition"), "captions invokes automatic subtitling");
        require(captions->menu() && captions->menu()->actions().contains(captions->defaultAction()), "automatic captions is also available in the menu");
        for (const auto &id : {"add_subtitle", "import_subtitle", "export_subtitle", "manage_subtitle"}) {
            bool found = false;
            for (auto *action : captions->menu()->actions()) found |= action->objectName() == QString::fromLatin1(id);
            require(found, "native subtitle editing command remains available");
        }
        auto *recognition = captions->defaultAction();
        const bool recognitionEnabled = recognition->isEnabled();
        recognition->setEnabled(false);
        require(!captions->isEnabled(), "captions follows the native command availability");
        recognition->setEnabled(recognitionEnabled);

        if (qEnvironmentVariableIsSet("CAPTHEME_TEST_CAPTIONS")) {
            require(recognitionEnabled, "recognition is available for the current project");
            QTimer::singleShot(4000, window, []() {
                auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
                require(dialog && dialog->windowTitle() == QStringLiteral("Automatic Subtitling"), "captions opens the real automatic subtitling dialog");
                require(dialog->findChild<QWidget *>(QStringLiteral("speech_model")), "native speech model selection is present");
                require(dialog->findChild<QWidget *>(QStringLiteral("timeline_full")), "native transcription scope selection is present");
                dialog->reject();
                // The native action creates a subtitle track. Exit this isolated
                // test without a save prompt; never touch a user project.
                QTimer::singleShot(0, qApp, []() { std::_Exit(0); });
            });
            captions->click();
            return;
        }

        QAction *effects = nullptr;
        QAction *reset = nullptr;
        for (QAction *action : header->actions() + navigation->actions()) {
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
