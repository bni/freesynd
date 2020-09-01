/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2010  Benoit Blancard <benblan@users.sourceforge.net>*
 *                                                                      *
 *    This program is free software;  you can redistribute it and / or  *
 *  modify it  under the  terms of the  GNU General  Public License as  *
 *  published by the Free Software Foundation; either version 2 of the  *
 *  License, or (at your option) any later version.                     *
 *                                                                      *
 *    This program is  distributed in the hope that it will be useful,  *
 *  but WITHOUT  ANY WARRANTY;  without even  the implied  warranty of  *
 *  MERCHANTABILITY  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 *  General Public License for more details.                            *
 *                                                                      *
 *    You can view the GNU  General Public License, online, at the GNU  *
 *  project's  web  site;  see <http://www.gnu.org/licenses/gpl.html>.  *
 *  The full text of the license is also included in the file COPYING.  *
 *                                                                      *
 ************************************************************************/

#ifndef WIDGET_H
#define WIDGET_H

#include <string>
#include <list>

#include "keys.h"
#include "gfx/fontmanager.h"
#include "utils/seqmodel.h"

class Menu;

/*!
 * This is a graphical component. It's the base class
 * for all widgets (text, button, ...).
 * A widget has a size and location and can be visible or not.
 * Each widget has a unique id.
 */
class Widget {
public:

    /*!
     * Default constructor.
     */
    Widget() {
        id_ = ++widgetCnt;
        x_ = 0;
        y_ = 0;
        width_ = 0;
        height_ = 0;
        visible_ = true;
    }

    /*!
     * Constructs a widget with given size and location.
     * \param peer The parent menu.
     * \param x X coordinate
     * \param y Y coordinate
     * \param widht Widget width
     * \param height Widget height
     * \param visible True means the widget is drawn
     */
    Widget(Menu *peer, int x, int y, int width, int height, bool visible) {
        id_ = ++widgetCnt;
        x_ = x;
        y_ = y;
        width_ = width;
        height_ = height;
        visible_ = visible;
        peer_ = peer;
    }
    
    /*!
     * Default constructor.
     */
    virtual ~Widget() {}

    //! Draw the widget on screen. All subclass must implement this method.
    virtual void draw() = 0;

    /*!
     * Returns the widget id.
     */
    int getId() { return id_; }
    /*!
     * Returns the parent.
     */
    Menu * getPeer() { return peer_; }

    virtual void setLocation(int x, int y);
    int getX() { return x_; }
    int getY() { return y_; }

    int getWidth() { return width_; }
    int getHeight() { return height_; }

    void setVisible(bool visible);
    bool isVisible() { return visible_; }

protected:
    void redraw();

protected:
    /*! lower left coordinates of the widget.*/
    int x_, y_;
    /*! Width of the widget. */
    int width_;
    /*! Height of the widget. */
    int height_;
    /*! True if the widget is displayed on screen. */
    bool visible_;

private:
    /*! A counter to have unique widget IDs.*/
    static int widgetCnt;

    /*! A unique ID identifying the widget.*/
    int id_;
    /*! Every widget belongs to a menu.*/
    Menu *peer_;
};

//! A text widget.
/*!
 * This class represents a static text label : it displays a text
 * at a given location.
 * The text is displayed in dark or light green.
 * Text label can be left aligned or centered.
 */
class MenuText : public Widget {
public:
    /*!
     * Use this constructor to left align text. Widget width will depend on text and font.
     */
    MenuText(Menu *peer, int x, int y, const char *text, MenuFont *pFont, bool highlighted,
            bool visible = true);

    /*!
     * Use this constructor to specify text alignment. Widget width will be fixed but text position
     * will depend of font size and text length.
     */
    MenuText(Menu *peer, int x, int y, int width, const char *text, MenuFont *pFont, bool highlighted,
            bool visible = true, bool centered = true);

    virtual ~MenuText() {}

    //! Draw the widget on screen
    void draw();

    void setLocation(int x, int y);

