// This file is part of QtUbuntu, a set of Qt components for Ubuntu.
// Copyright © 2013 Canonical Ltd.
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 3, as published by the
// Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
// SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.

#include "qmlscene_ubuntu.h"
#include <QtQuick>
#include <QtQml/qqmlengine.h>
#include <qpa/qplatformnativeinterface.h>
#include <cstdio>
#include <ctime>
#include <csignal>
#include <cfloat>

Scene::Scene(QObject* parent)
    : QObject(parent)
    , frames_(0)
    , sum_(0.0)
    , min_(DBL_MAX)
    , max_(0.0) {
}

void Scene::beforeRendering() {
  clock_gettime(CLOCK_MONOTONIC, &t1_);
}

void Scene::afterRendering() {
  clock_gettime(CLOCK_MONOTONIC, &t2_);
  double time = ((t2_.tv_sec * 1000000000 + t2_.tv_nsec) -
                 (t1_.tv_sec * 1000000000 + t1_.tv_nsec)) / 1000000.0;
  if (time < min_)
    min_ = time;
  if (time > max_)
    max_ = time;
  sum_ += time;
  frames_++;
}

void Scene::getStats(int* frames, double* min, double* max, double* average) {
  *frames = frames_;
  *min = min_;
  *max = max_;
  *average = sum_ / frames_;
}

static void usage() {
  fprintf(stdout,
          "Usage: qmlscene-ubuntu [options] <filename>\n\n"
          "  Options:\n"
          "    -i <path>                  ... Add <path> to the list of import paths\n"
          "    -f or --fullscreen         ... Show the window fullscreen\n"
          "    -m or --maximized          ... Show the window maximized\n"
          "    -s or --session <session>  ... Set the Ubuntu session type\n"
          "    -r or --role <role>        ... Set the Ubuntu surface role\n"
          "    -o or --opaque             ... Set the Ubuntu opaque surface flag\n"
          "    -h or --help               ... Show that help\n");
}

// static void logger(QtMsgType type, const char* msg) {
//   Q_UNUSED(type);
//   Q_UNUSED(msg);
// }

static void signalHandler(int signal) {
  switch (signal) {
    case SIGINT:
    case SIGTERM: {
      QGuiApplication::quit();
      break;
    }
    default: {
      break;
    }
  }
}

int main(int argc, char* argv[]) {
  QQuickView* view;
  int exit_code;
  QUrl url;
  QStringList imports;
  bool fullscreen = false;
  bool maximized = false;
  int session = 0;
  int role = 1;
  int opaque = 0;

  for (int i = 1; i < argc; ++i) {
    if (QFileInfo(QFile::decodeName(argv[i])).exists()) {
      url = QUrl::fromLocalFile(argv[i]);
    } else {
      const QString kArg = QString::fromLatin1(argv[i]).toLower();
      if (kArg == QLatin1String("-i") && i + 1 < argc) {
        imports.append(QString::fromLatin1(argv[++i]));
      } else if (kArg == QLatin1String("-f") || kArg == QLatin1String("--fullscreen")) {
        fullscreen = true;
      } else if (kArg == QLatin1String("-m") || kArg == QLatin1String("--maximized")) {
        maximized = true;
      } else if ((kArg == QLatin1String("-s") || kArg == QLatin1String("--session"))
                 && i + 1 < argc) {
        session = atoi(argv[++i]);
      } else if ((kArg == QLatin1String("-r") || kArg == QLatin1String("--role"))
                 && i + 1 < argc) {
        role = atoi(argv[++i]);
      } else if ((kArg == QLatin1String("-o") || kArg == QLatin1String("--opaque"))
                 && i + 1 < argc) {
        opaque = atoi(argv[++i]);
      } else if (kArg == QLatin1String("-h") || kArg == QLatin1String("--help")) {
        usage();
        return 0;
      }
    }
  }
  if (url.isEmpty()) {
    usage();
    return 1;
  }

  // Swallow all the messages to avoid cluttering the standard output.
  // qInstallMsgHandler(logger);

  signal(SIGINT, signalHandler);
  signal(SIGTERM, signalHandler);

  QGuiApplication app(argc, argv);
  app.setApplicationName("QmlSceneUbuntu");
  app.setOrganizationName("Canonical");
  app.setOrganizationDomain("canonical.com");

  view = new QQuickView();

  QPlatformNativeInterface* native = QGuiApplication::platformNativeInterface();
  native->setProperty("session", session);
  view->setProperty("role", role);
  view->setProperty("opaque", opaque);

  QQmlEngine* engine = view->engine();
  for (int i = 0; i < imports.size(); ++i)
    engine->addImportPath(imports.at(i));

  view->setColor(Qt::transparent);
  view->setTitle("QML Scene Ubuntu");
  view->setResizeMode(QQuickView::SizeRootObjectToView);
  view->setSource(url);
  if (fullscreen)
    view->showFullScreen();
  else if (maximized)
    view->showMaximized();
  else
    view->show();

  Scene scene;
  QObject::connect(view, SIGNAL(beforeRendering()), &scene, SLOT(beforeRendering()));
  QObject::connect(view, SIGNAL(afterRendering()), &scene, SLOT(afterRendering()));

  exit_code = app.exec();
  delete view;

  int frames;
  double min, max, average;
  scene.getStats(&frames, &min, &max, &average);
  fprintf(stdout, "Over %d frames\nmin: %.2lf ms\nmax: %.2lf ms\navg: %.2lf ms\n",
          frames, min, max, average);

  return exit_code;
}
