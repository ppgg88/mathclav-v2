#include "MainWindow.h"

#include "FormulaView.h"
#include "widgets/CreditsDialog.h"
#include "widgets/GrapherDialog.h"
#include "widgets/HelpDialog.h"
#include "widgets/HistoryDialog.h"
#include "widgets/MultiChoicePopup.h"
#include "widgets/RaptorEasterEgg.h"
#include "widgets/UpdateCheckDialog.h"

#include "mathclav/core/Version.h"
#include "mathclav/core/latex/CaretSplice.h"
#include "mathclav/core/session/SessionStore.h"
#include "mathclav/core/settings/SettingsSchema.h"

#include <QApplication>
#include <QButtonGroup>
#include <QCloseEvent>
#include <QColor>
#include <QDebug>
#include <QFont>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPalette>
#include <QPushButton>
#include <QStandardPaths>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>

namespace latex = mathclav::core::latex;
namespace session = mathclav::core::session;
namespace settings = mathclav::core::settings;
namespace input = mathclav::app::input;

namespace {

QString modeLabelText(input::Mode mode) {
    switch (mode) {
        case input::Mode::Normal: return QStringLiteral("Lettre Usuelle");
        case input::Mode::Math: return QStringLiteral("Mode Math");
        case input::Mode::Greek: return QStringLiteral("Lettre Grecque");
    }
    return {};
}

// One accent color per mode, reused for both the status pill and the
// matching mode-selector button's "checked" state so the two always agree
// visually at a glance -- picked to echo legacy's own blue/red/green mode
// colors (globals.py) rather than introducing a new palette.
QColor modeAccentColor(input::Mode mode) {
    switch (mode) {
        case input::Mode::Normal: return QColor(0x3a, 0x7c, 0xc7);
        case input::Mode::Math: return QColor(0xd9, 0x53, 0x4f);
        case input::Mode::Greek: return QColor(0x4c, 0xaf, 0x50);
    }
    return Qt::gray;
}

// Central QSS for the whole window: rounded, padded buttons with visible
// hover/pressed states (default QPushButton styling is flat and cramped),
// plus a distinct "checked" look for the mode-selector buttons so the
// active mode is identifiable without reading the pill label. Regenerated
// per theme toggle rather than trying to make one stylesheet answer to
// both via palette roles, since the checked-button accent color is a
// fixed brand color, not theme-dependent.
QString buttonStyleSheet(bool dark) {
    const QString base = dark ? QStringLiteral("#3c3c3c") : QStringLiteral("#e0e0e0");
    const QString hover = dark ? QStringLiteral("#4a4a4a") : QStringLiteral("#d0d0d0");
    const QString pressed = dark ? QStringLiteral("#2f2f2f") : QStringLiteral("#c8c8c8");
    const QString border = dark ? QStringLiteral("#555555") : QStringLiteral("#bbbbbb");
    const QString text = dark ? QStringLiteral("white") : QStringLiteral("black");
    return QStringLiteral(
               "QPushButton {"
               "  background-color: %1; color: %5; border: 1px solid %4;"
               "  border-radius: 6px; padding: 6px 14px; }"
               "QPushButton:hover { background-color: %2; }"
               "QPushButton:pressed { background-color: %3; }"
               "QPushButton:checked { background-color: #3a7cc7; color: white; border: 1px solid #2a5c99; }")
        .arg(base, hover, pressed, border, text);
}

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("MathClav v%1")
                       .arg(QString::fromUtf8(mathclav::core::version().data())));
    resize(1000, 620);
    setMinimumSize(640, 400);

    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);
    layout->setContentsMargins(12, 10, 12, 10);
    layout->setSpacing(8);

    modeLabel_ = new QLabel(modeLabelText(input::Mode::Normal), central);
    modeLabel_->setAlignment(Qt::AlignCenter);
    QFont modeFont = modeLabel_->font();
    modeFont.setBold(true);
    modeFont.setPointSize(modeFont.pointSize() + 1);
    modeLabel_->setFont(modeFont);
    modeLabel_->setFixedHeight(28);
    layout->addWidget(modeLabel_);

    formulaView_ = new FormulaView(central);
    formulaView_->setObjectName(QStringLiteral("formulaCanvas"));
    layout->addWidget(formulaView_, /*stretch=*/1);

    // NoFocus on every toolbar button: matches legacy's `takefocus=False`
    // on all of its ttk.Button()s (index.py 417-433), and for the same
    // reason -- without it, clicking (or even the initial show()) can
    // leave keyboard focus sitting on a button, which then intercepts
    // Left/Right arrow keys itself for its own focus-navigation instead of
    // ever reaching MainWindow::keyPressEvent. This is what a real user
    // report described as "arrows don't work once I'm inside an exponent"
    // -- CursorOps itself was never the problem (see the plan doc's
    // post-Phase-7 bug notes).
    auto* toolbar = new QHBoxLayout();
    toolbar->setSpacing(6);

    auto* copyButton = new QPushButton(QStringLiteral("Copier"), central);
    copyButton->setFocusPolicy(Qt::NoFocus);
    copyButton->setToolTip(QStringLiteral("Copier le code LaTeX (Ctrl+C)"));
    connect(copyButton, &QPushButton::clicked, this, [this] {
        dispatcher_.copyToClipboard();
        statusBar()->showMessage(QStringLiteral("Copié dans le presse-papier"), 2000);
    });
    toolbar->addWidget(copyButton);

    auto* clearButton = new QPushButton(QStringLiteral("Effacer Tout"), central);
    clearButton->setFocusPolicy(Qt::NoFocus);
    clearButton->setToolTip(QStringLiteral("Effacer tout le document"));
    connect(clearButton, &QPushButton::clicked, this, [this] {
        clearDocument();
        statusBar()->showMessage(QStringLiteral("Document effacé"), 2000);
    });
    toolbar->addWidget(clearButton);

    auto* themeButton = new QPushButton(QStringLiteral("Thème"), central);
    themeButton->setFocusPolicy(Qt::NoFocus);
    themeButton->setToolTip(QStringLiteral("Basculer entre thème clair et sombre"));
    connect(themeButton, &QPushButton::clicked, this, &MainWindow::toggleTheme);
    toolbar->addWidget(themeButton);

    toolbar->addSpacing(10);

    // Three direct-selection buttons instead of legacy's (and this
    // rewrite's own first draft's) single cycling "Mode" button: a
    // deliberate ergonomics improvement requested by the user -- the
    // active mode is visible via the checked/accented button, and any
    // other mode is one click away instead of up to two blind clicks
    // through a cycle. Keyboard toggles ("²", Ctrl) stay fully available
    // and keep these buttons in sync via onModeChanged.
    auto* modeGroup = new QButtonGroup(this);
    modeGroup->setExclusive(true);

    normalModeButton_ = new QPushButton(QStringLiteral("Usuel"), central);
    normalModeButton_->setCheckable(true);
    normalModeButton_->setFocusPolicy(Qt::NoFocus);
    normalModeButton_->setToolTip(QStringLiteral("Mode Lettre Usuelle (²)"));
    connect(normalModeButton_, &QPushButton::clicked, this,
            [this] { dispatcher_.setModeDirect(input::Mode::Normal); });
    modeGroup->addButton(normalModeButton_);
    toolbar->addWidget(normalModeButton_);

    mathModeButton_ = new QPushButton(QStringLiteral("Math"), central);
    mathModeButton_->setCheckable(true);
    mathModeButton_->setFocusPolicy(Qt::NoFocus);
    mathModeButton_->setToolTip(QStringLiteral("Mode Math (²)"));
    connect(mathModeButton_, &QPushButton::clicked, this,
            [this] { dispatcher_.setModeDirect(input::Mode::Math); });
    modeGroup->addButton(mathModeButton_);
    toolbar->addWidget(mathModeButton_);

    greekModeButton_ = new QPushButton(QStringLiteral("Grec"), central);
    greekModeButton_->setCheckable(true);
    greekModeButton_->setFocusPolicy(Qt::NoFocus);
    greekModeButton_->setToolTip(QStringLiteral("Mode Lettre Grecque (Ctrl)"));
    connect(greekModeButton_, &QPushButton::clicked, this,
            [this] { dispatcher_.setModeDirect(input::Mode::Greek); });
    modeGroup->addButton(greekModeButton_);
    toolbar->addWidget(greekModeButton_);

    toolbar->addSpacing(10);

    auto* graphButton = new QPushButton(QStringLiteral("Graph"), central);
    graphButton->setFocusPolicy(Qt::NoFocus);
    graphButton->setToolTip(QStringLiteral("Tracer le graphique (F10)"));
    connect(graphButton, &QPushButton::clicked, this, [this] {
        if (dispatcher_.onGraphRequested) dispatcher_.onGraphRequested();
    });
    toolbar->addWidget(graphButton);

    toolbar->addStretch(1);

    auto* helpButton = new QPushButton(QStringLiteral("Aide ?"), central);
    helpButton->setFocusPolicy(Qt::NoFocus);
    helpButton->setToolTip(QStringLiteral("Afficher l'aide"));
    connect(helpButton, &QPushButton::clicked, this, &MainWindow::openHelpDialog);
    toolbar->addWidget(helpButton);

    layout->addLayout(toolbar);

    setCentralWidget(central);
    setFocusPolicy(Qt::StrongFocus);
    statusBar()->setSizeGripEnabled(false);

    // Anchored to formulaView_ specifically (not the whole window): it's a
    // child widget positioned at the bottom of its anchor's own bounds
    // (MultiChoicePopup.cpp), so anchoring to the full MainWindow would
    // put it right on top of the toolbar instead of just below the
    // editing surface.
    multiChoicePopup_ = new MultiChoicePopup(formulaView_);

    dispatcher_.onChanged = [this] { updateRender(); };
    dispatcher_.onModeChanged = [this](input::Mode mode) { updateModeLabel(mode); };
    dispatcher_.onMultiChoice = [this](const std::vector<mathclav::core::ast::Node>& candidates, int activeIndex) {
        multiChoicePopup_->showCandidates(candidates, activeIndex);
    };
    dispatcher_.onMultiChoiceShouldHide = [this] { multiChoicePopup_->hide(); };
    dispatcher_.onGraphRequested = [this] {
        auto* dialog = new GrapherDialog(dispatcher_.document(), this);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->show();
    };
    dispatcher_.onHistoryRequested = [this] { openHistoryDialog(); };
    dispatcher_.onCreditsRequested = [this] {
        auto* dialog = new CreditsDialog(this);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->show();
    };
    dispatcher_.onRaptorTriggered = [this] {
        auto* dialog = new RaptorEasterEgg(this);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->show();
    };

    const settings::Settings loaded = settings::loadSettings(dataFilePath(QStringLiteral("settings.json")));
    darkTheme_ = loaded.theme != QStringLiteral("light");
    applyTheme();
    updateModeLabel(input::Mode::Normal);

    mathclav::core::ast::Container restored;
    if (session::loadSession(dataFilePath(QStringLiteral("session-autosave.json")), restored)) {
        dispatcher_.loadDocument(std::move(restored));
    }

    updateRender();

    // Matches legacy's update.update_q(self) call at the end of __init__
    // (index.py line 360), minus the silent-download-and-run half -- see
    // UpdateCheckDialog.h.
    auto* updateCheck = new UpdateCheckDialog(this, this);
    updateCheck->checkNow();
}

