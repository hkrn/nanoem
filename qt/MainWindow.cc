/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "MainWindow.h"

#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QGuiApplication>
#include <QHideEvent>
#include <QKeyEvent>
#include <QMenuBar>
#include <QMimeData>
#include <QMouseEvent>
#include <QOpenGLBuffer>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QProgressDialog>
#include <QSemaphore>
#include <QShowEvent>
#include <QTimer>
#include <QUrl>
#include <QWheelEvent>

#include "QtApplicationClient.h"
#include "QtApplicationMenuBuilder.h"
#include "QtApplicationService.h"

#include "bx/commandline.h"
#include "bx/os.h"

namespace nanoem {
namespace qt {

MainWindow::CentralWidget::CentralWidget(MainWindow *window)
    : QOpenGLWidget(window)
    , m_renderLoopTimer(new QTimer(this))
    , m_parent(window)
{
    connect(m_renderLoopTimer, &QTimer::timeout, [=]() { update(); });
}

MainWindow::CentralWidget::~CentralWidget()
{
}

void
MainWindow::CentralWidget::initializeGL()
{
    const Vector2UI16 logicalWindowSize(width(), height());
    QDir dir(QGuiApplication::applicationDirPath());
    const char *suffix = format().renderableType() == QSurfaceFormat::OpenGLES ? "gles3" : "glcore33";
#if defined(Q_OS_MACOS)
    BX_UNUSED_1(suffix);
    const QString path("../PlugIns/sokol_glcore33.dylib");
#elif defined(QT_OS_WIN32)
    const QString path(QString::asprintf("plugins/sokol_%s.dll", suffix));
#else
    const QString path(QString::asprintf("plugins/sokol_%s.so", suffix));
#endif
    const QByteArray pluginPathUTF8(dir.absoluteFilePath(path).toUtf8());
    QtApplicationClient::InitializeMessageDescription desc(
        logicalWindowSize, SG_PIXELFORMAT_RGBA8, devicePixelRatio(), pluginPathUTF8.constData());
    m_parent->m_client->sendInitializeMessage(desc);
    m_renderLoopTimer->start();
}

void
MainWindow::CentralWidget::paintGL()
{
    QtApplicationClient *client = m_parent->m_client;
    QtApplicationService *service = m_parent->m_service;
    service->draw();
    service->consumeDispatchAllCommandMessages();
    service->dispatchAllEventMessages();
    client->consumeDispatchAllEventMessages();
}

void
MainWindow::CentralWidget::resizeGL(int x, int y)
{
    m_parent->m_client->sendResizeWindowMessage(Vector2UI16(x, y));
}

MainWindow::MainWindow(const bx::CommandLine *commands, QtApplicationService *service, QtApplicationClient *client)
    : QMainWindow()
    , m_command(commands)
    , m_client(client)
    , m_service(service)
    , m_menu(new QtApplicationMenuBuilder(this, client, service->translator()))
    , m_translatorPtr(service->translator())
{
    const Vector2UI16 size(BaseApplicationService::minimumRequiredWindowSize());
    setAcceptDrops(true);
    CentralWidget *centerWidget = new CentralWidget(this);
    setCentralWidget(centerWidget);
    setMinimumSize(QSize(size.x, size.y));
    setWindowTitle(QCoreApplication::applicationName());
    registerAllPrerequisiteEventListeners();
    QGuiApplication::setFallbackSessionManagementEnabled(false);
    m_menu->build();
    setMouseTracking(true);
    centerWidget->setMouseTracking(true);
}

MainWindow::~MainWindow()
{
    delete m_menu;
    m_menu = nullptr;
}

void
MainWindow::openProgressDialog(const QString &title, const QString &message)
{
    if (!m_progressDialog) {
        QProgressDialog *dialog = new QProgressDialog(this);
        dialog->setWindowModality(Qt::WindowModal);
        dialog->setWindowTitle(title);
        dialog->setLabelText(message);
        dialog->open();
        m_progressDialog = dialog;
    }
}

void
MainWindow::closeProgressDialog()
{
    if (m_progressDialog) {
        m_progressDialog->close();
        delete m_progressDialog;
        m_progressDialog = nullptr;
    }
}

bool
MainWindow::confirmBeforeClose()
{
    m_client->sendIsProjectDirtyRequestMessage(
        [](void *userData, bool dirty) {
            auto self = static_cast<MainWindow *>(userData);
            QtApplicationClient *client = self->m_client;
            if (dirty) {
                client->clearAllProjectAfterConfirmOnceEventListeners();
                client->addSaveProjectAfterConfirmEventListener(
                    [](void *userData) {
                        auto self = static_cast<MainWindow *>(userData);
                        self->m_client->addCompleteSavingFileEventListener(
                            [](void *userData, const URI &fileURI, uint32_t type, uint64_t ticks) {
                                BX_UNUSED_3(fileURI, type, ticks);
                                auto self = static_cast<MainWindow *>(userData);
                                self->sendDestroyMessage();
                            },
                            self, true);
                        self->m_menu->saveProject();
                    },
                    self, true);
                client->addDiscardProjectAfterConfirmEventListener(
                    [](void *userData) {
                        auto self = static_cast<MainWindow *>(userData);
                        self->sendDestroyMessage();
                    },
                    self, true);
                client->sendConfirmBeforeExitApplicationMessage();
            }
            else {
                self->sendDestroyMessage();
            }
        },
        this);
    return m_state == CloseTransitionComplete;
}

int
MainWindow::translateKey(const QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Space:
        return BaseApplicationService::kKeyType_SPACE;
    case Qt::Key_Apostrophe:
        return BaseApplicationService::kKeyType_APOSTROPHE;
    case Qt::Key_Comma:
        return BaseApplicationService::kKeyType_COMMA;
    case Qt::Key_Minus:
        return BaseApplicationService::kKeyType_MINUS;
    case Qt::Key_Period:
        return BaseApplicationService::kKeyType_PERIOD;
    case Qt::Key_Slash:
        return BaseApplicationService::kKeyType_SLASH;
    case Qt::Key_0:
        return BaseApplicationService::kKeyType_0;
    case Qt::Key_1:
        return BaseApplicationService::kKeyType_1;
    case Qt::Key_2:
        return BaseApplicationService::kKeyType_2;
    case Qt::Key_3:
        return BaseApplicationService::kKeyType_3;
    case Qt::Key_4:
        return BaseApplicationService::kKeyType_4;
    case Qt::Key_5:
        return BaseApplicationService::kKeyType_5;
    case Qt::Key_6:
        return BaseApplicationService::kKeyType_6;
    case Qt::Key_7:
        return BaseApplicationService::kKeyType_7;
    case Qt::Key_8:
        return BaseApplicationService::kKeyType_8;
    case Qt::Key_9:
        return BaseApplicationService::kKeyType_9;
    case Qt::Key_Semicolon:
        return BaseApplicationService::kKeyType_SEMICOLON;
    case Qt::Key_Equal:
        return BaseApplicationService::kKeyType_EQUAL;
    case Qt::Key_A:
        return BaseApplicationService::kKeyType_A;
    case Qt::Key_B:
        return BaseApplicationService::kKeyType_B;
    case Qt::Key_C:
        return BaseApplicationService::kKeyType_C;
    case Qt::Key_D:
        return BaseApplicationService::kKeyType_D;
    case Qt::Key_E:
        return BaseApplicationService::kKeyType_E;
    case Qt::Key_F:
        return BaseApplicationService::kKeyType_F;
    case Qt::Key_G:
        return BaseApplicationService::kKeyType_G;
    case Qt::Key_H:
        return BaseApplicationService::kKeyType_H;
    case Qt::Key_I:
        return BaseApplicationService::kKeyType_I;
    case Qt::Key_J:
        return BaseApplicationService::kKeyType_J;
    case Qt::Key_K:
        return BaseApplicationService::kKeyType_K;
    case Qt::Key_L:
        return BaseApplicationService::kKeyType_L;
    case Qt::Key_M:
        return BaseApplicationService::kKeyType_M;
    case Qt::Key_N:
        return BaseApplicationService::kKeyType_N;
    case Qt::Key_O:
        return BaseApplicationService::kKeyType_O;
    case Qt::Key_P:
        return BaseApplicationService::kKeyType_P;
    case Qt::Key_Q:
        return BaseApplicationService::kKeyType_Q;
    case Qt::Key_R:
        return BaseApplicationService::kKeyType_R;
    case Qt::Key_S:
        return BaseApplicationService::kKeyType_S;
    case Qt::Key_T:
        return BaseApplicationService::kKeyType_T;
    case Qt::Key_U:
        return BaseApplicationService::kKeyType_U;
    case Qt::Key_V:
        return BaseApplicationService::kKeyType_V;
    case Qt::Key_W:
        return BaseApplicationService::kKeyType_W;
    case Qt::Key_X:
        return BaseApplicationService::kKeyType_X;
    case Qt::Key_Y:
        return BaseApplicationService::kKeyType_Y;
    case Qt::Key_Z:
        return BaseApplicationService::kKeyType_Z;
    case Qt::Key_BraceLeft:
        return BaseApplicationService::kKeyType_LEFT_BRACKET;
    case Qt::Key_Backslash:
        return BaseApplicationService::kKeyType_BACKSLASH;
    case Qt::Key_BracketRight:
        return BaseApplicationService::kKeyType_RIGHT_BRACKET;
    // case Qt::Key_GRAVE_ACCENT: return BaseApplicationService::kKeyType_GRAVE_ACCENT;
    // case Qt::Key_WORLD_1: return BaseApplicationService::kKeyType_WORLD_1;
    // case Qt::Key_WORLD_2: return BaseApplicationService::kKeyType_WORLD_2;
    case Qt::Key_Escape:
        return BaseApplicationService::kKeyType_ESCAPE;
    case Qt::Key_Enter:
        return BaseApplicationService::kKeyType_ENTER;
    case Qt::Key_Tab:
        return BaseApplicationService::kKeyType_TAB;
    case Qt::Key_Backspace:
        return BaseApplicationService::kKeyType_BACKSPACE;
    case Qt::Key_Insert:
        return BaseApplicationService::kKeyType_INSERT;
    case Qt::Key_Delete:
        return BaseApplicationService::kKeyType_DELETE;
    case Qt::Key_Right:
        return BaseApplicationService::kKeyType_RIGHT;
    case Qt::Key_Left:
        return BaseApplicationService::kKeyType_LEFT;
    case Qt::Key_Down:
        return BaseApplicationService::kKeyType_DOWN;
    case Qt::Key_Up:
        return BaseApplicationService::kKeyType_UP;
    case Qt::Key_PageUp:
        return BaseApplicationService::kKeyType_PAGE_UP;
    case Qt::Key_PageDown:
        return BaseApplicationService::kKeyType_PAGE_DOWN;
    case Qt::Key_Home:
        return BaseApplicationService::kKeyType_HOME;
    case Qt::Key_End:
        return BaseApplicationService::kKeyType_END;
    case Qt::Key_CapsLock:
        return BaseApplicationService::kKeyType_CAPS_LOCK;
    case Qt::Key_ScrollLock:
        return BaseApplicationService::kKeyType_SCROLL_LOCK;
    case Qt::Key_NumLock:
        return BaseApplicationService::kKeyType_NUM_LOCK;
    case Qt::Key_Print:
        return BaseApplicationService::kKeyType_PRINT_SCREEN;
    case Qt::Key_Pause:
        return BaseApplicationService::kKeyType_PAUSE;
    case Qt::Key_F1:
        return BaseApplicationService::kKeyType_F1;
    case Qt::Key_F2:
        return BaseApplicationService::kKeyType_F2;
    case Qt::Key_F3:
        return BaseApplicationService::kKeyType_F3;
    case Qt::Key_F4:
        return BaseApplicationService::kKeyType_F4;
    case Qt::Key_F5:
        return BaseApplicationService::kKeyType_F5;
    case Qt::Key_F6:
        return BaseApplicationService::kKeyType_F6;
    case Qt::Key_F7:
        return BaseApplicationService::kKeyType_F7;
    case Qt::Key_F8:
        return BaseApplicationService::kKeyType_F8;
    case Qt::Key_F9:
        return BaseApplicationService::kKeyType_F9;
    case Qt::Key_F10:
        return BaseApplicationService::kKeyType_F10;
    case Qt::Key_F11:
        return BaseApplicationService::kKeyType_F11;
    case Qt::Key_F12:
        return BaseApplicationService::kKeyType_F12;
    case Qt::Key_F13:
        return BaseApplicationService::kKeyType_F13;
    case Qt::Key_F14:
        return BaseApplicationService::kKeyType_F14;
    case Qt::Key_F15:
        return BaseApplicationService::kKeyType_F15;
    case Qt::Key_F16:
        return BaseApplicationService::kKeyType_F16;
    case Qt::Key_F17:
        return BaseApplicationService::kKeyType_F17;
    case Qt::Key_F18:
        return BaseApplicationService::kKeyType_F18;
    case Qt::Key_F19:
        return BaseApplicationService::kKeyType_F19;
    case Qt::Key_F20:
        return BaseApplicationService::kKeyType_F20;
    case Qt::Key_F21:
        return BaseApplicationService::kKeyType_F21;
    case Qt::Key_F22:
        return BaseApplicationService::kKeyType_F22;
    case Qt::Key_F23:
        return BaseApplicationService::kKeyType_F23;
    case Qt::Key_F24:
        return BaseApplicationService::kKeyType_F24;
    case Qt::Key_F25:
        return BaseApplicationService::kKeyType_F25;
    // case Qt::Key_KP_0: return BaseApplicationService::kKeyType_KP_0;
    // case Qt::Key_KP_1: return BaseApplicationService::kKeyType_KP_1;
    // case Qt::Key_KP_2: return BaseApplicationService::kKeyType_KP_2;
    // case Qt::Key_KP_3: return BaseApplicationService::kKeyType_KP_3;
    // case Qt::Key_KP_4: return BaseApplicationService::kKeyType_KP_4;
    // case Qt::Key_KP_5: return BaseApplicationService::kKeyType_KP_5;
    // case Qt::Key_KP_6: return BaseApplicationService::kKeyType_KP_6;
    // case Qt::Key_KP_7: return BaseApplicationService::kKeyType_KP_7;
    // case Qt::Key_KP_8: return BaseApplicationService::kKeyType_KP_8;
    // case Qt::Key_KP_9: return BaseApplicationService::kKeyType_KP_9;
    // case Qt::Key_KP_DECIMAL: return BaseApplicationService::kKeyType_KP_DECIMAL;
    // case Qt::Key_KP_DIVIDE: return BaseApplicationService::kKeyType_KP_DIVIDE;
    // case Qt::Key_KP_MULTIPLY: return BaseApplicationService::kKeyType_KP_MULTIPLY;
    // case Qt::Key_KP_SUBTRACT: return BaseApplicationService::kKeyType_KP_SUBTRACT;
    // case Qt::Key_KP_ADD: return BaseApplicationService::kKeyType_KP_ADD;
    // case Qt::Key_KP_ENTER: return BaseApplicationService::kKeyType_KP_ENTER;
    // case Qt::Key_KP_EQUAL: return BaseApplicationService::kKeyType_KP_EQUAL;
    // case Qt::Key_LEFT_SHIFT: return BaseApplicationService::kKeyType_LEFT_SHIFT;
    // case Qt::Key_LEFT_CONTROL: return BaseApplicationService::kKeyType_LEFT_CONTROL;
    // case Qt::Key_LEFT_ALT: return BaseApplicationService::kKeyType_LEFT_ALT;
    // case Qt::Key_LEFT_SUPER: return BaseApplicationService::kKeyType_LEFT_SUPER;
    // case Qt::Key_RIGHT_SHIFT: return BaseApplicationService::kKeyType_RIGHT_SHIFT;
    // case Qt::Key_RIGHT_CONTROL: return BaseApplicationService::kKeyType_RIGHT_CONTROL;
    // case Qt::Key_RIGHT_ALT: return BaseApplicationService::kKeyType_RIGHT_ALT;
    // case Qt::Key_RIGHT_SUPER: return BaseApplicationService::kKeyType_RIGHT_SUPER;
    case Qt::Key_Menu:
        return BaseApplicationService::kKeyType_MENU;
    default:
        return 0;
    }
}

int
MainWindow::translateCursorModifiers(const QMouseEvent *event)
{
    const Qt::KeyboardModifiers m = event->modifiers();
    int mods = 0;
    if (m & Qt::AltModifier) {
        mods |= Project::kCursorModifierTypeAlt;
    }
    if (m & Qt::ShiftModifier) {
        mods |= Project::kCursorModifierTypeShift;
    }
    if (m & Qt::ControlModifier) {
        mods |= Project::kCursorModifierTypeControl;
    }
    return mods;
}

Project::CursorType
MainWindow::translateCursorType(const QMouseEvent *event)
{
    switch (event->button()) {
    case Qt::LeftButton:
        return Project::kCursorTypeMouseLeft;
    case Qt::MidButton:
        return Project::kCursorTypeMouseMiddle;
    case Qt::RightButton:
        return Project::kCursorTypeMouseRight;
    case Qt::XButton1:
        return Project::kCursorTypeMouseButton4;
    case Qt::XButton2:
        return Project::kCursorTypeMouseButton5;
    default:
        return Project::kCursorTypeFirstEnum;
    }
}

void
MainWindow::closeEvent(QCloseEvent *event)
{
    confirmBeforeClose() ? event->accept() : event->ignore();
}

void
MainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        for (const auto &url : mimeData->urls()) {
            const QByteArray &path = url.path().toUtf8();
            m_client->sendDropFileMessage(URI::createFromFilePath(path.constData()));
        }
        event->acceptProposedAction();
    }
}

