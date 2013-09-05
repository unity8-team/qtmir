// This file is part of QtUbuntu, a set of Qt components for Ubuntu.
// Copyright © 2013 Canonical Ltd.
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 3, as published by
// the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
// SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// FIXME(loicm) Desktop file loading should be executed on a dedicated I/O thread.

#include "application_manager.h"
#include "application.h"
#include "desktopdata.h"
#include "logging.h"
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>

#include <QRegularExpression>

using namespace unity::shell::application;

// Size of the side stage in grid units.
const int kSideStageWidth = 40;

// The time (in ms) to wait before closing a process that's not been matched by a new session.
const int kTimeBeforeClosingProcess = 10000;

class TaskEvent : public QEvent {
 public:
  enum Task { kAddApplication = 0, kRemoveApplication, kUnfocusApplication, kFocusApplication,
              kRequestFocus, kRequestFullscreen };
  TaskEvent(QString appId, int id, int stage, int task, QEvent::Type type)
      : QEvent(type)
      , appId_(appId)
      , id_(id)
      , stage_(stage)
      , task_(task) {
    DLOG("TaskEvent::TaskEvent (this=%p, appId='%s', id=%d, stage=%d, task=%d, type=%d)",
         this, qPrintable(appId), id, stage, task, type);
  }
  ~TaskEvent() {
    DLOG("TaskEvent::~TaskEvent");
  }
  QString appId_;
  int id_;
  int stage_;
  int task_;
};

// FIXME(kaleo, loicm): If we keep that keyboard geometry/visibilty API, we should integrate that
//     event type in the existing task event system.
class KeyboardGeometryEvent : public QEvent {
 public:
  KeyboardGeometryEvent(QRect geometry, QEvent::Type type)
      : QEvent(type)
      , geometry_(geometry) {
    DLOG("KeyboardGeometryEvent::KeyboardGeometryEvent (this=%p, type=%d)", this, type);
  }
  ~KeyboardGeometryEvent() {
    DLOG("KeyboardGeometryEvent::~KeyboardGeometryEvent");
  }

  QRect geometry_;
};

static void continueTask(int pid, void* context)
{
  DLOG("continueTask(pid=%d, context=%p)", pid, context);
  Q_UNUSED(context)
  kill(pid, SIGCONT);
}

static void suspendTask(int pid, void* context)
{
  DLOG("suspendTask(pid=%d, context=%p)", pid, context);
  Q_UNUSED(context)
  kill(pid, SIGSTOP);
}

static void sessionBornCallback(ubuntu_ui_session_properties session, void* context) {
  DLOG("sessionBornCallback (session=%p, context=%p)", session, context);
  DASSERT(context != NULL);
  // Post a task to be executed on the ApplicationManager thread (GUI thread).
  ApplicationManager* manager = static_cast<ApplicationManager*>(context);

  // Determine appId from the absolute path to the desktop file of the application
  QString appId = QString(ubuntu_ui_session_properties_get_desktop_file_hint(session))
          .split(QLatin1String("/"))
          .last()
          .remove(QRegularExpression("\\.desktop$"));

  QCoreApplication::postEvent(manager, new TaskEvent(
      appId,
      ubuntu_ui_session_properties_get_application_instance_id(session),
      ubuntu_ui_session_properties_get_application_stage_hint(session),
      TaskEvent::kAddApplication, manager->eventType()));
}

static void sessionDiedCallback(ubuntu_ui_session_properties session, void* context) {
  DLOG("sessionDiedCallback (session=%p, context=%p)", session, context);
  DASSERT(context != NULL);
  // Post a task to be executed on the ApplicationManager thread (GUI thread).
  ApplicationManager* manager = static_cast<ApplicationManager*>(context);
  QCoreApplication::postEvent(manager, new TaskEvent(
      QString(), ubuntu_ui_session_properties_get_application_instance_id(session),
      ubuntu_ui_session_properties_get_application_stage_hint(session),
      TaskEvent::kRemoveApplication, manager->eventType()));
}

static void sessionUnfocusedCallback(ubuntu_ui_session_properties session, void* context) {
  DLOG("sessionUnfocusedCallback (session=%p, context=%p)", session, context);
  DASSERT(context != NULL);
  // Post a task to be executed on the ApplicationManager thread (GUI thread).
  ApplicationManager* manager = static_cast<ApplicationManager*>(context);
  QCoreApplication::postEvent(manager, new TaskEvent(
      QString(), ubuntu_ui_session_properties_get_application_instance_id(session),
      ubuntu_ui_session_properties_get_application_stage_hint(session),
      TaskEvent::kUnfocusApplication, manager->eventType()));
}

