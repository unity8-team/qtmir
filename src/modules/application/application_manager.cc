// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

// FIXME(loicm) Desktop file loading should be executed on a dedicated I/O thread.

#include "application_manager.h"
#include "application_list_model.h"
#include "application.h"
#include "logging.h"

// Retrieves the size of an array at compile time.
#define ARRAY_SIZE(a) \
    ((sizeof(a) / sizeof(*(a))) / static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))

// The time (in ms) to wait before closing a process that's not been matched by a new session.
const int kTimeBeforeClosingProcess = 30000;

class TaskEvent : public QEvent {
 public:
  enum Task { kAddApplication = 0, kRemoveApplication, kRequestFocus };
  TaskEvent(char* desktopFile, int pid, int task, QEvent::Type type)
      : QEvent(type)
      , desktopFile_(desktopFile)
      , pid_(pid)
      , task_(task) {
    DLOG("TaskEvent::TaskEvent (this=%p, desktopFile='%s', id=%d, task=%d, type=%d)",
         this, desktopFile, pid, task, type);
  }
  ~TaskEvent() {
    DLOG("TaskEvent::~TaskEvent");
    delete [] desktopFile_;
  }
  char* desktopFile_;
  int pid_;
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

static void sessionFocusedCallback(ubuntu_ui_session_properties session, void* context) {
  Q_UNUSED(session);
  Q_UNUSED(context);
  // FIXME(loicm) Set focused app once Ubuntu application API has support for unfocused signal.
}

static void sessionRequestedCallback(ubuntu_ui_session_properties session, void* context) {
  DLOG("sessionRequestedCallback (session=%p, context=%p)", session, context);
  DASSERT(context != NULL);
  // Post a task to be executed on the ApplicationManager thread (GUI thread). The favorite
  // application is stored in the PID field.
  ApplicationManager* manager = static_cast<ApplicationManager*>(context);
  QCoreApplication::postEvent(manager, new TaskEvent(
      NULL, reinterpret_cast<int>(session), TaskEvent::kRequestFocus, manager->eventType()));
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
    , pidHash_()
    , unmatchedProcesses_()
    , eventType_(static_cast<QEvent::Type>(QEvent::registerEventType())) {
  static int once = false;
  if (!once) {
    DLOG("starting application watcher");
    static ubuntu_ui_session_lifecycle_observer watcher = {
      sessionRequestedCallback, sessionBornCallback, NULL, sessionFocusedCallback,
      sessionDiedCallback, this
    };
    ubuntu_ui_session_install_session_lifecycle_observer(&watcher);
    once = true;
  }
  DLOG("ApplicationManager::ApplicationManager (this=%p)", this);
}

ApplicationManager::~ApplicationManager() {
  DLOG("ApplicationManager::~ApplicationManager");
  pidHash_.clear();
  unmatchedProcesses_.clear();
  delete applications_;
}

void ApplicationManager::customEvent(QEvent* event) {
  DLOG("ApplicationManager::customEvent (this=%p, event=%p)", this, event);
  DASSERT(QThread::currentThread() == thread());
  TaskEvent* taskEvent = static_cast<TaskEvent*>(event);
  switch (taskEvent->task_) {

    case TaskEvent::kAddApplication: {
      DLOG("handling add application task");
      DesktopData* desktopData = NULL;
      QProcess* process = NULL;
      // Search for a matching process.
      const int kSize = unmatchedProcesses_.size();
      for (int i = 0; i < kSize; i++) {
        DASSERT(unmatchedProcesses_[i].process_ != NULL);
        DASSERT(unmatchedProcesses_[i].desktopData_ != NULL);
        if (unmatchedProcesses_[i].process_->pid() == taskEvent->pid_) {
          DLOG("got a match with '%s' in the unmatched processes",
               unmatchedProcesses_[i].desktopData_->name().toLatin1().data());
          killTimer(unmatchedProcesses_[i].timerId_);
          desktopData = unmatchedProcesses_[i].desktopData_;
          process = unmatchedProcesses_[i].process_;
          unmatchedProcesses_.removeAt(i);
          break;
        }
      }
      // Load the desktop file if no process matches.
      if (desktopData == NULL) {
        DLOG("didn't get a match in the unmatched processes, loading the desktop file");
        desktopData = new DesktopData(taskEvent->desktopFile_);
        if (!desktopData->loaded()) {
          delete desktopData;
          break;
        }
      }
      // Create the application and store it in the data model.
      Application* application = new Application(desktopData, process, taskEvent->pid_);
      DASSERT(!pidHash_.contains(taskEvent->pid_));
      pidHash_.insert(taskEvent->pid_, application);
      applications_->add(application);
      break;
    }

    case TaskEvent::kRemoveApplication: {
      DLOG("handling remove application task");
      // Remove the application from the data model.
      Application* application = pidHash_.take(taskEvent->pid_);
      if (application != NULL) {
        applications_->remove(application);
        delete application;
      }
      break;
    }

    case TaskEvent::kRequestFocus: {
      DLOG("handling request focus task");
      // The favorite application is stored in the PID field.
      emit focusRequested(static_cast<FavoriteApplication>(taskEvent->pid_));
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
  const int kSize = unmatchedProcesses_.size();
  const int timerId = event->timerId();
  for (int i = 0; i < kSize; i++) {
    if (unmatchedProcesses_[i].timerId_ == timerId) {
      DASSERT(unmatchedProcesses_[i].desktopData_ != NULL);
      DASSERT(unmatchedProcesses_[i].process_ != NULL);
      DLOG("closing process '%s' as it's not been matched by a new session",
           unmatchedProcesses_[i].desktopData_->name().toLatin1().data());
      delete unmatchedProcesses_[i].desktopData_;
      delete unmatchedProcesses_[i].process_;
      unmatchedProcesses_.removeAt(i);
    }
  }
  killTimer(event->timerId());
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
  DesktopData* desktopData = new DesktopData(desktopFile);
  if (desktopData->loaded()) {
    // FIXME(loicm) Special field codes are simply ignored for now.
    //     http://standards.freedesktop.org/desktop-entry-spec/latest/ar01s06.html
    QStringList execArguments = desktopData->exec().split(" ", QString::SkipEmptyParts);
    DASSERT(execArguments.size() > 0);
    QString exec(execArguments[0]);
    const int kSize = execArguments.size();
    for (int i = 1; i < kSize; i++) {
      if ((execArguments[i].size() == 2) && (execArguments[i][0].toLatin1() == '%')) {
        const char kChar = execArguments[i][1].toLatin1();
        if  (kChar == 'F' || kChar == 'u' || kChar == 'U' || kChar == 'd' || kChar == 'D'
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
    QProcess* process = new QProcess();
    process->start(exec, arguments);
    int timerId = startTimer(kTimeBeforeClosingProcess);
    unmatchedProcesses_.prepend(ApplicationManager::Process(desktopData, process, timerId));
  } else {
    delete desktopData;
  }
  return NULL;
}

void ApplicationManager::stopProcess(Application* application) {
  DLOG("ApplicationManager::stopProcess (this=%p, application=%p)", this, application);
  if (application != NULL) {
    QProcess* process = application->process();
    DLOG_IF(process == NULL, "can't stop process not started by the application manager");
    // FIXME(loicm) Should we add support to stop process using the pid if not started by the
    //     application manager?
    if (process != NULL && process->state()) {
      DLOG("stopping process '%s' with pid %d", application->exec().toLatin1().data(),
           application->handle());
      process->close();
    }
  }
}
