// SPDX-License-Identifier: LGPL-2.1

/*
 * Copyright (C) 2018 VMware Inc, Yordan Karadzhov <ykaradzhov@vmware.com>
 */

/**
 *  @file    KsSearchFSM.cpp
 *  @brief   Finite-state machine for searching in trace data.
 */

// KernelShark
#include "KsSearchFSM.hpp"
#include "KsUtils.hpp"
#include "KsTraceViewer.hpp"
#include "KsWidgetsLib.hpp"

static bool notHaveCond(const QString &searchText, const QString &itemText)
{
	return !itemText.contains(searchText, Qt::CaseInsensitive);
}

static bool containsCond(const QString &searchText, const QString &itemText)
{
	return itemText.contains(searchText, Qt::CaseInsensitive);
}

static bool matchCond(const QString &searchText, const QString &itemText)
{
	return (itemText.compare(searchText, Qt::CaseInsensitive) == 0);
}

static bool noCond(const QString &searchText, const QString &itemText)
{
	return false;
}

/** Create a Finite-state machine for searching. */
KsSearchFSM::KsSearchFSM(QWidget *parent)
: _currentState(new NotDone),
  _lastRowSearched(0),
  _searchProgBar(parent),
  _searchCountLabel("", parent),
  _columnComboBox(parent),
  _selectComboBox(parent),
  _searchLineEdit(parent),
  _prevButton("Prev", parent),
  _nextButton("Next", parent),
  _searchRestartButton(QIcon::fromTheme("media-playback-start"), "", parent),
//   _searchStopButton(QIcon::fromTheme("media-playback-pause"), "", parent),
  _searchStopButton(QIcon::fromTheme("process-stop"), "", parent),
  _cond(nullptr),
  _pbAction(nullptr),
  _searchStopAction(nullptr),
  _searchRestartAction(nullptr)
{
	int bWidth = FONT_WIDTH * 6;

	_nextButton.setFixedWidth(bWidth);
	_prevButton.setFixedWidth(bWidth);

	_searchProgBar.setMaximumWidth(FONT_WIDTH * 10);
	_searchProgBar.setRange(0, KS_PROGRESS_BAR_MAX);

	_selectComboBox.addItem("contains");
	_selectComboBox.addItem("full match");
	_selectComboBox.addItem("does not have");
	updateCondition();
}

/**
 * Position all buttons and labels of the Finite-state machine for searching
 * in a toolbar.
 */
void KsSearchFSM::placeInToolBar(QToolBar *tb)
{
	tb->addWidget(&_columnComboBox);
	tb->addWidget(&_selectComboBox);
	tb->addWidget(&_searchLineEdit);
	tb->addSeparator();

	tb->addWidget(&_nextButton);
	tb->addWidget(&_prevButton);
	tb->addSeparator();

	_pbAction = tb->addWidget(&_searchProgBar);
	_pbAction->setVisible(false);

	tb->addWidget(&_searchCountLabel);

	_searchStopAction = tb->addWidget(&_searchStopButton);
	_searchStopAction->setVisible(false);

	_searchRestartAction = tb->addWidget(&_searchRestartButton);
	_searchRestartAction->setVisible(false);
	tb->addSeparator();
}

/**
 * Update the Matching condition function of the search according to the user
 * input.
 */
void KsSearchFSM::updateCondition()
{
	int xSelect = _selectComboBox.currentIndex();

	switch (xSelect) {
	case Condition::Containes:
		_cond = containsCond;
		return;

	case Condition::Match:
		_cond = matchCond;
		return;

	case Condition::NotHave:
		_cond = notHaveCond;
		return;

	default:
		_cond = noCond;
		return;
	}
}

void KsSearchFSM ::_lockSearchPanel(bool lock)
{
	_columnComboBox.setEnabled(!lock);
	_selectComboBox.setEnabled(!lock);
	_searchLineEdit.setReadOnly(lock);
	_prevButton.setEnabled(!lock);
	_nextButton.setEnabled(!lock);
// 	_graphFollowsCheckBox.setEnabled(!lock);
}

/** Act according to the provided input. */
void NotDone::handleInput(KsSearchFSM* sm, sm_input_t input)
{
	switch(input) {
	case sm_input_t::Start:
		sm->_lastRowSearched = -1;
		sm->lockSearchPanel();
		sm->updateCondition();
		sm->progressBarVisible(true);
		sm->searchStopVisible(true);
		sm->changeState(std::shared_ptr<InProgress>(new InProgress));
		break;

	case sm_input_t::Finish:
		sm->changeState(std::shared_ptr<Done>(new Done));
		break;

	default:
		/* Ignore the input. */
		break;
	}
}

/** Act according to the provided input. */
void Paused::handleInput(KsSearchFSM* sm, sm_input_t input)
{
	switch(input) {
	case sm_input_t::Start:
		sm->lockSearchPanel();
		sm->searchStopVisible(true);
		sm->searchRestartVisible(false);

		if (sm->column() != KsViewModel::TRACE_VIEW_COL_INFO &&
		    sm->column() != KsViewModel::TRACE_VIEW_COL_LAT)
			sm->_searchCountLabel.setText("");

		sm->changeState(std::shared_ptr<InProgress>(new InProgress));
		break;

	case sm_input_t::Change:
		sm->_searchProgBar.setValue(0);
		sm->_searchCountLabel.setText("");
		sm->progressBarVisible(false);
		sm->searchRestartVisible(false);
		sm->changeState(std::shared_ptr<NotDone>(new NotDone));
		break;

	default:
		/* Ignore the input. */
		break;
	}
}

/** Act according to the provided input. */
void InProgress::handleInput(KsSearchFSM* sm, sm_input_t input)
{
	auto lamUnlock = [&sm] () {
		sm->searchStopVisible(false);
		sm->unlockSearchPanel();
	};

	switch(input) {
	case sm_input_t::Stop:
		lamUnlock();
		sm->searchRestartVisible(true);
		sm->changeState(std::shared_ptr<Paused>(new Paused));
		break;

	case sm_input_t::Finish:
		lamUnlock();
		sm->progressBarVisible(false);
		sm->changeState(std::shared_ptr<Done>(new Done));
		break;

	default:
		/* Ignore the input. */
		break;
	}
}

/** Act according to the provided input. */
void Done::handleInput(KsSearchFSM* sm, sm_input_t i)
{
	switch(i) {
	case sm_input_t::Change:
		sm->_searchProgBar.setValue(0);
		sm->progressBarVisible(false);
		sm->_searchCountLabel.setText("");
		sm->searchStopVisible(false);
		sm->searchRestartVisible(false);
		sm->changeState(std::shared_ptr<NotDone>(new NotDone));
		break;

	default:
		/* Ignore the input. */
		break;
	}
}
