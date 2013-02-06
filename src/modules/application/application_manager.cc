// This file is part of QtUbuntu, a set of Qt components for Ubuntu.
// Copyright © 2013 Canonical Ltd.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; version 3.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// FIXME(loicm) Desktop file loading should be executed on a dedicated I/O thread.

#include "application_manager.h"
#include "application_list_model.h"
#include "application.h"
#include "logging.h"
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>

// Retrieves the size of an array at compile time.
#define ARRAY_SIZE(a) \
    ((sizeof(a) / sizeof(*(a))) / static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))

// The time (in ms) to wait before closing a process that's not been matched by a new session.
const int kTimeBeforeClosingProcess = 10000;

class TaskEvent : public QEvent {
 public:
  enum Task { kAddApplication = 0, kRemoveApplication, kUnfocusApplication, kFocusApplication,
              kRequestFocus };
  TaskEvent(char* desktopFile, int id, int task, QEvent::Type type)
      : QEvent(type)
      , desktopFile_(desktopFile)
      , id_(id)
      , task_(task) {
    DLOG("TaskEvent::TaskEvent (this=%p, desktopFile='%s', id=%d, task=%d, type=%d)",
         this, desktopFile, id, task, type);
  }
  ~TaskEvent() {
    DLOG("TaskEvent::~TaskEvent");
    delete [] desktopFile_;
  }
  char* desktopFile_;
  int id_;
  int task_;
};

static void sessionBornCallback(ubuntu_ui_session_properties session, void* context) {
  DLOG("sessionBornCallback (session=%p, context=%p)", session, context);
  DASSERT(context != NULL);
  // Post a task to be executed on the ApplicationManager thread (GUI thread).
  ApplicationManager* manager = static_cast<ApplicationManager*>(context);
  QCoreApplication::postEvent(manager, new TaskEvent(
      qstrdup(ubuntu_ui_session_properties_get_desktop_file_hint(session)),
      ubuntu_ui_session_properties_get_application_instance_id(session),
      TaskEvent::kAddApplication, manager->eventType()));
}

static void sessionDiedCallback(ubuntu_ui_session_properties session, void* context) {
  DLOG("sessionDiedCallback (session=%p, context=%p)", session, context);
  DASSERT(context != NULL);
  // Post a task to be executed on the ApplicationManager thread (GUI thread).
  ApplicationManager* manager = static_cast<ApplicationManager*>(context);
  QCoreApplication::postEvent(manager, new TaskEvent(
      NULL, ubuntu_ui_session_properties_get_application_instance_id(session),
      TaskEvent::kRemoveApplication, manager->eventType()));
}

static void sessionUnfocusedCallback(ubuntu_ui_session_properties session, void* context) {
  DLOG("sessionUnfocusedCallback (session=%p, context=%p)", session, context);
  DASSERT(context != NULL);
  // Post a task to be executed on the ApplicationManager thread (GUI thread).
  ApplicationManager* manager = static_cast<ApplicationManager*>(context);
  QCoreApplication::postEvent(manager, new TaskEvent(
      NULL, ubuntu_ui_session_properties_get_application_instance_id(session),
      TaskEvent::kUnfocusApplication, manager->eventType()));
}

static void sessionFocusedCallback(ubuntu_ui_session_properties session, void* context) {
  DLOG("sessionFocusedCallback (session=%p, context=%p)", session, context);
  DASSERT(context != NULL);
  // Post a task to be executed on the ApplicationManager thread (GUI thread).
  ApplicationManager* manager = static_cast<ApplicationManager*>(context);
  QCoreApplication::postEvent(manager, new TaskEvent(
      NULL, ubuntu_ui_session_properties_get_application_instance_id(session),
      TaskEvent::kFocusApplication, manager->eventType()));
}

