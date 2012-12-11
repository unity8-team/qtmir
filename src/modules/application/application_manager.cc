// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "application_manager.h"
#include "application_list_model.h"
#include "application.h"
#include "logging.h"

#define ARRAY_SIZE(a) \
    ((sizeof(a) / sizeof(*(a))) / static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))

class TaskEvent : public QEvent {
 public:
  enum Task { kAdd = 0, kRemove };

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
  QCoreApplication::postEvent(
      manager, new TaskEvent(
          qstrdup(ubuntu_ui_session_properties_get_desktop_file_hint(session)),
          ubuntu_ui_session_properties_get_application_instance_id(session), TaskEvent::kAdd,
          manager->eventType()));
}

static void sessionDiedCallback(ubuntu_ui_session_properties session, void* context) {
  DLOG("sessionDiedCallback (session=%p, context=%p)", session, context);
  DASSERT(context != NULL);
  // Post a task to be executed on the ApplicationManager thread (GUI thread).
  ApplicationManager* manager = static_cast<ApplicationManager*>(context);
  QCoreApplication::postEvent(
      manager, new TaskEvent(
          NULL, ubuntu_ui_session_properties_get_application_instance_id(session),
          TaskEvent::kRemove, manager->eventType()));
}

static void sessionFocusedCallback(ubuntu_ui_session_properties session, void* context) {
  Q_UNUSED(session);
  Q_UNUSED(context);
  // FIXME(loicm) Set focused app once Ubuntu application API has support for unfocused signal.
}

Application* ApplicationManager::createApplication(const char* desktopFile, int id) {
  DLOG("ApplicationManager::createApplication (this=%p, desktopFile=%s, id=%d)",
       this, desktopFile, id);
  DASSERT(desktopFile != NULL);
  const struct { const char* const name; int size; unsigned int flag; } kEntryNames[] = {
    { "Name=",    sizeof("Name=") - 1,    1 << 0 },
    { "Comment=", sizeof("Comment=") - 1, 1 << 1 },
    { "Icon=",    sizeof("Icon=") - 1,    1 << 2 },
  };
  const unsigned int kMatchMask = kEntryNames[0].flag | kEntryNames[1].flag | kEntryNames[2].flag;
  const int kBufferSize = 256;
  const int kEntriesCount = ARRAY_SIZE(kEntryNames);
  static char entryBuffers[kEntriesCount][kBufferSize];
  char* entries[kEntriesCount] = { 0 };
  QFile file(desktopFile);

  // Open file.
  if (!file.open(QFile::ReadOnly | QIODevice::Text)) {
    DLOG("can't open file: %s", file.errorString().toLatin1().data());
    return NULL;
  }

  // Validate "magic key" (standard group header).
  if (file.readLine(&entryBuffers[0][0], kBufferSize) != -1) {
    if (strncmp(&entryBuffers[0][0], "[Desktop Entry]", sizeof("[Desktop Entry]" - 1))) {
      DLOG("not a desktop file");
      return NULL;
    }
  }

  int length;
  unsigned int matches = 0;
  while ((length = file.readLine(&entryBuffers[matches][0], kBufferSize)) != -1) {
    // Skip empty lines.
    if (length > 1) {
      // Stop when reaching unsupported next group header.
      if (entryBuffers[matches][0] == '[') {
        DLOG("reached next group header, leaving loop");
        break;
      }
      // Lookup entries ignoring duplicates if any.
      for (int i = 0; i < kEntriesCount; i++) {
        if (!strncmp(&entryBuffers[matches][0], kEntryNames[i].name, kEntryNames[i].size)) {
          if (~matches & kEntryNames[i].flag) {
            entryBuffers[matches][length-1] = '\0';
            entries[i] = &entryBuffers[matches][kEntryNames[i].size];
            matches |= kEntryNames[i].flag;
            break;
          }
        }
      }
      // Stop when matching the right number of entries.
      if (matches == kMatchMask) {
        break;
      }
    }
  }

  // Check that at least the Name and Icon entries are set.
  if (matches & (kEntryNames[0].flag | kEntryNames[2].flag)) {
    return new Application(entries[0], entries[1], entries[2], id);
  } else {
    DLOG("not a valid desktop file, missing entries in the standard group header");
    return NULL;
  }
}

ApplicationManager::ApplicationManager()
    : applications_(new ApplicationListModel())
    , eventType_(static_cast<QEvent::Type>(QEvent::registerEventType())) {
  DLOG("ApplicationManager::ApplicationManager (this=%p)", this);
}

ApplicationManager::~ApplicationManager() {
  DLOG("ApplicationManager::~ApplicationManager");
  idHash_.clear();
  delete applications_;
}

void ApplicationManager::customEvent(QEvent* event) {
  DLOG("ApplicationManager::customEvent (this=%p, event=%p)", this, event);
  DASSERT(QThread::currentThread() == thread());
  TaskEvent* taskEvent = static_cast<TaskEvent*>(event);
  switch (taskEvent->task_) {
    case TaskEvent::kAdd: {
      DASSERT(!idHash_.contains(taskEvent->id_));
      // FIXME(loicm) createApplication should be executed on a dedicated I/O thread.
      Application* application = createApplication(taskEvent->desktopFile_, taskEvent->id_);
      if (application) {
        idHash_.insert(taskEvent->id_, application);
        applications_->add(application);
      }
      break;
    }
    case TaskEvent::kRemove: {
      Application* application = idHash_.take(taskEvent->id_);
      if (application != NULL) {
        applications_->remove(application);
        delete application;
      }
      break;
    }
    default: {
      break;
    }
  }
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

void ApplicationManager::focusApplication(int applicationId) {
  DLOG("ApplicationManager::focusApplication (this=%p, applicationId=%d)", this, applicationId);
  ubuntu_ui_session_focus_running_session_with_id(applicationId);
}

void ApplicationManager::focusFavoriteApplication(
    ApplicationManager::FavoriteApplication application) {
  DLOG("ApplicationManager::focusFavoriteApplication (this=%p, application=%d)",
       this, static_cast<int>(application));
  ubuntu_ui_session_trigger_switch_to_well_known_application(
     static_cast<ubuntu_ui_well_known_application>(application));
}

void ApplicationManager::startWatcher() {
  DLOG("ApplicationManager::startWatcher (this=%p)", this);
  static int once = false;
  if (!once) {
    DLOG("starting watcher for once");
    static ubuntu_ui_session_lifecycle_observer watcher = {
      NULL, sessionBornCallback, sessionFocusedCallback, sessionDiedCallback, this
    };
    ubuntu_ui_session_install_session_lifecycle_observer(&watcher);
    once = true;
  }
}