static void sessionFocusedCallback(ubuntu_ui_session_properties session, void* context) {
  DLOG("sessionFocusedCallback (session=%p, context=%p)", session, context);
  DASSERT(context != NULL);
  // Post a task to be executed on the ApplicationManager thread (GUI thread).
  ApplicationManager* manager = static_cast<ApplicationManager*>(context);
  QCoreApplication::postEvent(manager, new TaskEvent(
      QString(), ubuntu_ui_session_properties_get_application_instance_id(session),
      ubuntu_ui_session_properties_get_application_stage_hint(session),
      TaskEvent::kFocusApplication, manager->eventType()));
}

static void sessionRequestedFullscreenCallback(
    ubuntu_ui_session_properties session, void* context) {
  DLOG("sessionRequestedFullscreenCallback (session=%p, context=%p)", session, context);
  DASSERT(context != NULL);
  // Post a task to be executed on the ApplicationManager thread (GUI thread).
  ApplicationManager* manager = static_cast<ApplicationManager*>(context);
  QCoreApplication::postEvent(manager, new TaskEvent(
      QString(), ubuntu_ui_session_properties_get_application_instance_id(session), 0,
      TaskEvent::kRequestFullscreen, manager->eventType()));
}

static void sessionRequestedCallback(ubuntu_ui_well_known_application application, void* context) {
  DLOG("sessionRequestedCallback (application=%d, context=%p)", application, context);
  DASSERT(context != NULL);
  // Post a task to be executed on the ApplicationManager thread (GUI thread).
  ApplicationManager* manager = static_cast<ApplicationManager*>(context);
  QCoreApplication::postEvent(manager, new TaskEvent(
      QString(), static_cast<int>(application), 0, TaskEvent::kRequestFocus, manager->eventType()));
}

static void keyboardGeometryChanged(int x, int y, int width, int height, void* context) {
  DLOG("keyboardGeometryChanged (x=%d, y=%d, width=%d, height=%d, context=%p)", x, y, width, height, context);
  DASSERT(context != NULL);
  // Post a task to be executed on the ApplicationManager thread (GUI thread).
  ApplicationManager* manager = static_cast<ApplicationManager*>(context);
  QCoreApplication::postEvent(manager,
        new KeyboardGeometryEvent(QRect(x, y, width, height),
                                  manager->keyboardGeometryEventType()));
}

ApplicationManager::ApplicationManager(QObject* parent)
    : ApplicationManagerInterface(parent)
    , keyboardHeight_(0)
    , keyboardVisible_(false)
    , pidHash_()
    , eventType_(static_cast<QEvent::Type>(QEvent::registerEventType()))
    , keyboardGeometryEventType_(static_cast<QEvent::Type>(QEvent::registerEventType())) {
  static int once = false;
  if (!once) {
    DLOG("starting application watcher");
    static ubuntu_ui_session_lifecycle_observer watcher = {
      sessionRequestedCallback, sessionBornCallback, sessionUnfocusedCallback,
      sessionFocusedCallback, keyboardGeometryChanged, sessionRequestedFullscreenCallback,
      sessionDiedCallback, this
    };
    ubuntu_ui_session_install_session_lifecycle_observer(&watcher);

    static ubuntu_ui_task_controller controller = {
        continueTask, suspendTask, this
    };
    ubuntu_ui_install_task_controller(&controller);

    once = true;
  }
  DLOG("ApplicationManager::ApplicationManager (this=%p)", this);
}

ApplicationManager::~ApplicationManager() {
  DLOG("ApplicationManager::~ApplicationManager");
  pidHash_.clear();
  for (auto app : applications_) {
      delete app;
  }
  applications_.clear();
}

int ApplicationManager::rowCount(const QModelIndex& parent) const {
  DLOG("ApplicationManager::rowCount (this=%p)", this);
  return !parent.isValid() ? applications_.size() : 0;
}

QVariant ApplicationManager::data(const QModelIndex& index, int role) const {
  DLOG("ApplicationManager::data (this=%p, role=%d)", this, role);
  if (index.row() < 0 || index.row() >= applications_.size())
    return QVariant();

  auto app = applications_.at(index.row());
  switch(role) {
  case RoleAppId:
      return app->appId();
  case RoleName:
      return app->name();
  case RoleComment:
      return app->comment();
  case RoleIcon:
      return app->icon();
  case RoleStage:
      return app->stage();
  case RoleState:
      return app->state();
  case RoleFocused:
      return app->focused();
  default:
      return QVariant();
  }
}

