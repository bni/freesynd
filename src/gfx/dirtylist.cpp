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

#include "common.h"
#include "dirtylist.h"

DirtyList::DirtyList(int screenWidth, int screenHeight) {
    size_ = 0;
    pHead_ = NULL;
    screenWidth_ = screenWidth;
    screenHeight_ = screenHeight;
}

DirtyList::~DirtyList() {
    flush();
}

DirtyList::Link * DirtyList::createLink(int x, int y, int width, int height) {
    Link *l = new Link();
    l->pNext = NULL;
    l->pPrev = NULL;
    l->element.x = x;
    l->element.y = y;
    l->element.width = width;
    l->element.height = height;

    return l;
}

void DirtyList::addRect(int x, int y, int width, int height) {

    if (x >= screenWidth_ || y >= screenHeight_ ) {
        return;
    }

    if (pHead_ == NULL) {
        // Create new link
        pHead_ = createLink(x, y, width, height);
    } else {

        Link *pCurr = pHead_;
        do {
            if (pCurr->element.x <= x && pCurr->element.y <= y &&
                (pCurr->element.x + pCurr->element.width) >= (x + width) &&
                (pCurr->element.y + pCurr->element.height) >= (y + height)) {
                // Current rect is enclosing new rect so don't add new rect
                return;
            } else if (pCurr->element.x > x && pCurr->element.y > y &&
                (pCurr->element.x + pCurr->element.width) < (x + width) &&
                (pCurr->element.y + pCurr->element.height) < (y + height)) {
                // Current rect is enclosed by new rect so remove it from the list
                Link *pToRemove = pCurr;
                if (pToRemove->pNext != NULL && pToRemove->pPrev != NULL) {
                    // Current is between 2 links
                    pToRemove->pPrev->pNext = pToRemove->pNext;
                    pToRemove->pNext->pPrev = pToRemove->pPrev;
                    pCurr = pToRemove->pNext;
                } else if (pToRemove->pPrev != NULL && pToRemove->pNext == NULL) {
                    // We are on the last element of the list
                    pToRemove->pPrev->pNext = NULL;
                    pCurr = pToRemove->pPrev;
                } else if (pToRemove->pPrev == NULL && pToRemove->pNext != NULL) {
                    // There no previous so we're on the head
                    pHead_ = pCurr = pToRemove->pNext;
                    pHead_->pPrev = NULL;
                } else {
                    pCurr = NULL;
                }

                // Delete current
                pToRemove->pNext = NULL;
                pToRemove->pPrev = NULL;
                delete pToRemove;
                size_--;
            } else if (pCurr->pNext == NULL) {
                break;
            } else {
                pCurr = pCurr->pNext;
            }
            
        } while(pCurr != NULL);

        Link *l = createLink(x, y, width, height);
        if (pCurr == NULL) {
            pHead_ = l;
        } else {
            pCurr->pNext = l;
            l->pPrev = pCurr;
        }
    }
    size_++;
}

DirtyRect * DirtyList::getRectAt(int pos) {
    if (pHead_ && pos >= 0 && pos < size_) {
        int i = 0;
        Link * l = pHead_;
        while (i < pos) {
            l = l->pNext;
            i++;
        }
        return &(l->element);
    }
       
    return NULL;
}

void DirtyList::flush() {
    size_ = 0;
    if (pHead_) {
        Link *pCurr = pHead_;
        do {
            Link *pNext = pCurr->pNext;
            if (pNext) {
                pNext->pPrev = NULL;
                pCurr->pNext = NULL;
            }
            delete pCurr;
            pCurr = pNext;
        } while(pCurr != NULL);
    }
    pHead_ = NULL;
}

bool DirtyList::intersectsList(int x, int y, int width, int height)
{
    if (pHead_) {
        Link * l = pHead_;
        while (l) {
            if ( !((x > l->element.x + l->element.width) || 
                    (x + width < l->element.x) ||
                    (y > l->element.y + l->element.height) ||
                    (y + height < l->element.y)) ) {
                return true;
            }


            l = l->pNext;
        }
    }
       
    return false;
}