void
MainWindow::hideEvent(QHideEvent * /* event */)
{
    m_client->sendDeactivateMessage();
}

void
MainWindow::keyPressEvent(QKeyEvent *event)
{
    m_client->sendKeyPressMessage(translateKey(event));
}

void
MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    m_client->sendKeyReleaseMessage(translateKey(event));
}

void
MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    Vector2SI32 cursor, delta;
    if (getCursorPosition(event, cursor, delta)) {
        m_client->sendCursorMoveMessage(cursor, delta, translateCursorType(event), translateCursorModifiers(event));
        setLastCursorPosition(cursor, delta);
    }
}

void
MainWindow::mousePressEvent(QMouseEvent *event)
{
    Vector2SI32 cursor, delta;
    if (getCursorPosition(event, cursor, delta)) {
        m_client->sendCursorPressMessage(cursor, translateCursorType(event), translateCursorModifiers(event));
        setLastCursorPosition(cursor);
    }
}

void
MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    Vector2SI32 cursor, delta;
    if (getCursorPosition(event, cursor, delta)) {
        m_client->sendCursorReleaseMessage(cursor, translateCursorType(event), translateCursorModifiers(event));
        setLastCursorPosition(Vector2());
    }
}

void
MainWindow::showEvent(QShowEvent * /* event */)
{
    m_client->sendActivateMessage();
}