Application *ApplicationManager::get(int row) const {
  DLOG("ApplicationManager::get (this=%p, row=%d)", this, row);
  if (row < 0 || row >= applications_.size())
    return nullptr;

  return applications_.at(row);
}

Application *ApplicationManager::findApplication(const QString &appId) const {
  DLOG("ApplicationManager::findApplication (this=%p, appId=%s)", this, qPrintable(appId));
  for (Application *app : applications_) {
    if (app->appId() == appId) {
      return app;
    }
  }
  return nullptr;
}

void ApplicationManager::move(int from, int to) {
  DLOG("ApplicationManager::move (this=%p, from=%d, to=%d)", this, from, to);
  if (from == to) return;

  if (from >= 0 && from < applications_.size() && to >= 0 && to < applications_.size()) {
      QModelIndex parent;
    /* When moving an item down, the destination index needs to be incremented
       by one, as explained in the documentation:
       http://qt-project.org/doc/qt-5.0/qtcore/qabstractitemmodel.html#beginMoveRows */
    beginMoveRows(parent, from, from, parent, to + (to > from ? 1 : 0));
    applications_.move(from, to);
    endMoveRows();
  }
}

void ApplicationManager::killProcess(qint64 pid) {
  DLOG("ApplicationManager::kill (this=%p, pid=%lld)", this, pid);
#if !defined(QT_NO_DEBUG)
  int result = kill(static_cast<pid_t>(pid), SIGKILL);
  if (result != -1) {
    LOG("killed process with pid %lld", pid);
  } else {
    LOG("couldn't kill process with pid %lld: %s", pid, strerror(errno));
  }
#else
  kill(static_cast<pid_t>(pid), SIGKILL);
#endif
}

void ApplicationManager::customEvent(QEvent* event) {
  DLOG("ApplicationManager::customEvent (this=%p, event=%p)", this, event);
  DASSERT(QThread::currentThread() == thread());

  // FIXME(kaleo, loicm) If we keep that keyboard geometry/visibilty API, we should integrate that
  //     event type in the existing task event system. Moreover, Qt code shouldn't use C++ RTTI
  //     (which is slow) but the Qt meta object implementation.
  KeyboardGeometryEvent* keyboardGeometryEvent = dynamic_cast<KeyboardGeometryEvent*>(event);
  if (keyboardGeometryEvent != NULL) {
      bool visible = keyboardGeometryEvent->geometry_.isValid();
      int height = keyboardGeometryEvent->geometry_.height();
      if (height != keyboardHeight_) {
          keyboardHeight_ = height;
          emit keyboardHeightChanged();
      }
      if (visible != keyboardVisible_) {
          keyboardVisible_ = visible;
          emit keyboardVisibleChanged();
      }
      return;
  }

  TaskEvent* taskEvent = static_cast<TaskEvent*>(event);
  switch (taskEvent->task_) {

    case TaskEvent::kAddApplication: {
      DLOG("handling add application task");
      const int kPid = taskEvent->id_;
      Application* application = pidHash_.value(kPid, NULL);
      if (application) {
        DLOG("got a match in the application lists, setting '%s' (%d) to running", qPrintable(application->name()), kPid);
#if !defined(QT_NO_DEBUG)
        // Ensure we're in sync with Ubuntu Platform.
        ASSERT(applications_.contains(application));
#endif
        application->setState(Application::Running);
        killTimer(application->timerId());
      } else {
        DLOG("didn't get a match in the application lists, loading the desktop file");
        DesktopData* desktopData = new DesktopData(taskEvent->appId_);
        if (desktopData->loaded()) {
          Application* application = new Application(
              desktopData, kPid, Application::MainStage, Application::Running, -1);
          pidHash_.insert(kPid, application);
          DLOG("desktopFile loaded, storing '%s' (%d) in the application list", qPrintable(desktopData->name()), kPid);
          add(application);
        } else {
          DLOG("unknown application, not storing in the application lists");
          delete desktopData;
        }
      }
      break;
    }

    case TaskEvent::kRemoveApplication: {
      DLOG("handling remove application task");
      const int kPid = taskEvent->id_;
      Application* application = pidHash_.take(kPid);
      if (application != NULL) {
        DLOG("removing application '%s' (%d) from the application lists", qPrintable(application->name()), kPid);
        if (application->state() == Application::Starting) {
          killTimer(application->timerId());
        }

        remove(application);
        if (application->focused()) {
          emit focusedApplicationIdChanged();
        }
        application->deleteLater();
      } else {
        DLOG("Unknown application, not stored in the application lists");
      }
      break;
    }

    case TaskEvent::kUnfocusApplication: {
      DLOG("handling unfocus application task");
      // Reset the currently focused application.
      Application* application = pidHash_.value(taskEvent->id_);
      if (application != NULL) {
        application->setFocused(false);
        emit focusedApplicationIdChanged();
      }
      break;
    }

    case TaskEvent::kFocusApplication: {
      DLOG("handling focus application task");
      // Update the currently focused application.
      Application* application = pidHash_.value(taskEvent->id_);
      if (application != NULL) {
        application->setFocused(true);
        //move application to top of applications_ list
        int index = applications_.indexOf(application);
        this->move(index, 0);
        emit focusedApplicationIdChanged();
      }
      break;
    }

    case TaskEvent::kRequestFullscreen: {
      DLOG("handling request fullscreen task");
      Application* application = pidHash_.value(taskEvent->id_);
      if (application != NULL) {
        application->setFullscreen(true);
      }
      break;
    }

    case TaskEvent::kRequestFocus: {
      DLOG("handling request focus task");
      emit focusRequested(static_cast<FavoriteApplication>(taskEvent->id_));
      break;
    }

    default: {
      DNOT_REACHED();
      break;
    }
  }
}

