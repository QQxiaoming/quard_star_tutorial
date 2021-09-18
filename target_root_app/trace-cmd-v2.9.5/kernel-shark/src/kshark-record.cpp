// SPDX-License-Identifier: GPL-2.0

/*
 * Copyright (C) 2018 VMware Inc, Yordan Karadzhov <y.karadz@gmail.com>
 */

// C
#include <unistd.h>

// KernelShark
#include "KsCaptureDialog.hpp"

int main(int argc, char **argv)
{
	QApplication a(argc, argv);
	KsCaptureDialog cd;

	int c;
	while ((c = getopt(argc, argv, "o:")) != -1) {
		switch(c) {
		case 'o':
			cd.setOutputFileName(QString(optarg));
			break;
		}
	}

	cd.show();
	return a.exec();
}
