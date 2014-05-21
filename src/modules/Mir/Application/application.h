/*
 * Copyright (C) 2013-2014 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef APPLICATION_H
#define APPLICATION_H

// std
#include <memory>

//Qt
#include <QtCore/QtCore>
#include <QImage>
#include <QSharedPointer>

// Unity API
#include <unity/shell/application/ApplicationInfoInterface.h>

namespace mir { namespace scene { class Session; }}

namespace qtmir
{

class DesktopFileReader;
class TaskController;

class Application : public unity::shell::application::ApplicationInfoInterface
{
    Q_OBJECT
    Q_PROPERTY(QString desktopFile READ desktopFile CONSTANT)
    Q_PROPERTY(QString exec READ exec CONSTANT)
    Q_PROPERTY(bool fullscreen READ fullscreen NOTIFY fullscreenChanged)
    Q_PROPERTY(Stage stage READ stage WRITE setStage NOTIFY stageChanged)

public:
    Application(const QSharedPointer<TaskController>& taskController,
                DesktopFileReader *desktopFileReader,
                State state,
                const QStringList &arguments,
                QObject *parent = 0);
    virtual ~Application();

    // ApplicationInfoInterface
    QString appId() const override;
    QString name() const override;
    QString comment() const override;
    QUrl icon() const override;
    Stage stage() const override;
    Stages supportedStages() const override;
    State state() const override;
    bool focused() const override;
    QUrl screenshot() const override;

    bool setStage(const Stage stage) override;

    QImage screenshotImage() const;
    void updateScreenshot();

    bool canBeResumed() const;
    void setCanBeResumed(const bool);

    bool isValid() const;
    QString desktopFile() const;
    QString exec() const;
    bool fullscreen() const;
    std::shared_ptr<::mir::scene::Session> session() const;
    pid_t pid() const;

public Q_SLOTS:
    void suspend();
    void resume();
    void respawn();

Q_SIGNALS:
    void fullscreenChanged();
    void stageChanged(Stage stage);

private:
    QString longAppId() const;
    void setPid(pid_t pid);
    void setState(State state);
    void setFocused(bool focus);
    void setFullscreen(bool fullscreen);
    void setSession(const std::shared_ptr<::mir::scene::Session>& session);
    void setSessionName(const QString& name);

    QSharedPointer<TaskController> m_taskController;
    DesktopFileReader* m_desktopData;
    QString m_longAppId;
    qint64 m_pid;
    Stage m_stage;
    Stages m_supportedStages;
    State m_state;
    bool m_focused;
    QUrl m_screenshot;
    QImage m_screenshotImage;
    bool m_canBeResumed;
    bool m_fullscreen;
    std::shared_ptr<mir::scene::Session> m_session;
    QString m_sessionName;
    QStringList m_arguments;
    QTimer* m_suspendTimer;

    friend class ApplicationManager;
    friend class MirSurfaceManager;
};

} // namespace qtmir

Q_DECLARE_METATYPE(qtmir::Application*)

#endif  // APPLICATION_H
