// SPDX-License-Identifier: GPL-2.0

/*
 * Copyright (C) 2017 VMware Inc, Yordan Karadzhov (VMware) <y.karadz@gmail.com>
 */

// C++
#include<iostream>
using namespace std;

// Qt
#include <QtWidgets>

// KernelShark
#include "KsUtils.hpp"

int main(int argc, char **argv)
{
	QString text = "echo \"I want \\\" here\" \\\n \"and \\\' here\"";
	QApplication a(argc, argv);
	QStringList argList;
	bool ok = true;
	QProcess proc;

	while (ok) {
		text = QInputDialog::getMultiLineText(nullptr,
						      "Shell quoting test",
						      "Shell input:",
						      text,
						      &ok);

		if (ok) {
			argList = KsUtils::splitArguments(text);
			qInfo() << argList;

			proc.setProgram(argList.takeFirst());
			proc.setArguments(argList);

			proc.start();
			proc.waitForFinished();

			if (proc.exitCode())
				cout << proc.errorString().toStdString() << endl;

			cout << proc.readAllStandardError().toStdString()
			     << endl
			     << proc.readAllStandardOutput().toStdString()
			     << endl;
		}
	}

	return 0;
}
