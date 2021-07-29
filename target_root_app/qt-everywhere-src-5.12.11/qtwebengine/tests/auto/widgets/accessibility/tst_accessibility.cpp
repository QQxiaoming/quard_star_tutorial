/*
    Copyright (C) 2015 The Qt Company Ltd.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <qtest.h>
#include "../util.h"

#include <qaccessible.h>
#include <qwebengineview.h>
#include <qwebenginepage.h>
#include <qwidget.h>

class tst_Accessibility : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private Q_SLOTS:
    void noPage();
    void hierarchy();
    void text();
    void value();
    void roles_data();
    void roles();
};

// This will be called before the first test function is executed.
// It is only called once.
void tst_Accessibility::initTestCase()
{
}

// This will be called after the last test function is executed.
// It is only called once.
void tst_Accessibility::cleanupTestCase()
{
}

// This will be called before each test function is executed.
void tst_Accessibility::init()
{
}

// This will be called after every test function.
void tst_Accessibility::cleanup()
{
}

void tst_Accessibility::noPage()
{
    QWebEngineView webView;
    webView.show();

    QTest::qWait(1000);
    QAccessibleInterface *view = QAccessible::queryAccessibleInterface(&webView);
    QVERIFY(view);
    QCOMPARE(view->role(), QAccessible::Client);
    QCOMPARE(view->childCount(), 1);
    QAccessibleInterface *document = view->child(0);
    QCOMPARE(document->role(), QAccessible::WebDocument);
    QCOMPARE(document->parent(), view);
    QCOMPARE(document->childCount(), 0);
}

void tst_Accessibility::hierarchy()
{
    QWebEngineView webView;
    webView.setHtml("<html><body>" \
        "Hello world" \
        "<input type='text' value='some text'></input>" \
        "</body></html>");
    webView.show();
    QSignalSpy spyFinished(&webView, &QWebEngineView::loadFinished);
    QVERIFY(spyFinished.wait());

    QAccessibleInterface *view = QAccessible::queryAccessibleInterface(&webView);
    QVERIFY(view);
    QCOMPARE(view->role(), QAccessible::Client);
    QCOMPARE(view->childCount(), 1);
    // Wait for accessibility to be fully initialized
    QTRY_VERIFY(view->child(0)->childCount() == 1);
    QAccessibleInterface *document = view->child(0);
    QCOMPARE(document->role(), QAccessible::WebDocument);
    QCOMPARE(document->parent(), view);
    QCOMPARE(view->indexOfChild(document), 0);
    QCOMPARE(document->childCount(), 1);
    QAccessibleInterface *grouping = document->child(0);
    QVERIFY(grouping);
    QCOMPARE(grouping->parent(), document);
    QCOMPARE(document->indexOfChild(grouping), 0);
    QCOMPARE(grouping->childCount(), 2);

    QAccessibleInterface *text = grouping->child(0);
    QCOMPARE(text->role(), QAccessible::StaticText);
    QCOMPARE(text->parent(), grouping);
    QCOMPARE(grouping->indexOfChild(text), 0);
    QCOMPARE(text->childCount(), 0);
    QCOMPARE(text->text(QAccessible::Name), QStringLiteral("Hello world"));
    QCOMPARE(text->text(QAccessible::Description), QString());
    QCOMPARE(text->text(QAccessible::Value), QString());

    QAccessibleInterface *input = grouping->child(1);
    QCOMPARE(input->role(), QAccessible::EditableText);
    QCOMPARE(input->parent(), grouping);
    QCOMPARE(grouping->indexOfChild(input), 1);
    QCOMPARE(input->childCount(), 0);
    QCOMPARE(input->text(QAccessible::Name), QString());
    QCOMPARE(input->text(QAccessible::Description), QString());
    QCOMPARE(input->text(QAccessible::Value), QStringLiteral("some text"));

    QRect windowRect = webView.geometry();
    QRect inputRect = input->rect();
    QVERIFY(!inputRect.isEmpty());
    QVERIFY(windowRect.contains(inputRect));
    QPoint inputCenter = inputRect.center();
    QAccessibleInterface *hitTest = view;
    QAccessibleInterface *child = Q_NULLPTR;
    while (hitTest) {
        child = hitTest;
        hitTest = hitTest->childAt(inputCenter.x(), inputCenter.y());
    }
    QCOMPARE(input, child);
}

void tst_Accessibility::text()
{
    QWebEngineView webView;
    webView.setHtml("<html><body>" \
        "<input type='text' value='Good morning!'></input>" \
        "<p id='labelName'>Enter your name here:</p>" \
        "<input type='text' value='my name' aria-labelledby='labelName' aria-describedby='explanation'></input>" \
        "<p id='explanation'>Provide both first and last name.</p>" \
        "<input type='text' value='Good day!' placeholder='day'></input>" \
        "</body></html>");
    webView.show();
    QSignalSpy spyFinished(&webView, &QWebEngineView::loadFinished);
    QVERIFY(spyFinished.wait());

    QAccessibleInterface *view = QAccessible::queryAccessibleInterface(&webView);
    // Wait for accessibility to be fully initialized

    QTRY_VERIFY(view->child(0)->childCount() == 5);
    QAccessibleInterface *document = view->child(0);
    QVERIFY(document);

    // Good morning! [edit]
    QAccessibleInterface *grouping1 = document->child(0);
    QAccessibleInterface *input1 = grouping1;
    QCOMPARE(input1->role(), QAccessible::EditableText);
    QCOMPARE(input1->text(QAccessible::Name), QString());
    QCOMPARE(input1->text(QAccessible::Description), QString());
    QCOMPARE(input1->text(QAccessible::Value), QStringLiteral("Good morning!"));

    QAccessibleTextInterface *textInterface1 = input1->textInterface();
    QVERIFY(textInterface1);
    QCOMPARE(textInterface1->characterCount(), 13);
    QCOMPARE(textInterface1->selectionCount(), 0);
    QCOMPARE(textInterface1->text(2, 9), QStringLiteral("od morn"));
    int start = -1;
    int end = -1;
    QCOMPARE(textInterface1->textAtOffset(8, QAccessible::WordBoundary, &start, &end), QStringLiteral("morning"));

    // Enter your name here:
    // my name [edit]
    // Provide both first and last name here.
    QAccessibleInterface *grouping2 = document->child(1);
    QAccessibleInterface *label1 = grouping2->child(0);
    QCOMPARE(label1->role(), QAccessible::StaticText);
    QCOMPARE(label1->text(QAccessible::Name), QStringLiteral("Enter your name here:"));
    QCOMPARE(label1->text(QAccessible::Description), QString());
    QCOMPARE(label1->text(QAccessible::Value), QString());
    QAccessibleInterface *grouping3 = document->child(2);
    QAccessibleInterface *input2 = grouping3;
    QCOMPARE(input2->role(), QAccessible::EditableText);
    QCOMPARE(input2->text(QAccessible::Name), QStringLiteral("Enter your name here:"));
    QCOMPARE(input2->text(QAccessible::Description), QStringLiteral("Provide both first and last name."));
    QCOMPARE(input2->text(QAccessible::Value), QStringLiteral("my name"));
    QAccessibleInterface *grouping4 = document->child(3);
    QAccessibleInterface *label2 = grouping4->child(0);
    QCOMPARE(label2->role(), QAccessible::StaticText);
    QCOMPARE(label2->text(QAccessible::Name), QStringLiteral("Provide both first and last name."));
    QCOMPARE(label2->text(QAccessible::Description), QString());
    QCOMPARE(label2->text(QAccessible::Value), QString());

    // Good day! [edit]
    QAccessibleInterface *grouping5 = document->child(4);
    QAccessibleInterface *input3 = grouping5;
    QCOMPARE(input3->role(), QAccessible::EditableText);
    QCOMPARE(input3->text(QAccessible::Name), QStringLiteral("day"));
    QCOMPARE(input3->text(QAccessible::Description), QString());
    QCOMPARE(input3->text(QAccessible::Value), QStringLiteral("Good day!"));
}

void tst_Accessibility::value()
{
    QWebEngineView webView;
    webView.setHtml("<html><body>" \
        "<div role='slider' aria-valuenow='4' aria-valuemin='1' aria-valuemax='10'></div>" \
        "<div class='progress' role='progressbar' aria-valuenow='77' aria-valuemin='22' aria-valuemax='99'></div>" \
        "</body></html>");
    webView.show();
    QSignalSpy spyFinished(&webView, &QWebEngineView::loadFinished);
    QVERIFY(spyFinished.wait());

    QAccessibleInterface *view = QAccessible::queryAccessibleInterface(&webView);
    QTRY_COMPARE(view->child(0)->childCount(), 2);
    QAccessibleInterface *document = view->child(0);
    QCOMPARE(document->childCount(), 2);

    QAccessibleInterface *slider = document->child(0);
    QCOMPARE(slider->role(), QAccessible::Slider);
    QCOMPARE(slider->text(QAccessible::Name), QString());
    QCOMPARE(slider->text(QAccessible::Description), QString());
    QCOMPARE(slider->text(QAccessible::Value), QString());
    QAccessibleValueInterface *valueInterface = slider->valueInterface();
    QVERIFY(valueInterface);
    QCOMPARE(valueInterface->currentValue().toInt(), 4);
    QCOMPARE(valueInterface->minimumValue().toInt(), 1);
    QCOMPARE(valueInterface->maximumValue().toInt(), 10);

    QAccessibleInterface *progressBar = document->child(1);
    QCOMPARE(progressBar->role(), QAccessible::ProgressBar);
    QCOMPARE(progressBar->text(QAccessible::Name), QString());
    QCOMPARE(progressBar->text(QAccessible::Description), QString());
    QCOMPARE(progressBar->text(QAccessible::Value), QString());
    QAccessibleValueInterface *progressBarValueInterface = progressBar->valueInterface();
    QVERIFY(progressBarValueInterface);
    QCOMPARE(progressBarValueInterface->currentValue().toInt(), 77);
    QCOMPARE(progressBarValueInterface->minimumValue().toInt(), 22);
    QCOMPARE(progressBarValueInterface->maximumValue().toInt(), 99);
}

void tst_Accessibility::roles_data()
{
    QTest::addColumn<QString>("html");
    QTest::addColumn<bool>("isSection");
    QTest::addColumn<QAccessible::Role>("role");

    QTest::newRow("AX_ROLE_ABBR") << QString("<abbr>a</abbr>") << false << QAccessible::StaticText;
    QTest::newRow("AX_ROLE_ALERT") << QString("<div role='alert'>alert</div>") << true << QAccessible::AlertMessage;
    QTest::newRow("AX_ROLE_ALERT_DIALOG") << QString("<div role='alertdialog'>alert</div>") << true << QAccessible::AlertMessage;
    //QTest::newRow("AX_ROLE_ANCHOR") << QString("<a>target</a>") << false << QAccessible::Link; // FIXME: The test case might be wrong (see https://codereview.chromium.org/2713193003)
    QTest::newRow("AX_ROLE_ANNOTATION") << QString("<rt>a</rt>") << false << QAccessible::StaticText;
    QTest::newRow("AX_ROLE_APPLICATION") << QString("<div role='application'>landmark</div>") << true << QAccessible::Document;
    QTest::newRow("AX_ROLE_ARTICLE") << QString("<article>a</article>") << true << QAccessible::Section;
    QTest::newRow("AX_ROLE_AUDIO") << QString("<audio controls><source src='test.mp3' type='audio/mpeg'></audio>") << false << QAccessible::Sound;
    QTest::newRow("AX_ROLE_BANNER") << QString("<header>a</header>") << true << QAccessible::Section;
    QTest::newRow("AX_ROLE_BLOCKQUOTE") << QString("<blockquote>a</blockquote>") << true << QAccessible::Section;
    QTest::newRow("AX_ROLE_BUTTON") << QString("<button>a</button>") << false << QAccessible::Button;
    //QTest::newRow("AX_ROLE_BUTTON_DROP_DOWN"); // TODO: Remove this during the next Chromium update: https://chromium-review.googlesource.com/842475
    //QTest::newRow("AX_ROLE_CANVAS") << QString("<canvas width='10' height='10'></canvas>") << true << QAccessible::Canvas; // FIXME: The test case might be wrong (see AXLayoutObject.cpp)
    QTest::newRow("AX_ROLE_CAPTION") << QString("<table><caption>a</caption></table>") << false << QAccessible::Heading;
    //QTest::newRow("AX_ROLE_CARET"); // Not a blink accessibility role
    //QTest::newRow("AX_ROLE_CELL") << QString("<td role='cell'>a</td>") << true << QAccessible::Cell; // FIXME: Aria role 'cell' should work for <td>
    QTest::newRow("AX_ROLE_CHECK_BOX") << QString("<input type='checkbox'>a</input>") << false << QAccessible::CheckBox;
    QTest::newRow("AX_ROLE_CLIENT") << QString("") << true << QAccessible::Client;
    QTest::newRow("AX_ROLE_COLOR_WELL") << QString("<input type='color'>a</input>") << false << QAccessible::ColorChooser;
    //QTest::newRow("AX_ROLE_COLUMN") << QString("<table><tr><td>a</td></tr>") << true << QAccessible::Column; // FIXME: The test case might be wrong (see AXTableColumn.h)
    QTest::newRow("AX_ROLE_COLUMN_HEADER") << QString("<div role='columnheader'>a</div>") << true << QAccessible::ColumnHeader;
    QTest::newRow("AX_ROLE_COMBO_BOX_GROUPING") << QString("<div role='combobox'><input></div>") << true << QAccessible::ComboBox;
    QTest::newRow("AX_ROLE_COMBO_BOX_MENU_BUTTON") << QString("<div tabindex=0 role='combobox'>Select</div>") << true << QAccessible::ComboBox;
    QTest::newRow("AX_ROLE_TEXT_FIELD_WITH_COMBO_BOX") << QString("<input role='combobox'>") << false << QAccessible::ComboBox;
    QTest::newRow("AX_ROLE_COMPLEMENTARY") << QString("<aside>a</aside>") << true << QAccessible::ComplementaryContent;
    QTest::newRow("AX_ROLE_CONTENT_INFO") << QString("<address>a</address>") << true << QAccessible::Section;
    QTest::newRow("AX_ROLE_DATE") << QString("<input type='date'></input>") << false << QAccessible::Clock;
    QTest::newRow("AX_ROLE_DATE_TIME") << QString("<input type='datetime-local'></input>") << false << QAccessible::Clock;
    QTest::newRow("AX_ROLE_DEFINITION") << QString("<div role='definition'>landmark</div>") << true << QAccessible::Paragraph;
    QTest::newRow("AX_ROLE_DESCRIPTION_LIST") << QString("<dl>a</dl>") << true << QAccessible::List;
    QTest::newRow("AX_ROLE_DESCRIPTION_LIST_DETAIL") << QString("<dd>a</dd>") << true << QAccessible::Paragraph;
    QTest::newRow("AX_ROLE_DESCRIPTION_LIST_TERM") << QString("<dt>a</dt>") << true << QAccessible::ListItem;
    QTest::newRow("AX_ROLE_DETAILS") << QString("<details>a</details>") << true << QAccessible::Grouping;
    //QTest::newRow("AX_ROLE_DESKTOP"); // Not a blink accessibility role
    QTest::newRow("AX_ROLE_DIALOG") << QString("<div role='dialog'></div>") << true << QAccessible::Dialog;
    //QTest::newRow("AX_ROLE_DIRECTORY") << QString("<div role='directory'></div>") << true << QAccessible::NoRole; // FIXME: Aria role 'directory' should work
    QTest::newRow("AX_ROLE_DISCLOSURE_TRIANGLE") << QString("<details><summary>a</summary></details>") << false << QAccessible::NoRole;
    QTest::newRow("AX_ROLE_GENERIC_CONTAINER") << QString("<div>a</div>") << true << QAccessible::Section;
    QTest::newRow("AX_ROLE_DOCUMENT") << QString("<div role='document'>a</div>") << true << QAccessible::Document;
    QTest::newRow("AX_ROLE_EMBEDDED_OBJECT") << QString("<object width='10' height='10'></object>") << false << QAccessible::Grouping;
    QTest::newRow("AX_ROLE_FEED") << QString("<div role='feed'>a</div>") << true << QAccessible::Section;
    QTest::newRow("AX_ROLE_FIGCAPTION") << QString("<figcaption>a</figcaption>") << true << QAccessible::Heading;
    QTest::newRow("AX_ROLE_FIGURE") << QString("<figure>a</figure>") << true << QAccessible::Section;
    QTest::newRow("AX_ROLE_FOOTER") << QString("<footer>a</footer>") << true << QAccessible::Footer;
    QTest::newRow("AX_ROLE_FORM") << QString("<form></form>") << true << QAccessible::Form;
    QTest::newRow("AX_ROLE_GRID") << QString("<div role='grid'></div>") << true << QAccessible::Table;
    QTest::newRow("AX_ROLE_GROUP") << QString("<fieldset></fieldset>") << true << QAccessible::Grouping;
    QTest::newRow("AX_ROLE_HEADING") << QString("<h1>a</h1>") << true << QAccessible::Heading;
    QTest::newRow("AX_ROLE_IFRAME") << QString("<iframe>a</iframe>") << true << QAccessible::Section;
    QTest::newRow("AX_ROLE_IFRAME_PRESENTATIONAL") << QString("<iframe role='presentation'>a</iframe>") << false << QAccessible::NoRole;
    //QTest::newRow("AX_ROLE_IGNORED") << QString("<tag>a</tag>") << true << QAccessible::NoRole; // FIXME: The HTML element should not be exposed as an element (see AXNodeObject.cpp)
    QTest::newRow("AX_ROLE_IMAGE") << QString("<img>") << false << QAccessible::Graphic;
    //QTest::newRow("AX_ROLE_IMAGE_MAP") << QString("<map>a</map>") << true << QAccessible::Graphic; // FIXME: The test case might be wrong (see AXLayoutObject.cpp)
    QTest::newRow("AX_ROLE_INLINE_TEXT_BOX") << QString("<textarea rows='4' cols='50'></textarea>") << false << QAccessible::EditableText;
    QTest::newRow("AX_ROLE_INPUT_TIME") << QString("<input type='time'></input>") << false << QAccessible::SpinBox;
    QTest::newRow("AX_ROLE_LABEL_TEXT") << QString("<label>a</label>") << false << QAccessible::StaticText;
    QTest::newRow("AX_ROLE_LEGEND") << QString("<legend>a</legend>") << true << QAccessible::StaticText;
    QTest::newRow("AX_ROLE_LINE_BREAK") << QString("<br>") << false << QAccessible::Separator;
    QTest::newRow("AX_ROLE_LINK") << QString("<a href=''>link</a>") << false << QAccessible::Link;
    QTest::newRow("AX_ROLE_LIST") << QString("<ul></ul>") << true << QAccessible::List;
    QTest::newRow("AX_ROLE_LIST_BOX") << QString("<select multiple></select>") << false << QAccessible::ComboBox;
    QTest::newRow("AX_ROLE_LIST_BOX_OPTION") << QString("<option>a</option>") << true << QAccessible::ListItem;
    QTest::newRow("AX_ROLE_LIST_ITEM") << QString("<li>a</li>") << true << QAccessible::ListItem;
    QTest::newRow("AX_ROLE_LIST_MARKER") << QString("<li><ul></ul></li>") << false << QAccessible::StaticText;
    //QTest::newRow("AX_ROLE_LOCATION_BAR"); // Not a blink accessibility role
    QTest::newRow("AX_ROLE_LOG") << QString("<div role='log'>a</div>") << true << QAccessible::Section;
    QTest::newRow("AX_ROLE_MAIN") << QString("<main>a</main>") << true << QAccessible::Grouping;
    QTest::newRow("AX_ROLE_MARK") << QString("<mark>a</mark>") << false << QAccessible::StaticText;
    QTest::newRow("AX_ROLE_MARQUEE") << QString("<div role='marquee'>a</div>") << true << QAccessible::Section;
    QTest::newRow("AX_ROLE_MATH") << QString("<math>a</math>") << false << QAccessible::Equation;
    QTest::newRow("AX_ROLE_MENU") << QString("<div role='menu'>a</div>") << true << QAccessible::PopupMenu;
    QTest::newRow("AX_ROLE_MENU_BAR") << QString("<div role='menubar'>a</div>") << true << QAccessible::MenuBar;
    QTest::newRow("AX_ROLE_MENU_ITEM") << QString("<menu role='menu'><input type='button' /></menu>") << false << QAccessible::MenuItem;
    QTest::newRow("AX_ROLE_MENU_ITEM_CHECK_BOX") << QString("<menu role='menu'><input type='checkbox'></input></menu>") << false << QAccessible::CheckBox;
    QTest::newRow("AX_ROLE_MENU_ITEM_RADIO") << QString("<menu role='menu'><input type='radio'></input></menu>") << false << QAccessible::RadioButton;
    QTest::newRow("AX_ROLE_MENU_BUTTON") << QString("<menu role='group'><div role='menuitem'>a</div></menu>") << false << QAccessible::MenuItem;
    //QTest::newRow("AX_ROLE_MENU_LIST_OPTION") << QString("<select role='menu'><option>a</option></select>") << false << QAccessible::MenuItem; // FIXME: <select> should be a menu list (see AXMenuListOption.cpp)
    //QTest::newRow("AX_ROLE_MENU_LIST_POPUP") << QString("<menu type='context'>a</menu>") << true << QAccessible::PopupMenu; // FIXME: <menu> is not fully supported by Chromium (see AXMenuListPopup.h)
    QTest::newRow("AX_ROLE_METER") << QString("<meter>a</meter>") << false << QAccessible::Chart;
    QTest::newRow("AX_ROLE_NAVIGATION") << QString("<nav>a</nav>") << true << QAccessible::Section;
    QTest::newRow("AX_ROLE_NOTE") << QString("<div role='note'>a</div>") << true << QAccessible::Note;
    //QTest::newRow("AX_ROLE_PANE"); // Not a blink accessibility role
    QTest::newRow("AX_ROLE_PARAGRAH") << QString("<p>a</p>") << true << QAccessible::Paragraph;
    QTest::newRow("AX_ROLE_POP_UP_BUTTON") << QString("<select><option>a</option></select>") << false << QAccessible::ComboBox;
    QTest::newRow("AX_ROLE_PRE") << QString("<pre>a</pre>") << true << QAccessible::Section;
    //QTest::newRow("AX_ROLE_PRESENTATIONAL") << QString("<div role='presentation'>a</div>") << true << QAccessible::NoRole; // FIXME: Aria role 'presentation' should work
    QTest::newRow("AX_ROLE_PROGRESS_INDICATOR") << QString("<div role='progressbar' aria-valuenow='77' aria-valuemin='22' aria-valuemax='99'></div>") << true << QAccessible::ProgressBar;
    QTest::newRow("AX_ROLE_RADIO_BUTTON") << QString("<input type='radio'></input>") << false << QAccessible::RadioButton;
    QTest::newRow("AX_ROLE_RADIO_GROUP") << QString("<fieldset role='radiogroup'></fieldset>") << true << QAccessible::Grouping;
    QTest::newRow("AX_ROLE_REGION") << QString("<section>a</section>") << true << QAccessible::Section;
    //QTest::newRow("AX_ROLE_ROW") << QString("<tr role='row'>a</tr>") << true << QAccessible::Row; // FIXME: Aria role 'row' should work for <tr>
    //QTest::newRow("AX_ROLE_ROW_HEADER") << QString("<td role='rowheader'>a</td>") << true << QAccessible::RowHeader; // FIXME: Aria role 'rowheader' should work for <td>
    QTest::newRow("AX_ROLE_RUBY") << QString("<ruby>a</ruby>") << false << QAccessible::StaticText;
    QTest::newRow("AX_ROLE_SCROLL_BAR") << QString("<div role='scrollbar'>a</a>") << true << QAccessible::ScrollBar;
    QTest::newRow("AX_ROLE_SEARCH") << QString("<div role='search'>landmark</div>") << true << QAccessible::Section;
    QTest::newRow("AX_ROLE_SEARCH_BOX") << QString("<input type='search'></input>") << false << QAccessible::EditableText;
    QTest::newRow("AX_ROLE_SLIDER") << QString("<input type='range'></input>") << false << QAccessible::Slider;
    //QTest::newRow("AX_ROLE_SLIDER_THUMB"); // No mapping to ARIA role
    QTest::newRow("AX_ROLE_SPIN_BUTTON") << QString("<input type='number'></input>") << false << QAccessible::SpinBox;
    //QTest::newRow("AX_ROLE_SPIN_BUTTON_PART"); // No mapping to ARIA role
    QTest::newRow("AX_ROLE_SPLITER") << QString("<hr>") << true << QAccessible::Splitter;
    QTest::newRow("AX_ROLE_STATIC_TEXT") << QString("a") << false << QAccessible::StaticText;
    QTest::newRow("AX_ROLE_STATUS") << QString("<output>a</output>") << false << QAccessible::Indicator;
    QTest::newRow("AX_ROLE_SVG_ROOT") << QString("<svg width='10' height='10'></svg>") << false << QAccessible::Graphic;
    QTest::newRow("AX_ROLE_SWITCH") << QString("<button aria-checked='false'>a</button>") << false << QAccessible::Button;
    //QTest::newRow("AX_ROLE_TABLE") << QString("<table>a</table>") << true << QAccessible::Table; // FIXME: The test case might be wrong (see AXTable.cpp)
    //QTest::newRow("AX_ROLE_TABLE_HEADER_CONTAINER"); // No mapping to ARIA role
    QTest::newRow("AX_ROLE_TAB") << QString("<div role='tab'>a</div>") << true << QAccessible::PageTab;
    QTest::newRow("AX_ROLE_TAB_LIST") << QString("<div role='tablist'>a</div>") << true << QAccessible::PageTabList;
    QTest::newRow("AX_ROLE_TAB_PANEL") << QString("<div role='tab'>a</div>") << true << QAccessible::PageTab;
    QTest::newRow("AX_ROLE_TERM") << QString("<div role='term'>a</div>") << true << QAccessible::StaticText;
    QTest::newRow("AX_ROLE_TEXT_FIELD") << QString("<input type='text'></input>") << false << QAccessible::EditableText;
    QTest::newRow("AX_ROLE_TIME") << QString("<time>a</time>") << false << QAccessible::Clock;
    QTest::newRow("AX_ROLE_TIMER") << QString("<div role='timer'>a</div>") << true << QAccessible::Clock;
    //QTest::newRow("AX_ROLE_TITLE_BAR"); // Not a blink accessibility role
    QTest::newRow("AX_ROLE_TOGGLE_BUTTON") << QString("<button aria-pressed='false'>a</button>") << false << QAccessible::Button;
    QTest::newRow("AX_ROLE_TOOLBAR") << QString("<div role='toolbar'>a</div>") << true << QAccessible::ToolBar;
    QTest::newRow("AX_ROLE_TOOLTIP") << QString("<div role='tooltip'>a</div>") << true << QAccessible::ToolTip;
    QTest::newRow("AX_ROLE_TREE") << QString("<div role='tree'>a</div>") << true << QAccessible::Tree;
    QTest::newRow("AX_ROLE_TREE_GRID") << QString("<div role='treegrid'>a</div>") << true << QAccessible::Tree;
    QTest::newRow("AX_ROLE_TREE_ITEM") << QString("<div role='treeitem'>a</div>") << true << QAccessible::TreeItem;
    QTest::newRow("AX_ROLE_VIDEO") << QString("<video><source src='test.mp4' type='video/mp4'></video>") << false << QAccessible::Animation;
    //QTest::newRow("AX_ROLE_WINDOW"); // No mapping to ARIA role
}

void tst_Accessibility::roles()
{
    QFETCH(QString, html);
    QFETCH(bool, isSection);
    QFETCH(QAccessible::Role, role);

    QWebEngineView webView;
    webView.setHtml("<html><body>" + html + "</body></html>");
    QSignalSpy spyFinished(&webView, &QWebEngineView::loadFinished);
    QVERIFY(spyFinished.wait());

    QAccessibleInterface *view = QAccessible::queryAccessibleInterface(&webView);

    // Corner case for Client role
    if (html.isEmpty()) {
        QCOMPARE(view->role(), QAccessible::Client);
        return;
    }

    QTRY_COMPARE(view->child(0)->childCount(), 1);
    QAccessibleInterface *document = view->child(0);
    QAccessibleInterface *section = document->child(0);

    if (isSection) {
        QCOMPARE(section->role(), role);
        return;
    }

    QVERIFY(section->childCount() > 0);
    QAccessibleInterface *element = section->child(0);
    QCOMPARE(element->role(), role);
}

static QByteArrayList params = QByteArrayList()
    << "--force-renderer-accessibility";

W_QTEST_MAIN(tst_Accessibility, params)
#include "tst_accessibility.moc"
