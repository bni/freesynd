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

#ifndef SEQMODEL_H
#define SEQMODEL_H

#include <string>
#include <list>
#include <vector>
#include <stdexcept>

/*!
 * A listener that can react to changes of the model.
 */
class ModelListener {
public:
    virtual ~ModelListener() {}
    /*!
     * Called by the model to alert on the changes.
     */
    virtual void handleModelChanged() = 0;
};

/*!
 * A SequenceModel represents an ordered collection of elements.
 * Clients can have the number of elements in the collection and
 * access to those elements.
 * A SequenceModel knows how to display itself using the getLabels()
 * method that return a list of strings representing its elements.
 * A SequenceModel can have listeners that will be called when the model
 * changes.
 */
class SequenceModel {
public:

    virtual ~SequenceModel();
    //! Fills the given list of string, one for each element
    virtual void getLabels(std::list<std::string> &labels) = 0;
    //! Returns a pointer to the element at given position
    virtual void * getElement(unsigned int i) = 0;
    //! Returns the line number for the given element
    virtual int getLineForElement(void *pElement) = 0;
    //! Returns the number of elements
    virtual unsigned int size() = 0;
    //! Removes all elements from the collection
    virtual void clear() = 0;

    //! Adds a new listeners to the model
    void addModelListener(ModelListener *pListener);
    //! Removes a listener from the list of listeners
    void removeModelListener(ModelListener *pListener);

protected:
    //! Calls all listeners to warn about a change in the model
    void fireModelChanged();

protected:
    /*! A list of listeners.*/
    std::list<ModelListener *> listeners_;
};

/*!
 * A implementation of SequenceModel using a vector.
 */
template<class T>
class VectorModel : public SequenceModel {
public:
    VectorModel() {}
    ~VectorModel(){
        elements_.clear();
    }

    /*!
     * Adds a element at the end of the vector.
     */
    void add(T item) {
        elements_.push_back(item);
        fireModelChanged();
    }

    /*!
     * Sets the element at the given position.
     * Replaces existing element.
     */
    void setAt(unsigned int index, T item) {
        elements_[index] = item;
        fireModelChanged();
    }

    /*!
     * Insert element at the given position.
     * Next elements are moved.
     */
    void insertAt(unsigned int index, T item) {
        unsigned int i=0;
        typename std::vector < T >::iterator it;
        for (it = elements_.begin();
         it != elements_.end(); it++, i++) {
             if (i == index) {
                 elements_.insert(it, item);
                 fireModelChanged();
                 return;
             }
        }

        throw std::range_error("Index");
    }

    /*!
     * Removes the element at the given position.
     */
    void remove(unsigned int index) {
        unsigned int i=0;
        typename std::vector < T >::iterator it;
        for (it = elements_.begin();
         it != elements_.end(); it++, i++) {
             if (i == index) {
                 elements_.erase(it);
                 fireModelChanged();
                 return;
             }
        }

        throw std::range_error("Index");
    }

    /*!
     * Returns the element at given position.
     */
    void * getElement(unsigned int i) {
        return get(i);
    }

    /*! 
     * Returns the line number for the given element or -1 if not found.
     */
    int getLineForElement(void *pElement) {
        int i=0;
        typename std::vector < T >::iterator it;
        for (it = elements_.begin();
         it != elements_.end(); it++, i++) {
             if (pElement == *it) {
                 return i;
             }
        }

        return -1;
    }

    /*!
     * Returns the element at given position.
     */
    T get(unsigned int index) {
        if (index < size()) {
            return elements_[index];
        }

        throw std::range_error("Erreur");
    }

    /*!
     * Returns number of elements in the vector.
     */
    unsigned int size() {
        return elements_.size();
    }

    /*!
     * Fills the given list with a string for each element of the collection.
     * Calls T::getName() to obtain the string.
     */
    void getLabels(std::list<std::string> &labels) {
        typename std::vector < T >::iterator it;
        for (it = elements_.begin();
         it != elements_.end(); it++) {
             if (*it) {
                labels.push_back((*it)->getName());
             } else {
                 labels.push_back("");
             }
        }
    }

    /*!
     * Removes all elements from the collection.
     */
    void clear() {
        elements_.clear();
        fireModelChanged();
    }

protected:
    /*! A vector of elements.*/
    std::vector<T> elements_;
};

#endif //SEQMODEL_H
