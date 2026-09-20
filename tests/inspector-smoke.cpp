// SPDX-License-Identifier: GPL-3.0-or-later
#include <QApplication>
#include <QAction>
#include <QFileDialog>
#include <QFileInfo>
#include <QLineEdit>
#include <QMainWindow>
#include <QScreen>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTest>
#include <QTimer>
#include <QTabWidget>
#include <QToolButton>
#include <QTreeView>
#include <cstdio>
#include <cstdlib>

static QMainWindow *editor = nullptr;
static void check(bool ok, const char *message)
{
    std::fprintf(stderr, "INSPECTOR %s: %s\n", ok ? "PASS" : "FAIL", message);
    if (!ok) {
        if (editor) editor->grab().save(qEnvironmentVariable("CAPTHEME_SCREENSHOT"));
        std::_Exit(2);
    }
}

static QWidget *named(const QString &name)
{
    for (auto *w : QApplication::allWidgets()) if (w->objectName() == name) return w;
    return nullptr;
}

static QModelIndex findClip(QAbstractItemModel *model, const QModelIndex &parent = {})
{
    for (int r = 0; r < model->rowCount(parent); ++r) {
        auto ix = model->index(r, 0, parent);
        if (ix.data().toString().contains(QStringLiteral("inspector-fixture"))) return ix;
        const auto child = findClip(model, ix);
        if (child.isValid()) return child;
    }
    return {};
}

static QAction *action(const QString &name)
{
    // KXMLGUI's action collection is not necessarily a QObject child of the window.
    for (auto *w : QApplication::allWidgets()) {
        for (auto *a : w->actions()) if (a->objectName() == name) return a;
    }
    return nullptr;
}

static double value(QWidget *w)
{
    if (auto *s = w->findChild<QDoubleSpinBox *>()) return s->value();
    if (auto *s = w->findChild<QSpinBox *>()) return s->value();
    check(false, "numeric editor exists");
    return 0;
}

static void setValue(const QString &name, double number)
{
    auto *w = named(name);
    check(w && QMetaObject::invokeMethod(w, "setValue", Q_ARG(double, number)), "native parameter edit accepted");
    QTest::qWait(150);
    check(qAbs(value(w) - number) < 0.1, "parameter value updated");
}