static void sessionRequestedCallback(ubuntu_ui_well_known_application application, void* context) {
  DLOG("sessionRequestedCallback (application=%d, context=%p)", application, context);
  DASSERT(context != NULL);
  // Post a task to be executed on the ApplicationManager thread (GUI thread).
  ApplicationManager* manager = static_cast<ApplicationManager*>(context);
  QCoreApplication::postEvent(manager, new TaskEvent(
      NULL, static_cast<int>(application), TaskEvent::kRequestFocus, manager->eventType()));
}

DesktopData::DesktopData(QString desktopFile)
    : file_(desktopFile)
    , entries_(DesktopData::kNumberOfEntries, "") {
  DLOG("DesktopData::DesktopData (this=%p, desktopFile='%s')", this, desktopFile.toLatin1().data());
  DASSERT(desktopFile != NULL);
  loaded_ = loadDesktopFile(desktopFile);
}

DesktopData::~DesktopData() {
  DLOG("DesktopData::~DesktopData");
  entries_.clear();
}

bool DesktopData::loadDesktopFile(QString desktopFile) {
  DLOG("DesktopData::loadDesktopFile (this=%p, desktopFile='%s')",
       this, desktopFile.toLatin1().data());
  DASSERT(desktopFile != NULL);
  const struct { const char* const name; int size; unsigned int flag; } kEntryNames[] = {
    { "Name=",    sizeof("Name=") - 1,    1 << DesktopData::kNameIndex },
    { "Comment=", sizeof("Comment=") - 1, 1 << DesktopData::kCommentIndex },
    { "Icon=",    sizeof("Icon=") - 1,    1 << DesktopData::kIconIndex },
    { "Exec=",    sizeof("Exec=") - 1,    1 << DesktopData::kExecIndex }
  };
  const unsigned int kAllEntriesMask =
      (1 << DesktopData::kNameIndex) | (1 << DesktopData::kCommentIndex)
      | (1 << DesktopData::kIconIndex) | (1 << DesktopData::kExecIndex);
  const unsigned int kMandatoryEntriesMask =
      (1 << DesktopData::kNameIndex) | (1 << DesktopData::kIconIndex)
      | (1 << DesktopData::kExecIndex);
  const int kEntriesCount = ARRAY_SIZE(kEntryNames);
  const int kBufferSize = 256;
  static char buffer[kBufferSize];
  QFile file(desktopFile);

  // Open file.
  if (!file.open(QFile::ReadOnly | QIODevice::Text)) {
    DLOG("can't open file: %s", file.errorString().toLatin1().data());
    return false;
  }

  // Validate "magic key" (standard group header).
  if (file.readLine(buffer, kBufferSize) != -1) {
    if (strncmp(buffer, "[Desktop Entry]", sizeof("[Desktop Entry]" - 1))) {
      DLOG("not a desktop file");
      return false;
    }
  }

  int length;
  unsigned int entryFlags = 0;
  while ((length = file.readLine(buffer, kBufferSize)) != -1) {
    // Skip empty lines.
    if (length > 1) {
      // Stop when reaching unsupported next group header.
      if (buffer[0] == '[') {
        DLOG("reached next group header, leaving loop");
        break;
      }
      // Lookup entries ignoring duplicates if any.
      for (int i = 0; i < kEntriesCount; i++) {
        if (!strncmp(buffer, kEntryNames[i].name, kEntryNames[i].size)) {
          if (~entryFlags & kEntryNames[i].flag) {
            buffer[length-1] = '\0';
            entries_[i] = QString::fromLatin1(&buffer[kEntryNames[i].size]);
            entryFlags |= kEntryNames[i].flag;
            break;
          }
        }
      }
      // Stop when matching the right number of entries.
      if (entryFlags == kAllEntriesMask) {
        break;
      }
    }
  }

  // Check that the mandatory entries are set.
  if ((entryFlags & kMandatoryEntriesMask) == kMandatoryEntriesMask) {
    DLOG("loaded desktop file with name='%s', comment='%s', icon='%s', exec='%s'",
         entries_[DesktopData::kNameIndex].toLatin1().data(),
         entries_[DesktopData::kCommentIndex].toLatin1().data(),
         entries_[DesktopData::kIconIndex].toLatin1().data(),
         entries_[DesktopData::kExecIndex].toLatin1().data());
    return true;
  } else {
    DLOG("not a valid desktop file, missing mandatory entries in the standard group header");
    return false;
  }
}

