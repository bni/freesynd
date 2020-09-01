#include <stdarg.h>

#include "menus/widget.h"
#include "menus/menu.h"
#include "menus/menumanager.h"
#include "gfx/screen.h"
#include "utils/utf8.h"
#include "appcontext.h"

int Widget::widgetCnt = 0;

/*!
 * Utility method to add a dirty rect to the menu.
 * Adds rect only if widget is visible.
 */
void Widget::redraw() {
    if (visible_) {
        getPeer()->addDirtyRect(x_, y_, width_, height_);
    }
}

void Widget::setLocation(int x, int y) {
    x_ = x;
    y_ = y;
}

void Widget::setVisible(bool visible) {
    if (visible != visible_) {
        // forcing widget update, if it becomes invisible widget erased,
        // if becomes visible widget is drawn
        getPeer()->addDirtyRect(x_, y_, width_, height_);
        visible_ = visible;
    }
}

MenuText::MenuText(Menu *peer, int x, int y, const char *text, MenuFont *pFont, 
                   bool highlighted, bool visible): 
            Widget(peer, x, y, 0, 0, visible)
{
        highlighted_ = highlighted;
        centered_ = false;
        anchorX_ = x;
        anchorY_ = y;
        pFont_ = pFont;
        // Height is fixed by font size
        height_ = pFont->textHeight();

        updateText(text);
}

MenuText::MenuText(Menu *peer, int x, int y, int width, const char *text, MenuFont *pFont, 
                   bool highlighted, bool visible, bool centered): 
            Widget(peer, x, y, width, 0, visible)
{
        
    highlighted_ = highlighted;
    centered_ = centered;
    anchorX_ = x;
    anchorY_ = y;
    pFont_ = pFont;
    // Height is fixed by font size
    height_ = pFont->textHeight();

    updateText(text);
}

void MenuText::updateText(const char *text) {
    std::string lbl(text);
    // Find if string starts with '#' caracter
    if (lbl.find_first_of('#') == 0) {
        // Erase the # caracter
        lbl.erase(0, 1);
        // and looks for the message in the langage file
        g_Ctx.getMessage(lbl.c_str(), lbl);
    }
    text_ = lbl;

    int textWidth = pFont_->textWidth(text_.c_str(), false);
    if (textWidth > width_) {
        width_ = textWidth;
    }

    if (centered_) {
        anchorX_ = (x_ + x_ + width_) / 2  - textWidth / 2;
    }
}

/*! 
 * Modify the MenuText text.
 * If the given text starts with a '#', the remaining
 * text identifies a key in the language file.
 * \param text to set
 */
void MenuText::setText(const char *text) {
    updateText(text);
    redraw();
}

/*! 
 * Modify the MenuText text.
 * The given text can be a formated string.
 * If it starts with a '#', the remaining
 * text identifies a key in the language file.
 * \param format message format
 */
void MenuText::setTextFormated(const char * format, ...) {

    char tmp[200];
    va_list list;

    std::string lbl(format);
    // Find if string starts with '#' caracter
    if (lbl.find_first_of('#') == 0) {
        // Erase the # caracter
        lbl.erase(0, 1);
        // and looks for the message in the langage file
        g_Ctx.getMessage(lbl.c_str(), lbl);
    }

    va_start(list, format);
    vsprintf(tmp, lbl.c_str(), list);
    va_end(list);

    setText(tmp);
}

void MenuText::setLocation(int x, int y) {
    int dx = x - x_;
    int dy = y - y_;
    x_ = x;
    y_ = y;
    anchorX_ += dx;
    anchorY_ += dy;
}

void MenuText::setHighlighted(bool highlighted) {
    highlighted_ = highlighted;
    redraw();
}

/*!
 * Draw the widget at the current position.
 * Actually, only a text is drawn (see Font). The borders are
 * already drawn on the background image.
 */
void MenuText::draw() {
    pFont_->drawText(anchorX_, anchorY_, text_.c_str(), highlighted_);
}

bool ActionWidget::isMouseOver(int x, int y) {
 
    return (x > x_  && 
            x < x_ + width_ && 
            y >= y_ && 
            y < y_ + height_);
}

void ActionWidget::setWidgetEnabled(bool enabled) {
    if (enabled != enabled_) {
        enabled_ = enabled;
        redraw();
    }
}

