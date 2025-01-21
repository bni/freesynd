/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2010, 2025  Benoit Blancard <benblan@users.sourceforge.net>
 *
 *   This program is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as 
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>. 
 * 
 */

#ifndef DIRTYLIST_H
#define DIRTYLIST_H

struct DirtyRect {
    int x, y;
    int width, height;
};

class DirtyList {
public:
    DirtyList(int screenWidth, int screenHeight);

    ~DirtyList();

    bool isEmpty() { return size_ == 0; }

    int getSize() { return size_; }

    void addRect(int x, int y, int width, int height);

    DirtyRect * getRectAt(int pos);

    void flush();

    //! Returns true if the given rect intersects with any dirty rect in the list
    bool intersectsList(int x, int y, int width, int height);

private:
    struct Link
    {
        Link *pPrev;
        Link *pNext;
        DirtyRect element;
    };

    Link * createLink(int x, int y, int width, int height);

    int size_;
    int screenWidth_;
    int screenHeight_;
    Link    *pHead_;
};
#endif // DIRTYLIST_H
