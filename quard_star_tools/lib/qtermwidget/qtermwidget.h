/*  
 Copyright (C) 2008 e_k (e_k@users.sourceforge.net)

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
#ifndef _Q_TERM_WIDGET
#define _Q_TERM_WIDGET

#include <QTranslator>
#include <QLocale>
#include <QWidget>
#include <QClipboard>
#include <QTimer>
#include "Emulation.h"
#include "Filter.h"

class QVBoxLayout;
class SearchBar;
class Session;
class TerminalDisplay;
class Emulation;
class QUrl;

class QTermWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * This enum describes the location where the scroll bar is positioned in the display widget.
     */
    enum ScrollBarPosition {
        /** Do not show the scroll bar. */
        NoScrollBar = 0,
        /** Show the scroll bar on the left side of the display. */
        ScrollBarLeft = 1,
        /** Show the scroll bar on the right side of the display. */
        ScrollBarRight = 2
    };
    enum UrlActivatedType {
        OpenFromContextMenu = 0,
        OpenContainingFromContextMenu = 1,
        OpenFromClick = 2
    };

    using KeyboardCursorShape = Emulation::KeyboardCursorShape;

    //Creation of widget
    QTermWidget(QWidget *messageParentWidget = nullptr, QWidget *parent = nullptr);

    ~QTermWidget() override;

    //Initial size
    QSize sizeHint() const override;

    // expose TerminalDisplay::TerminalSizeHint, setTerminalSizeHint
    void setTerminalSizeHint(bool enabled);
    bool terminalSizeHint();

    //look-n-feel, if you don`t like defaults

    //  Terminal font
    // Default is application font with family Monospace, size 10
    // Beware of a performance penalty and display/alignment issues when using a proportional font.
    void setTerminalFont(const QFont & font);
    QFont getTerminalFont();
    void setTerminalOpacity(qreal level);
    void setTerminalBackgroundImage(const QString& backgroundImage);
    void setTerminalBackgroundMovie(const QString& backgroundMovie);
    void setTerminalBackgroundVideo(const QString& backgroundVideo);
    void setTerminalBackgroundMode(int mode);

    //Text codec, default is UTF-8
    void setTextCodec(QStringEncoder codec);

    /** @brief Sets the color scheme, default is white on black
     *
     * @param[in] name The name of the color scheme, either returned from
     * availableColorSchemes() or a full path to a color scheme.
     */
    void setColorScheme(const QString & name);

    /**
     * @brief Retrieves the available color schemes in the OS for the terminal.
     *
     * @note This function is needed in addition to the static one for making it availble when accessing QTermWidget as a plugin.
     *
     * @return A list of color schemes.
     */
    QStringList getAvailableColorSchemes();
    static QStringList availableColorSchemes();

    void setBackgroundColor(const QColor &color);
    void setForegroundColor(const QColor &color);
    void setANSIColor(const int ansiColorId, const QColor &color);

    void setPreeditColorIndex(int index);

    /** Sets the history size (in lines)
     *
     * @param lines history size
     *  lines = 0, no history
     *  lies < 0, infinite history
     */
    void setHistorySize(int lines);

    // Returns the history size (in lines)
    int historySize() const;

    // Presence of scrollbar
    void setScrollBarPosition(ScrollBarPosition);

    // Wrapped, scroll to end.
    void scrollToEnd();

    // Send some text to terminal
    void sendText(const QString & text);

    // Send key event to terminal
    void sendKeyEvent(QKeyEvent* e);

    // Sets whether flow control is enabled
    void setFlowControlEnabled(bool enabled);

    // Returns whether flow control is enabled
    bool flowControlEnabled(void);

    /**
     * Sets whether the flow control warning box should be shown
     * when the flow control stop key (Ctrl+S) is pressed.
     */
    void setFlowControlWarningEnabled(bool enabled);

    /*! Get all available keyboard bindings
     */
    static QStringList availableKeyBindings();

    //! Return current key bindings
    QString keyBindings();

    void setMotionAfterPasting(int);

    /** Return the number of lines in the history buffer. */
    int historyLinesCount();

    int screenColumnsCount();
    int screenLinesCount();

    void setSelectionStart(int row, int column);
    void setSelectionEnd(int row, int column);
    void getSelectionStart(int& row, int& column);
    void getSelectionEnd(int& row, int& column);

    /**
     * Returns the currently selected text.
     * @param preserveLineBreaks Specifies whether new line characters should
     * be inserted into the returned text at the end of each terminal line.
     */
    QString selectedText(bool preserveLineBreaks = true);

    void setMonitorActivity(bool);
    void setMonitorSilence(bool);
    void setSilenceTimeout(int seconds);

    /** Returns the available hotspot for the given point \em pos.
     *
     * This method may return a nullptr if no hotspot is available.
     *
     * @param[in] pos The point of interest in the QTermWidget coordinates.
     * @return Hotspot for the given position, or nullptr if no hotspot.
     */
    Filter::HotSpot* getHotSpotAt(const QPoint& pos) const;

    /** Returns the available hotspots for the given row and column.
     *
     * @return Hotspot for the given position, or nullptr if no hotspot.
     */
    Filter::HotSpot* getHotSpotAt(int row, int column) const;

    /*
     * Proxy for TerminalDisplay::filterActions
     * */
    QList<QAction*> filterActions(const QPoint& position);

    int recvData(const char *buff, int len) const;

    /**
     * Sets the shape of the keyboard cursor.  This is the cursor drawn
     * at the position in the terminal where keyboard input will appear.
     */
    void setKeyboardCursorShape(KeyboardCursorShape shape);
    void setKeyboardCursorShape(uint32_t shape);

    void setBlinkingCursor(bool blink);

    /** Enables or disables bidi text in the terminal. */
    void setBidiEnabled(bool enabled);
    bool isBidiEnabled();

    /** change and wrap text corresponding to paste mode **/
    void bracketText(QString& text);

    /** forcefully disable bracketed paste mode **/
    void disableBracketedPasteMode(bool disable);
    bool bracketedPasteModeIsDisabled() const;

    /** Set the empty space outside the terminal */
    void setMargin(int);

    /** Get the empty space outside the terminal */
    int getMargin() const;

    void setDrawLineChars(bool drawLineChars);

    void setBoldIntense(bool boldIntense);

    void setConfirmMultilinePaste(bool confirmMultilinePaste);
    void setTrimPastedTrailingNewlines(bool trimPastedTrailingNewlines);
    void setEcho(bool echo);
    void setKeyboardCursorColor(bool useForegroundColor, const QColor& color);
    void proxySendData(QByteArray data) {
        emit sendData(data.data(), data.size());
    }

    void setLocked(bool enabled);

    void setSelectionOpacity(qreal opacity);

    void addHighLightText(const QString &text, const QColor &color);
    bool isContainHighLightText(const QString &text);
    void removeHighLightText(const QString &text);
    void clearHighLightTexts(void);
    QMap<QString, QColor> getHighLightTexts(void);

    void setWordCharacters(const QString &wordCharacters);
    QString wordCharacters(void);
    void setShowResizeNotificationEnabled(bool enabled);

    void setEnableHandleCtrlC(bool enable);

    int lines();
    int columns();
    int getCursorX();
    int getCursorY();
    void setCursorX(int x);
    void setCursorY(int y);

    QString screenGet(int row1, int col1, int row2, int col2, int mode);

    void setUrlFilterEnabled(bool enable);

    void setMessageParentWidget(QWidget *parent);
    void reTranslateUi(void);
    void set_fix_quardCRT_issue33(bool fix);