Option::Option(Menu *peer, int x, int y, int width, int height, const char *text,
    MenuFont *pFont, int to, bool visible, bool centered, int darkWidgetId,
    int lightWidgetId) : ActionWidget(peer, x, y, width, height, visible),
    text_(peer, x, y, width - 4, text, pFont, false, true, centered)
{
        to_ = to;
        darkWidget_ = NULL;
        lightWidget_ = NULL;
        hotKey_.keyFunc = KFC_UNKNOWN;
        hotKey_.unicode = 0;

        // If button label contains a '&' caracter, then the next
        // caracter is the acceleration key for that button
        // only 'a-zA-Z' characters are available
        char src[100];
        size_t size = text_.getText().size();

        memset(src, 0, 100);
        memcpy(src, text_.getText().c_str(), size);
        size_t nbCdpt = utf8::distance(src, src + size);

        char tmp[100];
        char *itSrc = src;
        char *itDst = tmp;
        memset(tmp, 0, 100);

        size_t i = 0;
        bool foundAmp = false;
        while (i<nbCdpt) {
            i++;
            int cp = utf8::next(itSrc, src + size);
            if (cp == '&') {
                if (!foundAmp) {
                    // found '&' : skip it
                    foundAmp = true;
                    continue;
                }
            } else if (foundAmp && hotKey_.unicode == 0) {
                if (cp >= 'A' && cp <= 'z') {
                    hotKey_.unicode = cp;
                }
            }
            // copy char
            itDst = utf8::append(cp, itDst);
        }
        text_.setText(tmp);

        //////////////////////////



        // Position the button text in the middle of height
        // add 1 pixel to height to compensate lost of division
        text_.setLocation(text_.getX(), y_ + (height_ / 2) - (text_.getHeight() / 2) + 1);

        if (darkWidgetId != 0) {
            darkWidget_ = peer->menuSprites().sprite(darkWidgetId);
            lightWidget_ = peer->menuSprites().sprite(lightWidgetId);
            // there's a small pad between heading widget ant text
            text_.setLocation(text_.getX() + darkWidget_->width() * 2 + 8, text_.getY());
        }
}

Option::~Option() {
    to_ = 0; 
}
/*!
 * Draw the widget at the current position.
 * Actually, only a text is drawn (see Font). The borders are
 * already drawn on the background image.
 */
void Option::draw() {
    int x = x_;

    if (text_.isHighlighted() && lightWidget_ != NULL) {
        lightWidget_->draw(x, y_, 0, false, true);
    } else if (darkWidget_ != NULL) {
        darkWidget_->draw(x, y_, 0, false, true);
    }

    text_.draw();
}

/*!
 * Calls Menu::handleAction() then redirect to
 * another menu if the field "to" has been set.
 */
void Option::executeAction(const int modKeys) {
    if (getPeer() && this->isVisible()) {
        getPeer()->handleAction(getId(), NULL, modKeys);
    }

    if (to_ != -1) {
        getPeer()->getMenuManager()->gotoMenu(to_);
        return;
    }
}

void Option::handleFocusGained() {
    text_.setHighlighted(true);
    redraw();
}
void Option::handleFocusLost() {
    text_.setHighlighted(false);
    redraw();
}

void Option::handleMouseDown(int x, int y, int button, const int modKeys) {
    executeAction(modKeys);
}

Group::~Group() {
    actions_.clear();
}

void Group::addButton(ToggleAction *pAction) {
    actions_.push_back(pAction);
}

void Group::selectButton(int id) {
    for (std::list < ToggleAction * >::iterator it = actions_.begin();
         it != actions_.end(); it++) {
        ToggleAction *pAction = *it;

        if (pAction->getId() != id ) {
            pAction->handleSelectionLost();
        } else
            pAction->handleSelectionAquire();
    }
}

ToggleAction::ToggleAction(Menu *peer, int x, int y, int width, int height, 
                            const char *text, MenuFont *pFont, bool selected, Group *pGroup)
: Option(peer, x, y, width, height, text, pFont, -1, true) {
    group_ = pGroup;
    setSelected(selected);
}

void ToggleAction::setSelected(bool isSelected) {
    selected_ = isSelected;
    // When a ToggleAction is selected it lighted
    text_.setHighlighted(selected_);
    redraw();
}

void ToggleAction::executeAction(const int modKeys) {
    // Deselect all other buttons of the group and select current
    group_->selectButton(getId());
    Option::executeAction(modKeys);
}

void ToggleAction::handleFocusLost() {
    // Toggle buttons get dark only
    // if they are not pushed
    if (!selected_) {
        Option::handleFocusLost();
    }
}

void ToggleAction::handleSelectionLost() {
    setSelected(false);
}