void
MainWindow::wheelEvent(QWheelEvent *event)
{
    const Vector2SI32 pos(event->x(), event->y());
    const Vector2SI32 delta(0, event->delta());
    m_client->sendCursorScrollMessage(pos, delta, 0);
}

void
MainWindow::getWindowCenterPoint(Vector2SI32 *value)
{
    const QRect &r = rect();
    value->x = static_cast<int>(r.width() * 0.5f);
    value->y = static_cast<int>(r.height() * 0.5f);
}

bool
MainWindow::getCursorPosition(const QMouseEvent *event, Vector2SI32 &position, Vector2SI32 &delta)
{
    bool result = false;
    if (m_cursorHidden.first) {
        const Vector2SI32 cursorPosition(event->x(), event->y() - menuBar()->height());
        position = m_virtualCursorPosition;
        delta = cursorPosition - m_lastCursorPosition;
        result = true;
    }
    else if (qApp->activeWindow() == this) {
        const Vector2SI32 cursorPosition(event->x(), event->y() - menuBar()->height());
        position = cursorPosition;
        delta = position - m_lastCursorPosition;
        result = true;
    }
    return result;
}

void
MainWindow::recenterCursorPosition()
{
    if (m_cursorHidden.first) {
        Vector2SI32 value;
        getWindowCenterPoint(&value);
        if (m_lastCursorPosition != value) {
            m_lastCursorPosition = value;
        }
    }
}

