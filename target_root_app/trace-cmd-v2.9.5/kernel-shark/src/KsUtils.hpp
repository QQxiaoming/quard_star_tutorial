/* SPDX-License-Identifier: LGPL-2.1 */

/*
 * Copyright (C) 2017 VMware Inc, Yordan Karadzhov <ykaradzhov@vmware.com>
 */

/**
 *  @file    KsUtils.hpp
 *  @brief   KernelShark Utils.
 */

#ifndef _KS_UTILS_H
#define _KS_UTILS_H

// C++ 11
#include <chrono>

// Qt
#include <QtWidgets>

// KernelShark
#include "libkshark.h"
#include "libkshark-model.h"
#include "KsCmakeDef.hpp"
#include "KsPlotTools.hpp"

/** Macro providing the height of the screen in pixels. */
#define SCREEN_HEIGHT  QGuiApplication::screens()[0]->geometry().height()

/** Macro providing the width of the screen in pixels. */
#define SCREEN_WIDTH   QGuiApplication::screens()[0]->geometry().width()

//! @cond Doxygen_Suppress

static auto fontHeight = []()
{
	QFont font;
	QFontMetrics fm(font);

	return fm.height();
};

static auto stringWidth = [](QString s)
{
	QFont font;
	QFontMetrics fm(font);

	return fm.horizontalAdvance(s);
};

//! @endcond

/** Macro providing the height of the font in pixels. */
#define FONT_HEIGHT		fontHeight()

/** Macro providing the width of the font in pixels. */
#define FONT_WIDTH 		stringWidth("4")

/** Macro providing the width of a string in pixels. */
#define STRING_WIDTH(s)		stringWidth(s)

/** Macro providing the height of the KernelShark graphs in pixels. */
#define KS_GRAPH_HEIGHT	(FONT_HEIGHT*2)

//! @cond Doxygen_Suppress

#define KS_JSON_CAST(doc) \
reinterpret_cast<json_object *>(doc)

#define KS_C_STR_CAST(doc) \
reinterpret_cast<const char *>(doc)

typedef std::chrono::high_resolution_clock::time_point  hd_time;

#define GET_TIME std::chrono::high_resolution_clock::now()

#define GET_DURATION(t0) \
std::chrono::duration_cast<std::chrono::duration<double>>( \
std::chrono::high_resolution_clock::now() - t0).count()

//! @endcond

namespace KsUtils {

QVector<int> getCPUList();

QVector<int> getPidList();

QVector<int> getEventIdList(tep_event_sort_type sortType=TEP_EVENT_SORT_ID);

QVector<int> getFilterIds(tracecmd_filter_id *filter);

/** @brief Geat the list of plugins. */
inline QStringList getPluginList() {return plugins.split(";");}

void listFilterSync(bool state);

void graphFilterSync(bool state);

QCheckBox *addCheckBoxToMenu(QMenu *menu, QString name);

/** @brief Convert the timestamp of the trace record into a string showing
 *	   the time in seconds.
 *
 * @param ts: Input location for the timestamp.
 * @param prec: the number of digits after the decimal point in the return
 *		string.
 *
 * @returns String showing the time in seconds.
 */
inline QString Ts2String(int64_t ts, int prec)
{
	return QString::number(ts * 1e-9, 'f', prec);
}

bool matchCPUVisible(struct kshark_context *kshark_ctx,
			      struct kshark_entry *e, int cpu);

bool isInstalled();

QString getFile(QWidget *parent,
		const QString &windowName,
		const QString &filter,
		QString &lastFilePath);

QStringList getFiles(QWidget *parent,
		     const QString &windowName,
		     const QString &filter,
		     QString &lastFilePath);

QString getSaveFile(QWidget *parent,
		    const QString &windowName,
		    const QString &filter,
		    const QString &extension,
		    QString &lastFilePath);

QStringList splitArguments(QString cmd);

QVector<int> parseIdList(QString v_str);

}; // KsUtils

/** Identifier of the Dual Marker active state. */
enum class DualMarkerState {
	A,
	B
};

/**
 * The KsDataStore class provides the access to trace data for all KernelShark
 * widgets.
 */
class KsDataStore : public QObject
{
	Q_OBJECT
public:
	explicit KsDataStore(QWidget *parent = nullptr);

	~KsDataStore();

	void loadDataFile(const QString &file);

	void clear();

	/** Get the trace event parser. */
	tep_handle *tep() const {return _tep;}

	/** Get the trace data array.. */
	struct kshark_entry **rows() const {return _rows;}

	/** Get the size of the data array. */
	ssize_t size() const {return _dataSize;}

	void reload();

	void update();

	void registerCPUCollections();

	void applyPosTaskFilter(QVector<int>);

	void applyNegTaskFilter(QVector<int>);

	void applyPosEventFilter(QVector<int>);

	void applyNegEventFilter(QVector<int>);

	void applyPosCPUFilter(QVector<int>);

	void applyNegCPUFilter(QVector<int>);

	void clearAllFilters();

signals:
	/**
	 * This signal is emitted when the data has changed and the View
	 * widgets have to update.
	 */
	void updateWidgets(KsDataStore *);

private:
	/** Page event used to parse the page. */
	tep_handle		*_tep;

	/** Trace data array. */
	struct kshark_entry	**_rows;

	/** The size of the data array. */
	ssize_t			_dataSize;

	void _freeData();
	void _unregisterCPUCollections();
	void _applyIdFilter(int filterId, QVector<int> vec);
};

/** A Plugin Manage class. */
class KsPluginManager : public QObject
{
	Q_OBJECT
public:
	explicit KsPluginManager(QWidget *parent = nullptr);

	/** A list of available built-in plugins. */
	QStringList	_ksPluginList;

	/** A list of registered built-in plugins. */
	QVector<bool>	_registeredKsPlugins;

	/** A list of available user plugins. */
	QStringList	_userPluginList;

	/** A list of registered user plugins. */
	QVector<bool>	_registeredUserPlugins;

	void registerFromList(kshark_context *kshark_ctx);
	void unregisterFromList(kshark_context *kshark_ctx);

	void registerPlugin(const QString &plugin);
	void unregisterPlugin(const QString &plugin);

	void addPlugins(const QStringList &fileNames);

	void unloadAll();

	void updatePlugins(QVector<int> pluginId);

signals:
	/** This signal is emitted when a plugin is loaded or unloaded. */
	void dataReload();

private:
	void _parsePluginList();

	char *_pluginLibFromName(const QString &plugin, int &n);

	template <class T>
	void _forEachInList(const QStringList &pl,
			    const QVector<bool> &reg,
			    T action)
	{
		int nPlugins;
		nPlugins = pl.count();
		for (int i = 0; i < nPlugins; ++i) {
			if (reg[i]) {
				action(pl[i]);
			}
		}
	}
};

KsPlot::Color& operator <<(KsPlot::Color &thisColor, const QColor &c);

#endif