void ToggleAction::handleSelectionAquire() {
    setSelected(true);
}

ListBox::ListBox(Menu *peer, int x, int y, int width, int height, MenuFont *pFont, bool visible) :
        ActionWidget(peer, x, y, width, height, visible) {
    focusedLine_ = -1;
    pModel_ = NULL;
    pFont_ = pFont;
}

ListBox::~ListBox() {
    labels_.clear();

    if (pModel_) {
        pModel_->removeModelListener(this);
    }
}

void ListBox::setModel(SequenceModel *pModel) {
    pModel_ = pModel;
    labels_.clear();
    pModel_->getLabels(labels_);
    pModel_->addModelListener(this);
}

void ListBox::handleModelChanged() {
    labels_.clear();
    pModel_->getLabels(labels_);
    redraw();
}

//! Draw the widget on screen
void ListBox::draw() {
    int i=0;
    for (std::list < std::string >::iterator it = labels_.begin();
         it != labels_.end(); it++, i++) {
             pFont_->drawText(getX(), getY() + i * 12, (*it).c_str(), focusedLine_ == i);
    }
}

void ListBox::handleMouseMotion(int x, int y, int state, const int modKeys) {
    
    if (pModel_) {
        // Gets the line pointed by the mouse
        unsigned int i = (y - getY()) / 12;
        if (i < pModel_->size()) {
            if (pModel_->getElement(i)) {
                // If line contains something, highlight it
                if (focusedLine_ != (int)i) {
                    redraw();
                    focusedLine_ = i;
                }
            } else {
                if (focusedLine_ != -1) {
                    redraw();
                    focusedLine_ = -1;
                }
            }
        }  else if (focusedLine_ != -1) {
            // A line was highlighted but not anymore
            redraw();
            focusedLine_ = -1;
        }
    }
}

void ListBox::handleMouseDown(int x, int y, int button, const int modKeys) {
    if (focusedLine_ != -1 && pModel_) {
        if (getPeer()) {
            // call the peer handleAction method giving the index of pressed line.
            std::pair<int, void *> tuple = std::make_pair(focusedLine_, pModel_->getElement(focusedLine_));
            getPeer()->handleAction(getId(), &tuple, modKeys);
            
        }
    }
}

void ListBox::handleFocusLost() {
    focusedLine_ = -1;
    redraw();
}

const int TeamListBox::LINE_OFFSET = 20;

TeamListBox::TeamListBox(Menu *peer, int x, int y, int width, int height, MenuFont *pFont, bool visible) :
        ListBox(peer, x, y, width, height, pFont, visible) {
    pTitle_ = new MenuText(peer, x, y, width, "#SELECT_CRYO_TITLE", pFont, true);
    lUnderline_ = pFont_->textWidth(pTitle_->getText().c_str(), false);
    xUnderline_ = (x + x + width) / 2  - lUnderline_ / 2;
    yUnderline_ = y + pFont_->textHeight();
    yOrigin_ = yUnderline_ + 2;
    for (int i=0; i<4; i++) {
        squadLines_[i] = i;
    }

    g_Ctx.getMessage("MENU_LB_EMPTY", emptyLbl_);
}

TeamListBox::~TeamListBox() {
    delete pTitle_;
    pTitle_ = NULL;
}

//! Draw the widget on screen
void TeamListBox::draw() {

    pTitle_->draw();
    g_Screen.drawRect(xUnderline_, yUnderline_, lUnderline_, 2, 252);

    int i=0;
    for (std::list < std::string >::iterator it = labels_.begin();
         it != labels_.end(); it++, i++) {
             if ((*it).size() != 0) {
                 for (int ln=0; ln<4; ln++) {
                     if (squadLines_[ln] ==  i) {
                        char tmp[5];
                        sprintf(tmp, "%d", ln+1);
                        pFont_->drawText(getX(), yOrigin_ + i * 12, tmp, focusedLine_ == i);
                        break;
                     }
                 }
                 pFont_->drawText(getX() + LINE_OFFSET, yOrigin_ + i * 12, (*it).c_str(), focusedLine_ == i);
             } else {
                 pFont_->drawText(getX() + LINE_OFFSET, yOrigin_ + i * 12, emptyLbl_.c_str(), focusedLine_ == i);
             }
    }
}