glm::vec2
MainWindow::lastCursorPosition() const
{
    return m_lastCursorPosition;
}

void
MainWindow::setLastCursorPosition(const Vector2SI32 &value)
{
    m_lastCursorPosition = value;
}

void
MainWindow::setLastCursorPosition(const Vector2SI32 &value, const Vector2SI32 &delta)
{
    m_virtualCursorPosition += delta;
    m_lastCursorPosition = value;
}

void
MainWindow::disableCursor(const Vector2SI32 &position)
{
    Vector2SI32 centerPoint;
    internalHideCursor(centerPoint);
    m_virtualCursorPosition = position;
    m_lastCursorPosition = centerPoint;
    m_restoreHiddenCursorPosition = position;
    m_cursorHidden.first = true;
}

void
MainWindow::enableCursor(const Vector2SI32 &position)
{
    BX_UNUSED_1(position);
    internalShowCursor(m_restoreHiddenCursorPosition);
    m_restoreHiddenCursorPosition = Vector2SI32();
    m_cursorHidden.first = false;
}

void
MainWindow::internalHideCursor(Vector2SI32 &location)
{
    BX_UNUSED_1(location);
}

void
MainWindow::internalShowCursor(const Vector2SI32 &location)
{
    BX_UNUSED_1(location);
}

