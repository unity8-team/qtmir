// This file is part of QtUbuntu, a set of Qt components for Ubuntu.
// Copyright © 2013 Canonical Ltd.
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 3, as published by
// the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
// SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <QtGui/QGuiApplication>
#include <QtGui/QClipboard>
#include <cstdio>

enum Operation { kNone, kCopy, kPaste };

static void usage() {
  fprintf(stdout,
          "Usage: clipboard [options]\n\n"
          "  Options:\n"
          "    -c or --copy \"text\"     ... Copy text to clipboard\n"
          "    -p or --paste           ... Paste text from clipboard\n"
          "    -h or --help            ... Show that help\n");
}

int main(int argc, char* argv[]) {
  Operation operation = kNone;
  QString text;

  for (int i = 1; i < argc; ++i) {
    const QString kArg = QString::fromLatin1(argv[i]).toLower();
    if ((kArg == QLatin1String("-c") || kArg == QLatin1String("--copy")) && i + 1 < argc) {
      operation = kCopy;
      text = QString::fromLatin1(argv[++i]);
    } else if (kArg == QLatin1String("-p") || kArg == QLatin1String("--paste")) {
      operation = kPaste;
    } else if (kArg == QLatin1String("-h") || kArg == QLatin1String("--help")) {
      usage();
      return 1;
    }
  }
  if (operation == kNone) {
    usage();
    return 1;
  }

  QGuiApplication app(argc, argv);
  if (operation == kCopy) {
    QGuiApplication::clipboard()->setText(text);
    fprintf(stdout, "Copied: \"%s\"\n", text.toLatin1().data());
  } else {
    fprintf(stdout, "Pasted: \"%s\"\n", QGuiApplication::clipboard()->text().toLatin1().data());
  }

  return 0;
}