signals:
    void finished();
    void copyAvailable(bool);
    void termGetFocus();
    void termLostFocus();
    void termKeyPressed(QKeyEvent *);
    void urlActivated(const QUrl&, uint32_t opcode);
    void notifyBell();
    void activity();
    void silence();
    /**
     * Emitted when the activity state of this session changes.
     *
     * @param state The new state of the session.  This may be one
     * of NOTIFYNORMAL, NOTIFYSILENCE or NOTIFYACTIVITY
     */
    void stateChanged(int state);
    /**
     * Emitted when the flow control state changes.
     *
     * @param enabled True if flow control is enabled or false otherwise.
     */
    void flowControlEnabledChanged(bool enabled);
    /**
     * Emitted when emulator send data to the terminal process
     * (redirected for external recipient). It can be used for
     * control and display the remote terminal.
     */
    void sendData(const char *,int);
    void dupDisplayOutput(const char* data,int len);
    void profileChanged(const QString & profile);
    void titleChanged(int title,const QString& newTitle);
    void changeTabTextColorRequest(int);
    void termSizeChange(int lines, int columns);
    void mousePressEventForwarded(QMouseEvent* event);
    void zmodemSendDetected();
    void zmodemRecvDetected();
    void handleCtrlC(void);

public slots:
    // Copy terminal to clipboard
    void copyClipboard();
    // Copy terminal to selection
    void copySelection();
    // Paste clipboard to terminal
    void pasteClipboard();
    // Paste selection to terminal
    void pasteSelection();
    // Select all text
    void selectAll();
    // Set zoom
    int zoomIn();
    int zoomOut();
    // Set size
    void setSize(const QSize &);
    /*! Set named key binding for given widget
     */
    void setKeyBindings(const QString & kb);
    /*! Clear the terminal content and move to home position
     */
    void clearScrollback();
    void clearScreen();
    void clear();
    void toggleShowSearchBar();
    void saveHistory(QIODevice *device, int format = 0, int start = -1, int end = -1);
    void saveHistory(QTextStream *stream, int format = 0, int start = -1, int end = -1);
    void screenShot(QPixmap *pixmap);
    void screenShot(const QString &fileName);
    void repaintDisplay(void);

