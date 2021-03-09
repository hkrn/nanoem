/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_QT_MAINWINDOW_H_
#define NANOEM_EMAPP_QT_MAINWINDOW_H_

#include "emapp/Project.h"

#include <QMainWindow>
#include <QOpenGLWidget>
#include <QString>

class QOpenGLContext;
class QOpenGLBuffer;
class QOpenGLShaderProgram;
class QOpenGLVertexArrayObject;
class QProgressDialog;
class QSemaphore;

namespace bx {
class CommandLine;
}

namespace nanoem {

class ITranslator;

namespace qt {

class QtApplicationClient;
class QtApplicationMenuBuilder;
class QtApplicationService;

class MainWindow final : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(const bx::CommandLine *commands, QtApplicationService *service, QtApplicationClient *client);
    ~MainWindow();

    void openProgressDialog(const QString &title, const QString &message);
    void closeProgressDialog();
    bool confirmBeforeClose();

private:
    class CentralWidget : public QOpenGLWidget {
    public:
        CentralWidget(MainWindow *window);
        ~CentralWidget();

    private:
        void initializeGL() override;
        void paintGL() override;
        void resizeGL(int x, int y) override;

        QTimer *m_renderLoopTimer;
        MainWindow *m_parent;
    };
    enum CloseTransitionState {
        CloseTransitionReady,
        CloseTransitionRequestDestroy,
        CloseTransitionRequestTerminate,
        CloseTransitionComplete,
    };

    static int translateKey(const QKeyEvent *event);
    static int translateCursorModifiers(const QMouseEvent *event);
    static Project::CursorType translateCursorType(const QMouseEvent *event);

    void closeEvent(QCloseEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    void getWindowCenterPoint(Vector2SI32 *value);
    bool getCursorPosition(const QMouseEvent *event, Vector2SI32 &position, Vector2SI32 &delta);
    void recenterCursorPosition();
    glm::vec2 lastCursorPosition() const;
    void setLastCursorPosition(const Vector2SI32 &value);
    void setLastCursorPosition(const Vector2SI32 &value, const Vector2SI32 &delta);
    void disableCursor(const Vector2SI32 &position);
    void enableCursor(const Vector2SI32 &position);
    void internalHideCursor(Vector2SI32 &location);
    void internalShowCursor(const Vector2SI32 &location);
    bool sendDestroyMessage();
    void registerAllPrerequisiteEventListeners();

    const bx::CommandLine *m_command;
    QtApplicationClient *m_client;
    QtApplicationService *m_service;
    QtApplicationMenuBuilder *m_menu;
    ITranslator *m_translatorPtr;
    QProgressDialog *m_progressDialog = nullptr;
    CloseTransitionState m_state = CloseTransitionReady;
    Vector2SI32 m_lastCursorPosition;
    Vector2SI32 m_virtualCursorPosition;
    Vector2SI32 m_restoreHiddenCursorPosition;
    tinystl::pair<bool, bool> m_cursorHidden = tinystl::make_pair(false, false);
};

} /* namespace qt */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_QT_MAINWINDOW_H_ */
