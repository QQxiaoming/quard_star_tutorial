/* SPDX-License-Identifier: LGPL-2.1 */

/*
 * Copyright (C) 2018 VMware Inc, Yordan Karadzhov <ykaradzhov@vmware.com>
 */

/**
 *  @file    KsSearchFSM.hpp
 *  @brief   Finite-state machine for searching in trace data.
 */

#ifndef _KS_SEARCH_FSM_H
#define _KS_SEARCH_FSM_H

// C++11
#include <memory>

// Qt
#include <QtWidgets>

/** Matching condition function type. To be user for searching. */
typedef bool (*search_condition_func)(const QString &, const QString &);

/** State Identifiers of the Finite-state machine for searching. */
enum class search_state_t
{
	/** Identifier of the "NotDone" state. */
	NotDone_s = 0,

	/** Identifier of the "InProgress" state. */
	InProgress_s = 1,

	/** Identifier of the "Paused" state. */
	Paused_s = 2,

	/** Identifier of the "Done" state. */
	Done_s = 3
};

/** Inputs of the Finite-state machine for searching. */
enum class sm_input_t
{
	Start = 0,
	Stop = 1,
	Finish = 2,
	Change = 3
};

class KsSearchFSM;

/**
 * State provides a base class for the states of the Finite-state machine for
 * searching.
 */
struct State
{
	/** Virtual destructor. */
	virtual ~State() {}

	/**
	 * Act according to the provided input. This is a pure virtual
	 * function.
	 */
	virtual void handleInput(KsSearchFSM* sm, sm_input_t i) = 0;

	/**
	 * Get the identifier of this state. This is a pure virtual function.
	 */
	virtual search_state_t id() = 0;
};

/** "NotDone" state. */
struct NotDone : public State
{
	void handleInput(KsSearchFSM* sm, sm_input_t i) override;

	search_state_t id() override {return search_state_t::NotDone_s;}
};

/** "InProgress" state. */
struct InProgress : public State
{
	void handleInput(KsSearchFSM* sm, sm_input_t i) override;

	/** Get the identifier of this state. */
	search_state_t id() override {return search_state_t::InProgress_s;}
};

/** "Paused" state. */
struct Paused : public State
{
	void handleInput(KsSearchFSM* sm, sm_input_t i) override;

	/** Get the identifier of this state. */
	search_state_t id() override {return search_state_t::Paused_s;}
};

/** "Done" state. */
struct Done : public State
{
	void handleInput(KsSearchFSM* sm, sm_input_t i) override;

	/** Get the identifier of this state. */
	search_state_t id() override {return search_state_t::Done_s;}
};

/** Finite-state machine for searching. */
class KsSearchFSM : public QWidget
{
	Q_OBJECT
public:
	explicit KsSearchFSM(QWidget *parent = nullptr);

	void placeInToolBar(QToolBar *tb);

	/** Act according to the provided input. */
	void handleInput(sm_input_t input)
	{
		_currentState->handleInput(this, input);
	}

	/** Switch the state. */
	void changeState(std::shared_ptr<State> newState)
	{
		_currentState = newState;
	}

	/** Get the identifier of the Current state. */
	search_state_t getState() const {return _currentState->id();}

	/** Get the data column to search in. */
	int column() const {return _columnComboBox.currentIndex();}

	/** Get the Matching condition function. */
	search_condition_func condition() const {return _cond;}

	/** Get the text to search for. */
	QString searchText() const {return _searchLineEdit.text();}

	/** Set the value of the Search Progress Bar. */
	void setProgress(int v) {_searchProgBar.setValue(v);}

	/** Increment the value of the Search Progress Bar. */
	void incrementProgress()
	{
		_searchProgBar.setValue(_searchProgBar.value() + 1);
	}

	void updateCondition();

	/** Disable the user searching input (lock the panel). */
	void lockSearchPanel() {_lockSearchPanel(true);}

	/** Enable the user searching input (unlock the panel). */
	void unlockSearchPanel() {_lockSearchPanel(false);}

	/** Set the visibility of the Search Progress Bar. */
	void progressBarVisible(bool v) {_pbAction->setVisible(v);}

	/** Set the visibility of the Search Stop button. */
	void searchStopVisible(bool v) {_searchStopAction->setVisible(v);}

	/** Set the visibility of the Search Restart button. */
	void searchRestartVisible(bool v) {_searchRestartAction->setVisible(v);}

	/** Current State of the Finite-state machine for searching. */
	std::shared_ptr<State>	_currentState;

	/**
	 * Last row, tested for matching. To be used when restarting the
	 * search. Note that the field uses "int" as a type because this
	 * is the type supported by the Qt widget (QTableView).
	 */
	int		_lastRowSearched;

//! @cond Doxygen_Suppress

	QProgressBar	_searchProgBar;

	QLabel		_searchCountLabel;

	QComboBox	_columnComboBox;

	QComboBox	_selectComboBox;

	QLineEdit	_searchLineEdit;

	QPushButton	_prevButton, _nextButton;

	QPushButton	_searchRestartButton, _searchStopButton;

//! @endcond

private:

	search_condition_func	_cond;

	QAction		*_pbAction, *_searchStopAction, *_searchRestartAction;

	void _lockSearchPanel(bool lock);

	enum Condition
	{
		Containes = 0,
		Match = 1,
		NotHave = 2
	};
};

#endif