void TeamListBox::handleMouseMotion(int x, int y, int state, const int modKeys) {
    
    if (pModel_) {
        // Gets the line pointed by the mouse
        int i = (y - yOrigin_) / 12;
        if (i >= 0 && i < (int) pModel_->size()) {
            if (pModel_->getElement(i)) {
                // If line contains something, highlight it
                if (focusedLine_ != i) {
                    redraw();
                    focusedLine_ = i;
                }
            } else {
                if (focusedLine_ != -1) {
                    redraw();
                    focusedLine_ = -1;
                }
            }
        }  else if (focusedLine_ != -1) {
            // A line was highlighted but not anymore
            redraw();
            focusedLine_ = -1;
        }
    }
}

void TeamListBox::setSquadLine(int squadSlot, unsigned int line) {
    if (pModel_ && line < pModel_->size() && squadSlot >= 0 && squadSlot < 4) {
        squadLines_[squadSlot] = line;
        redraw();
    }
}

// By default there's no empty label
std::string TextField::emptyLbl_ = "";

TextField::TextField(Menu *peer, int x, int y, int width, int height, MenuFont *pFont,
            int maxSize, bool displayEmpty, bool visible) 
            : ActionWidget(peer, x, y, width, height, visible), text_(peer, x, y, width, "", pFont, false, visible, false) {

    // Position the button text in the middle of height
    // add 1 pixel to height to compensate lost of division
    text_.setLocation(text_.getX(), y_ + (height_ / 2) - (text_.getHeight() / 2) + 1);
    isDisplayEmpty_ = displayEmpty;
    caretPosition_ = 0;
    yCaret_ = text_.getY() + text_.getFont()->textHeight(true);
    isInEdition_ = false;
    maxSize_ = maxSize;
}

TextField::~TextField() {
}

/*!
 * Draw the widget at the current position.
 * Actually, only a text is drawn (see Font). The borders are
 * already drawn on the background image.
 */
void TextField::draw() {
    if (!isInEdition_ && isDisplayEmpty_ && text_.getText().size() == 0) {
        text_.getFont()->drawText(getX(), text_.getY(), emptyLbl_.c_str(), false);
    } else {
        text_.draw();
        if (isInEdition_) {
            drawCaret();
        }
    }
}

void TextField::handleCaptureGained() {
    isInEdition_ = true;
    // by default set the caret at the end of the text
    char src[100];
    size_t size = text_.getText().size();
    memset(src, 0, 100);
    memcpy(src, text_.getText().c_str(), size);
    caretPosition_ = utf8::distance(src, src + size);

    setHighlighted(true);
    redraw();
}

void TextField::handleCaptureLost() {
    isInEdition_ = false;
    caretPosition_ = 0;
    setHighlighted(false);
    redraw();
}

/*!
 * Erase one character before caret.
 */
void TextField::handleBackSpace() {
    // Erase character only if caret is not on the first character
    if (caretPosition_ > 0) {
        char src[100];
        size_t size = text_.getText().size();
        memset(src, 0, 100);
        memcpy(src, text_.getText().c_str(), size);

        char tmp[100];
        char *itSrc = src;
        char *itDst = tmp;
        memset(tmp, 0, 100);

        size_t nbCdpt = utf8::distance(src, src + size);

        size_t i = 0;
        while (i<nbCdpt) {
            int cp = utf8::next(itSrc, src + size);
            if (i != caretPosition_ - 1) {
                itDst = utf8::append(cp, itDst);
            }
            i++;
        }
            
        setText(tmp);
        caretPosition_--;
    }
}

/*!
 * Erase one character after caret.
 */
void TextField::handleDelete() {
    char src[100];
    size_t size = text_.getText().size();

    memset(src, 0, 100);
    memcpy(src, text_.getText().c_str(), size);
    size_t nbCdpt = utf8::distance(src, src + size);

    // Erase one character only if caret is not at the end
    if (caretPosition_ < nbCdpt) {
        char tmp[100];
        char *itSrc = src;
        char *itDst = tmp;
        memset(tmp, 0, 100);

        size_t i = 0;
        while (i<nbCdpt) {
            int cp = utf8::next(itSrc, src + size);
            if (i != caretPosition_) {
                itDst = utf8::append(cp, itDst);
            }
            i++;
        }
            
        setText(tmp);
    }
}