    void setText(const char * text);
    void setTextFormated(const char * format, ...);
    std::string getText() { return text_; }

    void setHighlighted(bool highlighted);
    bool isHighlighted() { return highlighted_; }

    /*! Returns the font size. */
    MenuFont * getFont() { return pFont_; }

protected:
    /*!
     * Utility method to update text and compute associated data.
     */
    void updateText(const char *text);

protected:
    /*! The text to be displayed.*/
    std::string text_;
    bool highlighted_;
    /*! True means the text is centered, false the text is anchored
     * to the left.
     */
    bool centered_;
    /*! Real location of text.*/
    int anchorX_;
    /*! Real location of text.*/
    int anchorY_;
    /*! Font used to draw the text. */
    MenuFont *pFont_;
};

class Sprite;

/*!
 * A widget that responds to user interactions.
 */
class ActionWidget : public Widget {
public:
    ActionWidget(Menu *peer, int x, int y, int width, int height, bool visible, bool isEnabled = true) 
        : Widget(peer, x, y, width, height, visible) {
            enabled_ = isEnabled;
    }

    void setWidgetEnabled(bool enabled);

    bool isWidgetEnabled() { return enabled_; }

    //! Tells whether the pointer is over the widget or not
    bool isMouseOver(int x, int y);

    //! Callback method to respond to mouse motion event
    virtual void handleMouseMotion(int x, int y, int state, const int modKeys) {}
    //! Callback method to respond to mouse down event
    virtual void handleMouseDown(int x, int y, int button, const int modKeys) {}

    //! Callback method called when the mouse is over the widget
    virtual void handleFocusGained() {}
    //! Callback method called when the mouse leaves the widget
    virtual void handleFocusLost() {}

protected:
    bool enabled_;
};

//! A button widget.
/*!
 * This class represents a button. A button can lead to another screen 
 * which name is stored in the field "to".
 * It can have a widget in front of its text (like an arrow or a bullet).
 */
class Option : public ActionWidget {
public:

    //! Constructs a new button.
    Option(Menu *peer, int x, int y, int width, int height, const char *text, MenuFont *pFont,
            int to, bool visible, bool centered = true, int dark_widget = 0, int light_widget = 0);

    ~Option();

    Key getHotKey() { return hotKey_; }

    //! Draw the widget on screen
    void draw();

    void handleMouseDown(int x, int y, int button, const int modKeys);

    void handleFocusGained();
    void handleFocusLost();

    //! Runs the action for this widget
    virtual void executeAction(const int modKeys);

protected:

    MenuText text_;
    /*! The id of the next menu.*/
    int to_;
    /*! 
     * The widget to display when button is dark.
     * When id is zero, there is no widget.
    */
    Sprite *darkWidget_;
    /*! 
     * The widget to display when button is highlighted.
     * When id is zero, there is no widget.
     */
    Sprite *lightWidget_;
    /*!
     * This button can have an acceleration key.
     */
    Key hotKey_;
};

class ToggleAction;

/*!
 * A group is used to track the 
 * mutual exclusion between toggle actions.
 * Each time a button is selected, the others
 * must be unselected.
 */
class Group {
public:
    Group() {}
    ~Group();

    //! Adds a button to the group
    void addButton(ToggleAction *action);
    //! A toggle button has been selected
    void selectButton(int id);

private:
    /*! List of buttons that belongs to the group.*/
    std::list<ToggleAction *> actions_;
};

/*!
 * ToggleAction are buttons that stay pushed when clicked
 * but gets deselected when another button from the same group
 * gets selected.
 */
class ToggleAction : public Option {
public:
    //! Constructs a new button.
    ToggleAction(Menu *peer, int x, int y, int width, int height, 
                    const char *text, MenuFont *pFont, bool selected, Group *pGroup);

    void executeAction(const int modKeys);

    void handleFocusLost();

    void handleSelectionLost();