QString MainWindow::dataFilePath(const QString& fileName) const {
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1Char('/') + fileName;
}

void MainWindow::openHistoryDialog() {
    auto* dialog = new HistoryDialog(dispatcher_.document(), dataFilePath(QStringLiteral("history.json")), this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialog, &HistoryDialog::entryInsertRequested, this,
            [this](const mathclav::core::ast::Container& document) { dispatcher_.insertHistoryEntry(document); });
    dialog->show();
}

void MainWindow::openHelpDialog() {
    auto* dialog = new HelpDialog(darkTheme_, dataFilePath(QStringLiteral("settings.json")), this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void MainWindow::clearDocument() {
    // Ported from index.py's clear() (436-450): a fresh empty document,
    // same as at first launch. Session-autosave.json is only ever written
    // on close, so clearing and then quitting is what actually persists
    // this -- matching legacy, which likewise only pickles last.pkl in
    // quiter().
    dispatcher_.loadDocument({});
}

void MainWindow::toggleTheme() {
    darkTheme_ = !darkTheme_;
    applyTheme();

    settings::Settings current = settings::loadSettings(dataFilePath(QStringLiteral("settings.json")));
    current.theme = darkTheme_ ? QStringLiteral("dark") : QStringLiteral("light");
    if (!settings::saveSettings(dataFilePath(QStringLiteral("settings.json")), current)) {
        qWarning("MainWindow: failed to write settings.json");
    }
}

void MainWindow::applyTheme() {
    // A real, visible app-wide palette switch (window chrome, buttons,
    // labels), plus a QSS layer for rounded/padded buttons and a themed
    // "canvas" border around the formula area, plus the main canvas's own
    // glyph color (FormulaView::setForegroundColor -- MultiChoicePopup's
    // own candidate-preview FormulaViews are untouched, they always sit
    // on a fixed light background regardless of app theme). Darkening the
    // canvas background without this would leave near-black glyphs on a
    // near-black background -- caught by screenshot during this pass, not
    // simply skipped.
    QPalette palette;
    if (darkTheme_) {
        palette.setColor(QPalette::Window, QColor(0x2b, 0x2b, 0x2b));
        palette.setColor(QPalette::WindowText, Qt::white);
        palette.setColor(QPalette::Base, QColor(0x1e, 0x1e, 0x1e));
        palette.setColor(QPalette::Text, Qt::white);
        palette.setColor(QPalette::Button, QColor(0x3c, 0x3c, 0x3c));
        palette.setColor(QPalette::ButtonText, Qt::white);
    } else {
        palette.setColor(QPalette::Window, QColor(0xf0, 0xf0, 0xf0));
        palette.setColor(QPalette::WindowText, Qt::black);
        palette.setColor(QPalette::Base, Qt::white);
        palette.setColor(QPalette::Text, Qt::black);
        palette.setColor(QPalette::Button, QColor(0xe0, 0xe0, 0xe0));
        palette.setColor(QPalette::ButtonText, Qt::black);
    }
    qApp->setPalette(palette);
    qApp->setStyleSheet(buttonStyleSheet(darkTheme_));

    formulaView_->setStyleSheet(darkTheme_
                                     ? QStringLiteral("#formulaCanvas { background: #1e1e1e; border: 1px solid "
                                                       "#444444; border-radius: 8px; }")
                                     : QStringLiteral("#formulaCanvas { background: white; border: 1px solid "
                                                       "#cccccc; border-radius: 8px; }"));
    formulaView_->setForegroundColor(darkTheme_ ? 0xffe8e8e8u : 0xff202020u);
}

void MainWindow::updateRender() {
    const std::wstring latexSource = latex::renderWithCaret(dispatcher_.document(), dispatcher_.cursorPath());
    formulaView_->setFormula(latexSource);
}

void MainWindow::updateModeLabel(input::Mode mode) {
    modeLabel_->setText(modeLabelText(mode));
    const QColor accent = modeAccentColor(mode);
    modeLabel_->setStyleSheet(QStringLiteral("QLabel { background-color: %1; color: white; "
                                              "border-radius: 6px; padding: 4px 10px; }")
                                   .arg(accent.name()));

    normalModeButton_->setChecked(mode == input::Mode::Normal);
    mathModeButton_->setChecked(mode == input::Mode::Math);
    greekModeButton_->setChecked(mode == input::Mode::Greek);
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    if (dispatcher_.dispatch(event)) {
        return;
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    const QString path = dataFilePath(QStringLiteral("session-autosave.json"));
    if (!session::saveSession(path, dispatcher_.document())) {
        qWarning("MainWindow: failed to write session autosave to %s", qPrintable(path));
    }
    QMainWindow::closeEvent(event);
}