// Insert new character at caret position
void TextField::handleCharacter(Key key) {
    // Key can be displayed -> insert it at the caret position
    char src[100];
    size_t size = text_.getText().size();
    memset(src, 0, 100);
    memcpy(src, text_.getText().c_str(), size);
    char *itSrc = src;
    size_t nbCdpt = utf8::distance(src, src + size);
    if (nbCdpt < maxSize_) {
        char tmp[100];
        char *itDst = tmp;
        memset(tmp, 0, 100);

        size_t i = 0;
        while(i<caretPosition_) {
            int cp = utf8::next(itSrc, src + size);
            itDst = utf8::append(cp, itDst);
            i++;
        }
        // Add the new key
        if (key.unicode >= 0x0061 && key.unicode <= 0x007A) {
            // If the key is a letter between 'a' and 'z'
            // => capitalize it because MenuFont only displays capital letters
            itDst = utf8::append(key.unicode - 32, itDst);
        } else {
            itDst = utf8::append(key.unicode, itDst);
        }
            
        while(i<nbCdpt) {
            int cp = utf8::next(itSrc, src + size);
            itDst = utf8::append(cp, itDst);
            i++;
        }
        setText(tmp);

        caretPosition_++;
    }
}

bool TextField::handleKey(Key key, const int modKeys) {
    bool needRedraw = false;
    if (key.keyFunc == KFC_LEFT) {
        // Move caret to the left until start of the text
        if (caretPosition_ > 0) {
            caretPosition_--;
            needRedraw = true;
        }
    } else if (key.keyFunc == KFC_RIGHT) {
        char src[100];
        size_t size = text_.getText().size();

        memset(src, 0, 100);
        memcpy(src, text_.getText().c_str(), size);
        size_t dist = utf8::distance(src, src + size);
        // Move caret to the right until the end of the text
        if (caretPosition_ < dist) {
            caretPosition_++;
            needRedraw = true;
        }
    } else if (key.keyFunc == KFC_BACKSPACE) {
        handleBackSpace();
    } else if (key.keyFunc == KFC_DELETE) {
        handleDelete();
    } else if (key.keyFunc == KFC_HOME) {
        caretPosition_ = 0;
        needRedraw = true;
    } else if (key.keyFunc == KFC_END) {
        char src[100];
        size_t size = text_.getText().size();

        memset(src, 0, 100);
        memcpy(src, text_.getText().c_str(), size);
        caretPosition_ = utf8::distance(src, src + size);
        needRedraw = true;
    } else if (text_.getFont()->isPrintable(key.unicode)) {
        handleCharacter(key);
    } else {
        return false;
    }

    if (needRedraw)
        redraw();
    return true;
}

void TextField::handleMouseDown(int x, int y, int button, const int modKeys) {
    getPeer()->captureInputBy(this);

    char src[100];
    size_t sizeByte = text_.getText().size();

    memset(src, 0, 100);
    memcpy(src, text_.getText().c_str(), sizeByte);
    size_t nbCdpt = utf8::distance(src, src + sizeByte);

    // Size in pixel of the text
    int sizePxl = text_.getFont()->textWidth(src, false);

    // computes caret position
    if (x > sizePxl + getX()) {
        // Clicked after the text so caret is at the end
        caretPosition_ = nbCdpt;
    } else {
        // Clicked in the text so computes what exact letter
        char tmp[100];
        char *itSrc = src;
        char *itDst = tmp;
        memset(tmp, 0, 100);

        size_t i = 0;
        while (i<nbCdpt) {
            int cp = utf8::next(itSrc, src + sizeByte);
            itDst = utf8::append(cp, itDst);

            if (x < getX() + text_.getFont()->textWidth(tmp, false)) {
                caretPosition_ = i;
                break;
            }
            i++;
        }
    }

    redraw();
}

void TextField::setText(const char* text) {
    text_.setText(text);
    redraw();
}

void TextField::drawCaret() {
    char src[100];
    size_t size = text_.getText().size();
    memset(src, 0, 100);
    memcpy(src, text_.getText().c_str(), size);

    char tmp[100];
    char *itSrc = src;
    char *itDst = tmp;
    memset(tmp, 0, 100);

    size_t nbCdpt = utf8::distance(src, src + size);

    size_t i = 0;
    while (i<caretPosition_) {
        int cp = utf8::next(itSrc, src + size);
        itDst = utf8::append(cp, itDst);
        i++;
    }

    int x = getX() + text_.getFont()->textWidth(tmp, false) + 1;

    // width of caret is the same of the letter above
    int length = 10;
    if (caretPosition_ < nbCdpt) {
        memset(tmp, 0, 100);
        itDst = tmp;
        int cp = utf8::next(itSrc, src + size);
        itDst = utf8::append(cp, itDst);
        length = text_.getFont()->textWidth(tmp, false);
    }

    // Draw caret
    g_Screen.drawRect(x, yCaret_, length, 2, 252);
}