    void handleSelectionAquire();
protected:
    //! True to select the button
    void setSelected(bool isSelected);
protected:
    /*! State of the button. True means the button is on.*/
    bool selected_;
    /*! Group of the button.*/
    Group *group_;
};

/*!
 * A list box is a widget that displays a list of lines. Each line corresponds to
 * a game object (equips, mods, ...). A list box reacts to user actions
 * and calls Menu::handleAction() if one of its items has been pressed.
 */
class ListBox : public ActionWidget , public ModelListener {
public:
    //! Constructs a new list box.
    ListBox(Menu *peer, int x, int y, int width, int height, MenuFont *pFont, bool visible = true);

    virtual ~ListBox();

    //! Draw the widget on screen
    void draw(); 

    //! Callback method to respond to mouse motion event
    void handleMouseMotion(int x, int y, int state, const int modKeys);
    //! Callback method to respond to mouse down event
    void handleMouseDown(int x, int y, int button, const int modKeys);
    void handleFocusLost();

    void setModel(SequenceModel *pModel);
    void handleModelChanged();

protected:
    /*! Lines displayed in the list box.*/
    std::list<std::string> labels_;
    /*! Data that are displayed in the list box.*/
    SequenceModel *pModel_;
    /*! The line that the mouse is on. -1 if no line is hovered.*/
    int focusedLine_;
    /*! Font used to draw the text. */
    MenuFont *pFont_;
};

/*!
 * 
 * A Team list box has a title, displayed at the top of the list and prints a default
 * label for all empty lines.
 */
class TeamListBox : public ListBox {
public:
    //! Constructs a new list box.
    TeamListBox(Menu *peer, int x, int y, int width, int height, MenuFont *pFont, bool visible = true);

    ~TeamListBox();

    //! Draw the widget on screen
    void draw(); 

    //! Callback method to respond to mouse motion event
    void handleMouseMotion(int x, int y, int state, const int modKeys);

    void setSquadLine(int squadSlot, unsigned int line);

protected:
    static const int LINE_OFFSET;
    /*! Title of the list box. Optional.*/
    MenuText *pTitle_;
    /*! The label for an empty line.*/
    std::string emptyLbl_;
    /*! X coord for title underline.*/
    int xUnderline_;
    /*! Y coord for title underline.*/
    int yUnderline_;
    /*! Length of title underline.*/
    int lUnderline_;
    /*! Starting coordinate of content list.*/
    int yOrigin_;
    /*! Stores the lines for the current squad members.*/
    int squadLines_[4];
};

//! A text field widget.
/*!
 * 
 */
class TextField : public ActionWidget {
public:
    static void setEmptyLabel(std::string str) { emptyLbl_ = str; }

    //! Constructs a new textfield.
    TextField(Menu *peer, int x, int y, int width, int height, MenuFont *pFont,
            int maxSize, bool displayEmpty, bool visible);

    ~TextField();

    void setText(const char* text);
    std::string getText() { return text_.getText(); }

    void setHighlighted(bool highlighted) { text_.setHighlighted(highlighted); }
    bool isHighlighted() { return text_.isHighlighted(); }

    //! Draw the widget on screen
    void draw();

    void handleMouseDown(int x, int y, int button, const int modKeys);

    void handleCaptureGained();
    void handleCaptureLost();

    bool handleKey(Key key, const int modKeys);

protected:
    void drawCaret();

    // Removes one character backward
    void handleBackSpace();
    // Removes one character forward
    void handleDelete();
    // Insert new character at caret position
    void handleCharacter(Key key);

protected:
    /*! Label for empty lines.*/
    static std::string emptyLbl_;

    /*! This holds the text value.*/
    MenuText text_;
    /*! Displays or not a default string if textfield is empty.*/
    bool isDisplayEmpty_;
    /*! Position of caret in the name.*/
    size_t caretPosition_;
    /*! Vertical position of the caret.*/
    int yCaret_;
    /*! Tells whether the field is being edited.*/
    bool isInEdition_;
    /*! Maximum size of the text.*/
    size_t maxSize_;
};

#endif // WIDGET_H