static void exercise()
{
    auto *transform = qobject_cast<QToolButton *>(named(QStringLiteral("capthemeTransform")));
    check(transform, "inspector exists");
    editor = qobject_cast<QMainWindow *>(transform->window());
    check(editor, "editor window exists");
    editor->resize(1600, 1100);
    auto *bin = named(QStringLiteral("project_bin"));
    auto *tree = bin->findChild<QTreeView *>();
    check(tree && tree->model(), "project bin model exists");
    auto index = findClip(tree->model());
    check(index.isValid(), "real video imported");
    tree->setFocus();
    tree->setCurrentIndex(index);
    tree->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    QMetaObject::invokeMethod(tree, "doubleClicked", Q_ARG(QModelIndex, index));
    QTest::qWait(1500);
    const QString mediaKind = qEnvironmentVariable("CAPTHEME_TEST_MEDIA_KIND");
    if (!mediaKind.isEmpty()) {
        auto *tabs = qobject_cast<QTabWidget *>(named(QStringLiteral("capthemeCategories")));
        check(tabs, "media categories exist");
        const bool audioOnly = mediaKind == QStringLiteral("audio");
        check(tabs->isTabEnabled(0) == !audioOnly && tabs->isTabEnabled(2) == !audioOnly, "Video and Color follow actual media streams");
        check(tabs->isTabEnabled(1) == audioOnly, "Audio follows actual media streams");
        check(tabs->currentIndex() == (audioOnly ? 1 : 0), "compatible category is selected automatically");
        check(named(QStringLiteral("capthemeVolume"))->isEnabled() == audioOnly, "Volume follows selected clip audio availability");
        check(transform->isEnabled() == !audioOnly, "Transform follows selected clip video availability");
        std::_Exit(0);
    }
    check(transform->isEnabled(), "video selection enables Transform");
    check(named(QStringLiteral("capthemeVolume"))->isEnabled(), "AV selection enables Volume");
    transform->click();
    QTest::qWait(750);
    auto *panel = named(QStringLiteral("effect_stack"));
    auto *effects = panel->findChild<QTreeView *>();
    check(effects && effects->model()->rowCount() == 1, "Transform creates one effect");
    for (const auto &name : {"spinX", "spinY", "spinW", "spinH", "spinS", "spinO"}) {
        auto *w = named(QString::fromLatin1(name));
        check(w && w->isVisible(), name);
    }
    setValue(QStringLiteral("spinS"), 75);
    setValue(QStringLiteral("spinO"), 80);
    setValue(QStringLiteral("spinX"), 240);
    transform->click();
    QTest::qWait(250);
    check(effects->model()->rowCount() == 1, "reopening Transform does not duplicate it");
    check(value(named(QStringLiteral("spinX"))) == 240, "reopening preserves parameter values");
    auto *undo = action(QStringLiteral("edit_undo"));
    auto *redo = action(QStringLiteral("edit_redo"));
    check(undo && redo, "undo and redo actions available");
    undo->trigger();
    QTest::qWait(200);
    check(value(named(QStringLiteral("spinX"))) != 240, "position edit can be undone");
    redo->trigger();
    QTest::qWait(200);
    check(value(named(QStringLiteral("spinX"))) == 240, "position edit can be redone");
    QObject *keyframes = nullptr;
    for (auto *o : panel->findChildren<QObject *>()) if (o->inherits("KeyframeContainer")) { keyframes = o; break; }
    check(keyframes, "native keyframe controller exists");
    bool added = false;
    check(QMetaObject::invokeMethod(keyframes, "slotAddKeyframe", Q_RETURN_ARG(bool, added), Q_ARG(int, 25)) && added, "second keyframe created");
    check(QMetaObject::invokeMethod(keyframes, "slotSetPosition", Q_ARG(int, 25)), "seek to second keyframe");
    QTest::qWait(200);
    setValue(QStringLiteral("spinX"), 480);
    check(editor->screen()->grabWindow(editor->winId()).save(qEnvironmentVariable("CAPTHEME_SCREENSHOT")), "real inspector screenshot saved");

    auto *categories = qobject_cast<QTabWidget *>(named(QStringLiteral("capthemeCategories")));
    check(categories && categories->count() == 3, "Video, Audio and Color categories exist");
    const int baseline = effects->model()->rowCount();
    categories->setCurrentIndex(1);
    QTest::qWait(150);
    auto *volume = qobject_cast<QToolButton *>(named(QStringLiteral("capthemeVolume")));
    check(volume && volume->isVisible() && volume->isEnabled(), "Audio category exposes Volume");
    volume->click();
    QTest::qWait(300);
    check(effects->model()->rowCount() == baseline + 1, "Volume adds a native audio effect");
    volume->click();
    check(effects->model()->rowCount() == baseline + 1, "reopening Volume does not duplicate it");
    undo->trigger();
    QTest::qWait(200);
    check(effects->model()->rowCount() == baseline, "audio effect addition can be undone");
    redo->trigger();
    QTest::qWait(200);
    check(effects->model()->rowCount() == baseline + 1, "audio effect addition can be redone");
    categories->setCurrentIndex(2);
    auto *brightness = qobject_cast<QToolButton *>(named(QStringLiteral("capthemeBrightness")));
    check(brightness && brightness->isVisible() && brightness->isEnabled(), "Color category exposes Brightness");
    brightness->click();
    QTest::qWait(300);
    check(effects->model()->rowCount() == baseline + 2, "Brightness adds a native color effect");
    brightness->click();
    check(effects->model()->rowCount() == baseline + 2, "reopening Brightness does not duplicate it");
    categories->setCurrentIndex(0);
    transform->click();
    QTest::qWait(200);
    check(value(named(QStringLiteral("spinX"))) == 480, "switching categories preserves Transform keyframes");
    check(editor->screen()->grabWindow(editor->winId()).save(qEnvironmentVariable("CAPTHEME_SCREENSHOT")), "categorized inspector screenshot saved");

    auto *save = action(QStringLiteral("file_save_as"));
    check(save, "save action exists");
    qApp->setAttribute(Qt::AA_DontUseNativeDialogs);
    QTimer::singleShot(300, qApp, []() {
        for (QWidget *w : QApplication::topLevelWidgets()) {
            if (auto *dialog = qobject_cast<QFileDialog *>(w)) {
                const QFileInfo destination(qEnvironmentVariable("CAPTHEME_TEST_PROJECT"));
                dialog->setOption(QFileDialog::DontUseNativeDialog, true);
                dialog->setDirectory(destination.absolutePath());
                dialog->selectFile(destination.fileName());
                QTimer::singleShot(500, dialog, [dialog, destination]() {
                    auto *name = dialog->findChild<QLineEdit *>(QStringLiteral("fileNameEdit"));
                    check(name, "save filename editor exists");
                    name->setText(destination.fileName());
                    check(QMetaObject::invokeMethod(dialog, "accept"), "save dialog accepted");
                });
                return;
            }
        }
        check(false, "save dialog appeared");
    });
    save->trigger();
    QTest::qWait(500);
    check(QFile::exists(qEnvironmentVariable("CAPTHEME_TEST_PROJECT")), "edited project saved");
    check(editor->close(), "editor closed normally");
    qApp->quit();
}

static void start()
{
    if (!qEnvironmentVariableIsSet("CAPTHEME_INSPECTOR_TEST")) return;
    QCoreApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
    QTimer::singleShot(18000, qApp, exercise);
}
Q_COREAPP_STARTUP_FUNCTION(start)