bool
MainWindow::sendDestroyMessage()
{
    QtApplicationClient *client = m_client;
    if (m_state == CloseTransitionReady) {
        m_state = CloseTransitionRequestDestroy;
        client->sendConfirmBeforeExitApplicationMessage();
        openProgressDialog(m_translatorPtr->translate("nanoem.dialog.progress.exit.title"),
            m_translatorPtr->translate("nanoem.dialog.progress.exit.message"));
        client->addCompleteDestructionEventListener(
            [](void *userData) {
                auto self = static_cast<MainWindow *>(userData);
                self->m_client->addCompleteTerminationEventListener(
                    [](void *userData) {
                        auto self = static_cast<MainWindow *>(userData);
                        self->m_state = CloseTransitionComplete;
                        self->closeProgressDialog();
                        self->close();
                    },
                    self, true);
                self->m_state = CloseTransitionRequestTerminate;
                self->m_client->sendTerminateMessage();
            },
            this, true);
        client->sendDestroyMessage();
    }
    return m_state == CloseTransitionComplete;
}

void
MainWindow::registerAllPrerequisiteEventListeners()
{
    m_client->addInitializationCompleteEventListener(
        [](void *userData) {
            auto self = static_cast<MainWindow *>(userData);
            const bx::CommandLine *cmd = self->m_command;
            QtApplicationClient *client = self->m_client;
            QDir dir(QGuiApplication::applicationDirPath());
#if defined(Q_OS_MACOS)
            dir.cd("../Plugins");
#else
            dir.cd("plugins");
#endif
            URIList plugins;
            for (auto item : dir.entryList(QStringList() << "*." BX_DL_EXT)) {
                const QByteArray &path = dir.absoluteFilePath(item).toUtf8();
                plugins.push_back(URI::createFromFilePath(path.constData()));
            }
            client->sendLoadAllModelPluginsMessage(plugins);
            client->sendLoadAllMotionPluginsMessage(plugins);
            if (cmd->hasArg("bootstrap-project-from-clipboard")) {
                QClipboard *clipboard = QGuiApplication::clipboard();
                const QMimeData *mimeData = clipboard->mimeData();
                for (const auto &item : mimeData->urls()) {
                    const QByteArray &path = item.path().toUtf8();
                    const URI &fileURI = URI::createFromFilePath(path.constData());
                    client->sendLoadFileMessage(fileURI, IFileManager::kDialogTypeOpenProject);
                }
            }
            else if (cmd->hasArg("bootstrap-project")) {
                const URI &fileURI = URI::createFromFilePath(cmd->findOption("bootstrap-project"));
                if (FileUtils::exists(fileURI)) {
                    client->sendLoadFileMessage(fileURI, IFileManager::kDialogTypeOpenProject);
                }
            }
            else if (cmd->hasArg("recovery-from")) {
                const URI &fileURI = URI::createFromFilePath(cmd->findOption("recovery-from"));
                if (FileUtils::exists(fileURI)) {
                    client->sendRecoveryMessage(fileURI);
                }
            }
        },
        this, true);
    m_client->addTrackEventListener(
        [](void *userData, const char *screen, const char *category, const char *action, const char *label) {
            auto self = static_cast<MainWindow *>(userData);
            BX_UNUSED_5(self, screen, category, action, label);
        },
        this, false);
    m_client->addErrorEventListener(
        [](void *userData, int code, const char *reason, const char *recoverySuggestion) {
            BX_UNUSED_1(userData);
            qWarning("code=%d, reason=\"%s\", suggestion=\"%s\"", code, reason, recoverySuggestion);
        },
        this, false);
    m_client->addDisableCursorEventListener(
        [](void *userData, const Vector2SI32 &coord) {
            auto self = static_cast<MainWindow *>(userData);
            self->disableCursor(coord);
        },
        this, false);
    m_client->addEnableCursorEventListener(
        [](void *userData, const Vector2SI32 &coord) {
            auto self = static_cast<MainWindow *>(userData);
            self->enableCursor(coord);
        },
        this, false);
    m_client->addSetPreferredEditingFPSEvent(
        [](void *userData, uint32_t value) {
            auto self = static_cast<MainWindow *>(userData);
            BX_UNUSED_2(self, value);
            // self->m_preference->setPreferredEditingFPS(value);
        },
        this, false);
    m_client->addStopRecordingViewportPassEventListener(
        [](void *userData) {
            auto self = static_cast<MainWindow *>(userData);
            BX_UNUSED_1(self);
        },
        this, false);
    m_client->addQueryOpenSingleFileDialogEventListener(
        [](void *userData, uint32_t type, const StringList &allowedExtensions) {
            auto self = static_cast<MainWindow *>(userData);
            QFileDialog *dialog = new QFileDialog(self);
            dialog->setAcceptMode(QFileDialog::AcceptOpen);
            dialog->setFileMode(QFileDialog::ExistingFile);
            dialog->setNameFilters(QtApplicationMenuBuilder::toNameFilter(allowedExtensions));
            connect(dialog, &QFileDialog::accepted, [=] {
                const QByteArray &path = dialog->selectedUrls().first().path().toUtf8();
                self->m_client->sendQueryOpenSingleFileDialogMessage(type, URI::createFromFilePath(path.constData()));
            });
            connect(dialog, &QFileDialog::finished, [=](int) { self->setFocus(); });
            dialog->open();
        },
        this, false);
    m_client->addQueryOpenMultipleFilesDialogEventListener(
        [](void *userData, uint32_t type, const StringList &allowedExtensions) {
            auto self = static_cast<MainWindow *>(userData);
            QFileDialog *dialog = new QFileDialog(self);
            dialog->setAcceptMode(QFileDialog::AcceptOpen);
            dialog->setFileMode(QFileDialog::ExistingFiles);
            dialog->setNameFilters(QtApplicationMenuBuilder::toNameFilter(allowedExtensions));
            connect(dialog, &QFileDialog::accepted, [=] {
                URIList fileURIs;
                for (const auto &item : dialog->selectedUrls()) {
                    const QByteArray &path = item.path().toUtf8();
                    const URI &fileURI = URI::createFromFilePath(path.constData());
                    fileURIs.push_back(fileURI);
                }
                self->m_client->sendQueryOpenMultipleFilesDialogMessage(type, fileURIs);
            });
            connect(dialog, &QFileDialog::finished, [=](int) { self->setFocus(); });
            dialog->open();
        },
        this, false);
    m_client->addQuerySaveFileDialogEventListener(
        [](void *userData, uint32_t type, const StringList &allowedExtensions) {
            auto self = static_cast<MainWindow *>(userData);
            QFileDialog *dialog = new QFileDialog(self);
            dialog->setAcceptMode(QFileDialog::AcceptSave);
            dialog->setNameFilters(QtApplicationMenuBuilder::toNameFilter(allowedExtensions));
            connect(dialog, &QFileDialog::accepted, [=] {
                const QByteArray &path = dialog->selectedUrls().first().path().toUtf8();
                self->m_client->sendQuerySaveFileDialogMessage(type, URI::createFromFilePath(path.constData()));
            });
            connect(dialog, &QFileDialog::finished, [=](int) { self->setFocus(); });
            dialog->open();
        },
        this, false);
    m_client->addUpdateProgressEventListener(
        [](void *userData, uint32_t value, uint32_t total, uint32_t type, const char *text) {
            auto self = static_cast<MainWindow *>(userData);
            if (QProgressDialog *progressDialog = self->m_progressDialog) {
                progressDialog->setValue(int(value));
                progressDialog->setMaximum(int(total));
                if (text) {
                    switch (type) {
                    case 3:
                        progressDialog->setLabelText(text);
                        break;
                    case 4:
                        progressDialog->setLabelText(QString::asprintf("Loading Next Item... %s", text));
                        break;
                    }
                }
            }
        },
        this, false);
    auto playEventHandler = [](void *userData, nanoem_frame_index_t /* duration */,
                                nanoem_frame_index_t /* frameIndex */) {
        auto self = static_cast<MainWindow *>(userData);
        BX_UNUSED_1(self);
        // self->setDisplaySyncEnabled(self->m_disableVSync ? NO : YES);
    };
    auto stopEventHandler = [](void *userData, nanoem_frame_index_t /* duration */,
                                nanoem_frame_index_t /* frameIndex */) {
        auto self = static_cast<MainWindow *>(userData);
        BX_UNUSED_1(self);
        // const Preference *preference = self->m_preference;
        // self->setDisplaySyncEnabled(preference->preferredEditingFPS() != UINT32_MAX);
    };
    m_client->addSetPreferredMotionFPSEvent(
        [](void *userData, nanoem_u32_t /* fps */, bool unlimited) {
            auto self = static_cast<MainWindow *>(userData);
            BX_UNUSED_2(self, unlimited);
            // self->m_disableVSync = unlimited;
        },
        this, false);
    m_client->addPlayEvent(playEventHandler, this, false);
    m_client->addPauseEvent(stopEventHandler, this, false);
    m_client->addResumeEvent(playEventHandler, this, false);
    m_client->addStopEvent(stopEventHandler, this, false);
}

} /* namespace qt */
} /* namespace nanoem */
