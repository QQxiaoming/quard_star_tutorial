// SPDX-License-Identifier: GPL-2.0

/*
 * Copyright (C) 2017 VMware Inc, Yordan Karadzhov <ykaradzhov@vmware.com>
 */

// C
#include <sys/stat.h>
#include <getopt.h>
#include <unistd.h>

// C++
#include <iostream>

// Qt
#include <QtWidgets>

// KernelShark
#include "KsUtils.hpp"
#include "KsWidgetsLib.hpp"

#define default_input_file (char*)"trace.dat"

static char *input_file = nullptr;

using namespace std;

void usage(const char *prog)
{
	cout << "Usage: " << prog << endl
	     << "  -h	Display this help message\n"
	     << "  -v	Display version and exit\n"
	     << "  -i	input_file, default is " << default_input_file << endl
	     << "  -p	register plugin, use plugin name, absolute or relative path\n"
	     << "  -u	unregister plugin, use plugin name or absolute path\n";
}

struct TaskPrint : public QObject
{
	tep_handle	*_pevent;

	void print(QVector<int> pids)
	{
		for (auto const &pid: pids)
			cout << "task: "
			     << tep_data_comm_from_pid(_pevent, pid)
			     << "  pid: " << pid << endl;
	}
};

int main(int argc, char **argv)
{
	kshark_context *kshark_ctx(nullptr);
	QApplication a(argc, argv);
	KsPluginManager plugins;
	KsDataStore data;
	size_t nRows(0);
	int c;

	if (!kshark_instance(&kshark_ctx))
		return 1;

	while ((c = getopt(argc, argv, "hvi:p:u:")) != -1) {
		switch(c) {
		case 'v':
			printf("kshark-gui %s\n", KS_VERSION_STRING);
			return 0;

		case 'i':
			input_file = optarg;
			break;

		case 'p':
			plugins.registerPlugin(QString(optarg));
			break;

		case 'u':
			plugins.unregisterPlugin(QString(optarg));
			break;

		case 'h':
			usage(argv[0]);
			return 0;
		}
	}

	if (!input_file) {
			struct stat st;
			if (stat(default_input_file, &st) == 0)
				input_file = default_input_file;
	}

	if (input_file) {
		data.loadDataFile(input_file);
		nRows = data.size();
	} else {
		cerr << "No input file is provided.\n";
	}

	cout << nRows << " entries loaded\n";

	auto lamPrintPl = [&]()
	{
		kshark_plugin_list *pl;
		for (pl = kshark_ctx->plugins; pl; pl = pl->next)
			cout << pl->file << endl;
	};

	cout << "\n\n";
	lamPrintPl();
	sleep(1);

	QVector<bool> registeredPlugins;
	QStringList pluginsList;

	pluginsList << plugins._ksPluginList
		    << plugins._userPluginList;

	registeredPlugins << plugins._registeredKsPlugins
			  << plugins._registeredUserPlugins;

	KsCheckBoxWidget *pluginCBD
		= new KsPluginCheckBoxWidget(pluginsList);

	pluginCBD->set(registeredPlugins);

	KsCheckBoxDialog *dialog1 = new KsCheckBoxDialog(pluginCBD);
	QObject::connect(dialog1,	&KsCheckBoxDialog::apply,
			&plugins,	&KsPluginManager::updatePlugins);

	dialog1->show();
	a.exec();

	cout << "\n\nYou selected\n";
	lamPrintPl();
	sleep(1);

	if (!nRows)
		return 1;

	KsCheckBoxWidget *tasks_cbd =
		new KsTasksCheckBoxWidget(data.tep(), true);

	tasks_cbd->setDefault(false);

	TaskPrint p;
	p._pevent = data.tep();

	KsCheckBoxDialog *dialog2 = new KsCheckBoxDialog(tasks_cbd);
	QObject::connect(dialog2,	&KsCheckBoxDialog::apply,
			 &p,		&TaskPrint::print);

	cout << "\n\nYou selected\n";
	dialog2->show();
	a.exec();

	return 0;
}