void ApplicationManager::timerEvent(QTimerEvent* event) {
  DLOG("ApplicationManager::timerEvent (this=%p, event=%p)", this, event);

  const int kTimerId = event->timerId();
  Application* application = findFromTimerId(kTimerId);

  // Remove application from list and kill it.
  if (application != NULL) {
    const qint64 kPid = application->pid();
    DLOG("application '%s' (%lld) hasn't been matched, killing it", qPrintable(application->name()), kPid);
    DASSERT(pidHash_.contains(kPid));
    pidHash_.remove(kPid);
    remove(application);
    delete application;
    killProcess(kPid);
  }
  killTimer(kTimerId);
}

int ApplicationManager::keyboardHeight() const {
  DLOG("ApplicationManager::keyboardHeight (this=%p)", this);
  return keyboardHeight_;
}

bool ApplicationManager::keyboardVisible() const {
  DLOG("ApplicationManager::keyboardVisible (this=%p)", this);
  return keyboardVisible_;
}

int ApplicationManager::sideStageWidth() const {
  DLOG("ApplicationManager::sideStageWidth (this=%p)", this);
  return kSideStageWidth;
}

bool ApplicationManager::focusApplication(const QString &appId) {
  Application *application = this->findApplication(appId);
  if (application == nullptr)
    return false;

  DLOG("ApplicationManager::focusApplication (this=%p, app_pid=%lld)", this, application->pid());
  ubuntu_ui_session_focus_running_session_with_id(application->pid());
  return true;
}

void ApplicationManager::focusFavoriteApplication(
    ApplicationManager::FavoriteApplication application) {
  DLOG("ApplicationManager::focusFavoriteApplication (this=%p, application=%d)",
       this, static_cast<int>(application));
  ubuntu_ui_session_trigger_switch_to_well_known_application(
      static_cast<ubuntu_ui_well_known_application>(application));
}

void ApplicationManager::unfocusCurrentApplication() {
  DLOG("ApplicationManager::unfocusCurrentApplication (this=%p)", this);
  // FIXME(loicm): Add that once supported in Ubuntu Platform API.
  // ubuntu_ui_session_unfocus_running_sessions(static_cast<StageHint>(stageHint));
  ubuntu_ui_session_unfocus_running_sessions();
}

QString ApplicationManager::focusedApplicationId() const {
  for (Application *app : applications_) {
    if (app->focused())
      return app->appId();
  }
  return QString();
}

Application *ApplicationManager::startApplication(const QString &appId, const QStringList &arguments) {
    return startApplication(appId, NoFlag, arguments);
}