ApplicationManager::ApplicationManager()
    : applications_(new ApplicationListModel())
    , focusedApplication_(NULL)
    , pidHash_()
    , eventType_(static_cast<QEvent::Type>(QEvent::registerEventType())) {
  static int once = false;
  if (!once) {
    DLOG("starting application watcher");
    static ubuntu_ui_session_lifecycle_observer watcher = {
      sessionRequestedCallback, sessionBornCallback, sessionUnfocusedCallback,
      sessionFocusedCallback, sessionDiedCallback, this
    };
    ubuntu_ui_session_install_session_lifecycle_observer(&watcher);
    once = true;
  }
  DLOG("ApplicationManager::ApplicationManager (this=%p)", this);
}

ApplicationManager::~ApplicationManager() {
  DLOG("ApplicationManager::~ApplicationManager");
  pidHash_.clear();
  delete applications_;
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
  TaskEvent* taskEvent = static_cast<TaskEvent*>(event);
  switch (taskEvent->task_) {

    case TaskEvent::kAddApplication: {
      DLOG("handling add application task");
      const int kPid = taskEvent->id_;
      Application* application = pidHash_.value(kPid, NULL);
      if (application) {
        DLOG("got a match in the application list, setting '%s' (%d) to running",
             application->name().toLatin1().data(), kPid);
        application->setState(Application::Running);
        killTimer(application->timerId());
      } else {
        DLOG("didn't get a match in the application list, loading the desktop file");
        DesktopData* desktopData = new DesktopData(taskEvent->desktopFile_);
        if (desktopData->loaded()) {
          DLOG("desktopFile loaded, storing '%s' (%d) in the application list",
               desktopData->name().toLatin1().data(), kPid);
          Application* application = new Application(desktopData, kPid, Application::Running, -1);
          pidHash_.insert(kPid, application);
          applications_->add(application);
        } else {
          DLOG("unknown application, not storing in the application list");
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
        DLOG("removing application '%s' (%d) from the application list",
             application->name().toLatin1().data(), kPid);
        if (application->state() == Application::Starting) {
          killTimer(application->timerId());
        }
        if (focusedApplication_ == application) {
          focusedApplication_ = NULL;
          emit focusedApplicationChanged();
        }
        applications_->remove(application);
        delete application;
      } else {
        DLOG("Unknown application, not stored in the application list");
      }
      break;
    }

    case TaskEvent::kUnfocusApplication: {
      DLOG("handling unfocus application task");
      // Reset the currently focused application.
      Application* application = pidHash_.value(taskEvent->id_);
      if (application != NULL) {
        if (focusedApplication_ == application) {
          focusedApplication_ = NULL;
          emit focusedApplicationChanged();
        }
      }
      break;
    }

    case TaskEvent::kFocusApplication: {
      DLOG("handling focus application task");
      // Update the currently focused application.
      Application* application = pidHash_.value(taskEvent->id_);
      if (application != NULL) {
        if (focusedApplication_ != application) {
          focusedApplication_ = application;
          emit focusedApplicationChanged();
        }
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
  Application* application = applications_->findFromTimerId(kTimerId);
  if (application != NULL) {
    const qint64 kPid = application->handle();
    DLOG("application '%s' (%lld) hasn't been matched, killing it",
         application->name().toLatin1().data(), kPid);
    DASSERT(pidHash_.contains(kPid));
    pidHash_.remove(kPid);
    applications_->remove(application);
    delete application;
    killProcess(kPid);
  }
  killTimer(kTimerId);
}

ApplicationManager::StageHint ApplicationManager::stageHint() const {
  DLOG("ApplicationManager::stageHint (this=%p)", this);
  return static_cast<ApplicationManager::StageHint>(ubuntu_application_ui_setup_get_stage_hint());
}

ApplicationManager::FormFactorHint ApplicationManager::formFactorHint() const {
  DLOG("ApplicationManager::formFactorHint (this=%p)", this);
  return static_cast<ApplicationManager::FormFactorHint>(
      ubuntu_application_ui_setup_get_form_factor_hint());
}

ApplicationListModel* ApplicationManager::applications() const {
  DLOG("ApplicationManager::applications (this=%p)", this);
  return applications_;
}

Application* ApplicationManager::focusedApplication() const {
  DLOG("ApplicationManager::focusedApplication (this=%p)", this);
  return focusedApplication_;
}

void ApplicationManager::focusApplication(int handle) {
  DLOG("ApplicationManager::focusApplication (this=%p, handle=%d)", this, handle);
  ubuntu_ui_session_focus_running_session_with_id(handle);
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
  ubuntu_ui_session_unfocus_running_sessions();
}

Application* ApplicationManager::startProcess(QString desktopFile, QStringList arguments) {
  DLOG("ApplicationManager::startProcess (this=%p)", this);
  // Load desktop file.
  DesktopData* desktopData = new DesktopData(desktopFile);
  if (!desktopData->loaded()) {
    delete desktopData;
    return NULL;
  }

  // Format arguments.
  // FIXME(loicm) Special field codes are simply ignored for now.
  //     http://standards.freedesktop.org/desktop-entry-spec/latest/ar01s06.html
  QStringList execArguments = desktopData->exec().split(" ", QString::SkipEmptyParts);
  DASSERT(execArguments.size() > 0);
  QString exec(execArguments[0]);
  const int kSize = execArguments.size();
  for (int i = 1; i < kSize; i++) {
    if ((execArguments[i].size() == 2) && (execArguments[i][0].toLatin1() == '%')) {
      const char kChar = execArguments[i][1].toLatin1();
      if (kChar == 'F' || kChar == 'u' || kChar == 'U' || kChar == 'd' || kChar == 'D'
          || kChar == 'n' || kChar == 'N' || kChar == 'i' || kChar == 'c' || kChar == 'k'
          || kChar == 'v' || kChar == 'm') {
        continue;
      }
    }
    arguments.prepend(execArguments[i]);
  }
  arguments.append(QString("--desktop_file_hint=") + desktopData->file());
#if !defined(QT_NO_DEBUG)
  LOG("starting process '%s' with arguments:", exec.toLatin1().data());
  for (int i = 0; i < arguments.size(); i++)
    LOG("  '%s'", arguments[i].toLatin1().data());
#endif

  // Start process.
  bool result;
  qint64 pid = 0;
  struct passwd* passwd = getpwuid(getuid());
  DLOG("current working directory: '%s'", passwd ? passwd->pw_dir : "/");
  result = QProcess::startDetached(exec, arguments, QString(passwd ? passwd->pw_dir : "/"), &pid);
  DLOG_IF(result == false, "process failed to start");
  if (result == true) {
    DLOG("started process with pid %lld, adding '%s' to application list",
         pid, desktopData->name().toLatin1().data());
    Application* application = new Application(
        desktopData, pid, Application::Starting, startTimer(kTimeBeforeClosingProcess));
    pidHash_.insert(pid, application);
    applications_->add(application);
    return application;
  } else {
    return NULL;
  }
}

void ApplicationManager::stopProcess(Application* application) {
  DLOG("ApplicationManager::stopProcess (this=%p, application=%p)", this, application);
  if (application != NULL) {
    const qint64 kPid = application->handle();
    if (pidHash_.remove(kPid) > 0) {
      applications_->remove(application);
      delete application;
      killProcess(kPid);
    }
  }
}