protected:
    void resizeEvent(QResizeEvent *) override;

protected slots:
    void sessionFinished();
    void updateTerminalSize();
    void selectionChanged(bool textSelected);
    void monitorTimerDone();
    void activityStateSet(int);

private slots:
    /**
     * Emulation::cursorChanged() signal propagates to here and QTermWidget
     * sends the specified cursor states to the terminal display
     */
    void cursorChanged(Emulation::KeyboardCursorShape cursorShape, bool blinkingCursorEnabled);

private:
    class HighLightText {
    public:
        HighLightText(const QString& text, const QColor& color) : text(text), color(color) {
            regExpFilter = new RegExpFilter();
            regExpFilter->setRegExp(QRegularExpression(text));
            regExpFilter->setColor(color);
        }
        ~HighLightText() {
            delete regExpFilter;
        }
        QString text;
        QColor color;
        RegExpFilter *regExpFilter;
    };
    void search(bool forwards, bool next);
    int setZoom(int step);
    QWidget *messageParentWidget = nullptr;
    TerminalDisplay *m_terminalDisplay = nullptr;
    Emulation  *m_emulation = nullptr;
    SearchBar* m_searchBar = nullptr;
    QVBoxLayout *m_layout = nullptr;
    QList<HighLightText*> m_highLightTexts;
    bool m_echo = false;
    UrlFilter *m_urlFilter = nullptr;
    bool m_UrlFilterEnable = true;
    bool m_flowControl = true;
    // Color/Font Changes by ESC Sequences
    bool m_hasDarkBackground = true;
    bool m_monitorActivity = false;
    bool m_monitorSilence = false;
    bool m_notifiedActivity = false;
    QTimer* m_monitorTimer = nullptr;
    int m_silenceSeconds = 10;

    const static int STEP_ZOOM = 3;
};

#endif