Application *ApplicationManager::startApplication(const QString &appId, ApplicationManager::ExecFlags flags,
                                                  const QStringList &arguments) {
  DLOG("ApplicationManager::startProcess (this=%p, flags=%d)", this, (int) flags);
  // Load desktop file.
  DesktopData* desktopData = new DesktopData(appId);
  if (!desktopData->loaded()) {
    delete desktopData;
    return NULL;
  }

  QStringList argumentsCopy = arguments;

  // Format arguments.
  // FIXME(loicm) Special field codes are simply ignored for now.
  //     http://standards.freedesktop.org/desktop-entry-spec/latest/ar01s06.html
  QStringList execArguments = desktopData->exec().split(" ", QString::SkipEmptyParts);
  DASSERT(execArguments.size() > 0);
  QString exec(execArguments[0]);
  const int kSize = execArguments.size();
  for (int i = kSize - 1; i > 0; i--) {
    if ((execArguments[i].size() == 2) && (execArguments[i][0].toLatin1() == '%')) {
      const char kChar = execArguments[i][1].toLatin1();
      if (kChar == 'F' || kChar == 'u' || kChar == 'U' || kChar == 'd' || kChar == 'D'
          || kChar == 'n' || kChar == 'N' || kChar == 'i' || kChar == 'c' || kChar == 'k'
          || kChar == 'v' || kChar == 'm') {
        continue;
      }
    }
    argumentsCopy.prepend(execArguments[i]);
  }
  argumentsCopy.append(QString("--desktop_file_hint=") + desktopData->file());
  if (flags.testFlag(ApplicationManager::ForceMainStage))
    argumentsCopy.append(QString("--stage_hint=main_stage"));
  else if (desktopData->stageHint() == "SideStage")
    argumentsCopy.append(QString("--stage_hint=side_stage"));

#if !defined(QT_NO_DEBUG)
  LOG("starting process '%s' with arguments:", qPrintable(exec));
  for (int i = 0; i < argumentsCopy.size(); i++)
    LOG("  '%s'", qPrintable(argumentsCopy[i]));
#endif

  // Start process.
  bool result;
  qint64 pid = 0;
  QString path = "/";
  // respect Path from .desktop file
  if (desktopData->path() != "") {
    path = desktopData->path();
  } else {
    struct passwd* passwd = getpwuid(getuid());
    if (passwd)
      path = passwd->pw_dir;
  }
  DLOG("current working directory: '%s'", qPrintable(path));
  QByteArray envSetAppId = QString("APP_ID=%1").arg(appId).toLocal8Bit();
  putenv(envSetAppId.data()); // envSetAppId must be available and unmodified until the env var is unset
  result = QProcess::startDetached(exec, argumentsCopy, path, &pid);
  QByteArray envClearAppId = QString("APP_ID").toLocal8Bit();
  putenv(envClearAppId.data()); // now it's safe to deallocate envSetAppId.
  DLOG_IF(result == false, "process failed to start");
  if (result == true) {
    DLOG("started process with pid %lld, adding '%s' to application lists", pid, qPrintable(desktopData->name()));

    //decide stage
    Application::Stage stage = Application::MainStage;
    if (desktopData->stageHint() == "SideStage" && !flags.testFlag(ApplicationManager::ForceMainStage)) {
        stage = Application::SideStage;
    }

    Application* application = new Application(
        desktopData, pid, stage, Application::Starting,
        startTimer(kTimeBeforeClosingProcess));
    pidHash_.insert(pid, application);

    add(application);
    return application;
  } else {
    return nullptr;
  }
}

bool ApplicationManager::stopApplication(const QString &appId) {
  DLOG("ApplicationManager::stopProcess (this=%p, application=%p)", this, qPrintable(appId));

  Application *application = this->findApplication(appId);
  if (application == nullptr)
    return false;

  const qint64 kPid = application->pid();
  if (pidHash_.remove(kPid) > 0) {
    remove(application);
    application->deleteLater();
    killProcess(kPid);
  }
  return true;
}

void ApplicationManager::add(Application* application) {
  DASSERT(application != NULL);
  DLOG("ApplicationManager::add (this=%p, application='%s')", this, qPrintable(application->name()));

#if !defined(QT_NO_DEBUG)
  for (int i = 0; i < applications_.size(); i++)
    ASSERT(applications_.at(i) != application);
#endif
  beginInsertRows(QModelIndex(), applications_.size(), applications_.size());
  applications_.append(application);
  endInsertRows();
  emit countChanged();
}

void ApplicationManager::remove(Application *application) {
  DASSERT(application != NULL);
  DLOG("ApplicationManager::remove (this=%p, application='%s')", this, qPrintable(application->name()));

  int i = applications_.indexOf(application);
  if (i != -1) {
    beginRemoveRows(QModelIndex(), i, i);
    applications_.removeAt(i);
    endRemoveRows();
    emit countChanged();
  }
}

Application* ApplicationManager::findFromTimerId(int timerId) {
  DLOG("ApplicationManager::findFromTimerId (this=%p, timerId=%d)", this, timerId);

  const int kSize = applications_.size();
  for (int i = 0; i < kSize; i++)
    if (applications_[i]->timerId() == timerId)
      return applications_[i];
  return NULL;
}
