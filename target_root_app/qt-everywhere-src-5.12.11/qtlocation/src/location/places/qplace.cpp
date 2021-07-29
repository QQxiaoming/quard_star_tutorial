/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtLocation module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qplace.h"
#include "qplace_p.h"

#ifdef QPLACE_DEBUG
#include <QDebug>
#endif

#include <QStringList>

QT_BEGIN_NAMESPACE

template<>
QPlacePrivate *QSharedDataPointer<QPlacePrivate>::clone()
{
    return d->clone();
}

/*!
    \class QPlace
    \inmodule QtLocation
    \ingroup QtLocation-places
    \ingroup QtLocation-places-data
    \since 5.6

    \brief The QPlace class represents a set of data about a place.

    \input place-definition.qdocinc

    \section2 Contact Information
    The contact information of a place is based around a common set of
    \l {Contact Types}{contact types}. To retrieve all the phone numbers
    of a place, one would do:

    \snippet places/requesthandler.h Phone numbers

    The contact types are string values by design to allow for providers
    to introduce new contact types.

    For convenience there are a set of functions which return the value
    of the first contact detail of each type.
    \list
        \li QPlace::primaryPhone()
        \li QPlace::primaryEmail()
        \li QPlace::primaryWebsite()
        \li QPlace::primaryFax()
    \endlist

    \section2 Extended Attributes
    Places may have additional attributes which are not covered in the formal API.
    Similar to contacts attributes are based around a common set of
    \l {Attribute Types}{attribute types}.  To retrieve an extended attribute one
    would do:
    \snippet places/requesthandler.h Opening hours

    The attribute types are string values by design to allow providers
    to introduce new attribute types.

    \section2 Content
    The QPlace object is only meant to be a convenient container to hold
    rich content such as images, reviews and so on.  Retrieval of content
    should happen via QPlaceManager::getPlaceContent().

    The content is stored as a QPlaceContent::Collection which contains
    both the index of the content, as well as the content itself.  This enables
    developers to check whether a particular item has already been retrieved
    and if not, then request that content.

    \section3 Attribution
    Places have a field for a rich text attribution string.  Some providers
    may require that the attribution be shown when a place is displayed
    to a user.

    \section2 Categories
    Different categories may be assigned to a place to indicate that the place
    is associated with those categories.  When saving a place, the only meaningful
    data is the category id, the rest of the category data is effectively ignored.
    The category must already exist before saving the place (it is not possible
    to create a new category, assign it to the place, save the place and expect
    the category to be created).

    \section2 Saving Caveats
    \input place-caveats.qdocinc
*/

/*!
    Constructs an empty place object.
*/
QPlace::QPlace()
        : d_ptr(new QPlacePrivateDefault())
{
}

/*!
    Constructs an place object using \a dd as private implementation.
*/
QPlace::QPlace(const QSharedDataPointer<QPlacePrivate> &dd): d_ptr(dd)
{
}

/*!
    Returns the d-pointer.
*/
QSharedDataPointer<QPlacePrivate> &QPlace::d()
{
    return d_ptr;
}

/*!
    Constructs a copy of \a other.
*/
QPlace::QPlace(const QPlace &other)
        : d_ptr(other.d_ptr)
{
}

/*!
    Destroys this place.
*/
QPlace::~QPlace()
{
}

/*!
    Assigns \a other to this place and returns a reference
    to this place.
*/
QPlace &QPlace::operator= (const QPlace & other)
{
    if (this == &other)
        return *this;

    d_ptr = other.d_ptr;
    return *this;
}

inline QPlacePrivate *QPlace::d_func()
{
    return static_cast<QPlacePrivate *>(d_ptr.data());
}

inline const QPlacePrivate *QPlace::d_func() const
{
    return static_cast<const QPlacePrivate *>(d_ptr.constData());
}

/*!
    Returns true if \a other is equal to this place,
    otherwise returns false.
*/
bool QPlace::operator== (const QPlace &other) const
{
    return ( (d_ptr.constData() == other.d_ptr.constData())
             || (*d_ptr) == (*other.d_ptr));
}

/*!
    Returns true if \a other is not equal to this place,
    otherwise returns false.
*/
bool QPlace::operator!= (const QPlace &other) const
{
    return !(operator==(other));
}

/*!
    Returns categories that this place belongs to.
*/
QList<QPlaceCategory> QPlace::categories() const
{
    return d_ptr->categories();
}

/*!
    Sets a single \a category that this place belongs to.
*/
void QPlace::setCategory(const QPlaceCategory &category)
{
    d_ptr->setCategories(QList<QPlaceCategory>());
    d_ptr->setCategories(QList<QPlaceCategory>() << category);
}

/*!
    Sets the \a categories that this place belongs to.
*/
void QPlace::setCategories(const QList<QPlaceCategory> &categories)
{
     d_ptr->setCategories(categories);
}

/*!
    Returns the location of the place.
*/
QGeoLocation QPlace::location() const
{
    return d_ptr->location();
}

/*!
    Sets the \a location of the place.
*/
void QPlace::setLocation(const QGeoLocation &location)
{
    d_ptr->setLocation(location);
}

/*!
    Returns an aggregated rating of the place.
*/
QPlaceRatings QPlace::ratings() const
{
    return d_ptr->ratings();
}

/*!
    Sets the aggregated \a rating of the place.
*/
void QPlace::setRatings(const QPlaceRatings &rating)
{
    d_ptr->setRatings(rating);
}

/*!
    Returns the supplier of this place.
*/
QPlaceSupplier QPlace::supplier() const
{
    return d_ptr->supplier();
}

/*!
    Sets the supplier of this place to \a supplier.
*/
void QPlace::setSupplier(const QPlaceSupplier &supplier)
{
    d_ptr->setSupplier(supplier);
}

/*!
    Returns a collection of content associated with a place.
    This collection is a map with the key being the index of the content object
    and value being the content object itself.

    The \a type specifies which kind of content is to be retrieved.
*/
QPlaceContent::Collection QPlace::content(QPlaceContent::Type type) const
{
    return d_ptr->m_contentCollections.value(type);
}

/*!
    Sets a collection of \a content for the given \a type.
*/
void QPlace::setContent(QPlaceContent::Type type, const QPlaceContent::Collection &content)
{
    d_ptr->m_contentCollections.insert(type, content);
}

/*!
    Adds a collection of \a content of the given \a type to the place.  Any index in \a content
    that already exists is overwritten.
*/
void QPlace::insertContent(QPlaceContent::Type type, const QPlaceContent::Collection &content)
{
    QMapIterator<int, QPlaceContent> iter(content);
    while (iter.hasNext()) {
        iter.next();
        d_ptr->m_contentCollections[type].insert(iter.key(), iter.value());
    }
}

/*!
    Returns the total count of content objects of the given \a type.
    This total count indicates how many the manager/provider should have available.
    (As opposed to how many objects this place instance is currently assigned).

    A negative count indicates that the total number of items is unknown.
    By default the total content count is set to 0.
*/
int QPlace::totalContentCount(QPlaceContent::Type type) const
{
    return d_ptr->m_contentCounts.value(type, 0);
}

/*!
    Sets the \a totalCount of content objects of the given \a type.
*/
void QPlace::setTotalContentCount(QPlaceContent::Type type, int totalCount)
{
    d_ptr->m_contentCounts.insert(type, totalCount);
}

/*!
    Returns the name of the place.
*/
QString QPlace::name() const
{
    return d_ptr->name();
}

/*!
    Sets the \a name of the place.
*/
void QPlace::setName(const QString &name)
{
    d_ptr->setName(name);
}

/*!
    Returns the identifier of the place.  The place identifier is only meaningful to the QPlaceManager that
    generated it and is not transferable between managers.  The place identifier is not guaranteed
    to be universally unique, but unique for the manager that generated it.
*/
QString QPlace::placeId() const
{
    return d_ptr->placeId();
}

/*!
    Sets the \a identifier of the place.
*/
void QPlace::setPlaceId(const QString &identifier)
{
    d_ptr->setPlaceId(identifier);
}

/*!
    Returns a rich text attribution string of the place.  Note, some providers may have a
    requirement where the attribution must be shown whenever a place is displayed to an end user.
*/
QString QPlace::attribution() const
{
    return d_ptr->attribution();
}

/*!
    Sets the \a attribution string of the place.
*/
void QPlace::setAttribution(const QString &attribution)
{
    d_ptr->setAttribution(attribution);
}

/*!
    Returns the icon of the place.
*/
QPlaceIcon QPlace::icon() const
{
    return d_ptr->icon();
}

/*!
    Sets the \a icon of the place.
*/
void QPlace::setIcon(const QPlaceIcon &icon)
{
    d_ptr->setIcon(icon);
}

/*!
    Returns the primary phone number for this place.  This accesses the first contact detail
    of the \l {QPlaceContactDetail::Phone}{phone number type}.  If no phone details exist, then an empty string is returned.
*/
QString QPlace::primaryPhone() const
{
    QList<QPlaceContactDetail> phoneNumbers = d_ptr->contacts().value(QPlaceContactDetail::Phone);
    if (!phoneNumbers.isEmpty())
        return phoneNumbers.at(0).value();
    else
        return QString();
}

/*!
    Returns the primary fax number for this place.  This convenience function accesses the first contact
    detail of the \l {QPlaceContactDetail::Fax}{fax type}.  If no fax details exist, then an empty string is returned.
*/
QString QPlace::primaryFax() const
{
    QList<QPlaceContactDetail> faxNumbers = d_ptr->contacts().value(QPlaceContactDetail::Fax);
    if (!faxNumbers.isEmpty())
        return faxNumbers.at(0).value();
    else
        return QString();
}

/*!
    Returns the primary email address for this place.  This convenience function accesses the first
    contact detail of the \l {QPlaceContactDetail::Email}{email type}.  If no email addresses exist, then
    an empty string is returned.
*/
QString QPlace::primaryEmail() const
{
    QList<QPlaceContactDetail> emailAddresses = d_ptr->contacts().value(QPlaceContactDetail::Email);
    if (!emailAddresses.isEmpty())
        return emailAddresses.at(0).value();
    else
        return QString();
}

/*!
    Returns the primary website of the place.  This convenience function accesses the first
    contact detail of the \l {QPlaceContactDetail::Website}{website type}.  If no websites exist,
    then an empty string is returned.
*/
QUrl QPlace::primaryWebsite() const
{
    QList<QPlaceContactDetail> websites = d_ptr->contacts().value(QPlaceContactDetail::Website);
    if (!websites.isEmpty())
        return QUrl(websites.at(0).value());
    else
        return QString();
}

/*!
    Returns true if the details of this place have been fetched,
    otherwise returns false.
*/
bool QPlace::detailsFetched() const
{
    return d_ptr->detailsFetched();
}

/*!
    Sets whether the details of this place have been \a fetched or not.
*/
void QPlace::setDetailsFetched(bool fetched)
{
    d_ptr->setDetailsFetched(fetched);
}

/*!
    Returns the types of extended attributes that this place has.
*/
QStringList QPlace::extendedAttributeTypes() const
{
    return d_ptr->extendedAttributes().keys();
}

/*!
    Returns the exteded attribute corresponding to the specified \a attributeType.
    If the place does not have that particular attribute type, a default constructed
    QPlaceExtendedAttribute is returned.
*/
QPlaceAttribute QPlace::extendedAttribute(const QString &attributeType) const
{
    return d_ptr->extendedAttribute(attributeType);
}

/*!
    Assigns an \a attribute of the given \a attributeType to a place.  If the given \a attributeType
    already exists in the place, then it is overwritten.

    If \a attribute is a default constructed QPlaceAttribute, then the \a attributeType
    is removed from the place which means it will not be listed by QPlace::extendedAttributeTypes().
*/
void QPlace::setExtendedAttribute(const QString &attributeType,
                                    const QPlaceAttribute &attribute)
{
    if (attribute == QPlaceAttribute())
        d_ptr->extendedAttributes().remove(attributeType);
    else
        d_ptr->extendedAttributes().insert(attributeType, attribute);
}

/*!
    Remove the attribute of \a attributeType from the place.

    The attribute will no longer be listed by QPlace::extendedAttributeTypes()
*/
void QPlace::removeExtendedAttribute(const QString &attributeType)
{
    setExtendedAttribute(attributeType, QPlaceAttribute());
}

/*!
    Returns the type of contact details this place has.

    See QPlaceContactDetail for a list of common \l {QPlaceContactDetail::Email}{contact types}.
*/
QStringList QPlace::contactTypes() const
{
    return d_ptr->contacts().keys();
}

/*!
    Returns a list of contact details of the specified \a contactType.

    See QPlaceContactDetail for a list of common \l {QPlaceContactDetail::Email}{contact types}.
*/
QList<QPlaceContactDetail> QPlace::contactDetails(const QString &contactType) const
{
    return d_ptr->contacts().value(contactType);
}

/*!
    Sets the contact \a details of a specified \a contactType.

    If \a details is empty, then the \a contactType is removed from the place such
    that it is no longer returned by QPlace::contactTypes().

    See QPlaceContactDetail for a list of common \l {QPlaceContactDetail::Email}{contact types}.
*/
void QPlace::setContactDetails(const QString &contactType, QList<QPlaceContactDetail> details)
{
    if (details.isEmpty())
        d_ptr->contacts().remove(contactType);
    else
        d_ptr->contacts().insert(contactType, details);
}

/*!
    Appends a contact \a detail of a specified \a contactType.

    See QPlaceContactDetail for a list of common \l {QPlaceContactDetail::Email}{contact types}.
*/
void QPlace::appendContactDetail(const QString &contactType, const QPlaceContactDetail &detail)
{
    QList<QPlaceContactDetail> details = d_ptr->contacts().value(contactType);
    details.append(detail);
    d_ptr->contacts().insert(contactType, details);
}

/*!
    Removes all the contact details of a given \a contactType.

    The \a contactType is no longer returned when QPlace::contactTypes() is called.
*/
void QPlace::removeContactDetails(const QString &contactType)
{
    d_ptr->contacts().remove(contactType);
}

/*!
    Sets the visibility of the place to \a visibility.
*/
void QPlace::setVisibility(QLocation::Visibility visibility)
{
    d_ptr->setVisibility(visibility);
}

/*!
    Returns the visibility of the place.

    The default visibility of a new place is set to QtLocatin::Unspecified visibility.
    If a place is saved with unspecified visibility the backend chooses an appropriate
    default visibility to use when saving.
*/
QLocation::Visibility QPlace::visibility() const
{
    return d_ptr->visibility();
}

/*!
    Returns a boolean indicating whether the all the fields of the place are empty or not.
*/
bool QPlace::isEmpty() const
{
    return d_ptr->isEmpty();
}

/*******************************************************************************
*******************************************************************************/

QPlacePrivate::QPlacePrivate()
:   QSharedData()
{
}

QPlacePrivate::QPlacePrivate(const QPlacePrivate &other)
        : QSharedData(other),
        m_contentCollections(other.m_contentCollections),
        m_contentCounts(other.m_contentCounts)
{
}

QPlacePrivate::~QPlacePrivate() {}

bool QPlacePrivate::operator== (const QPlacePrivate &other) const
{
    return (categories() == other.categories()
            && location() == other.location()
            && ratings() == other.ratings()
            && supplier() == other.supplier()
            && m_contentCollections == other.m_contentCollections
            && m_contentCounts == other.m_contentCounts
            && name() == other.name()
            && placeId() == other.placeId()
            && attribution() == other.attribution()
            && contacts() == other.contacts()
            && extendedAttributes() == other.extendedAttributes()
            && visibility() == other.visibility()
            && icon() == other.icon()
            );
}


bool QPlacePrivate::isEmpty() const
{
    return (categories().isEmpty()
            && location().isEmpty()
            && ratings().isEmpty()
            && supplier().isEmpty()
            && m_contentCollections.isEmpty()
            && m_contentCounts.isEmpty()
            && name().isEmpty()
            && placeId().isEmpty()
            && attribution().isEmpty()
            && contacts().isEmpty()
            && extendedAttributes().isEmpty()
            && QLocation::UnspecifiedVisibility == visibility()
            && icon().isEmpty()
            );
}

QPlaceAttribute QPlacePrivate::extendedAttribute(const QString &attributeType) const
{
    return extendedAttributes().value(attributeType);
}



//
//  Default implementation
//

QPlacePrivateDefault::QPlacePrivateDefault()
    : QPlacePrivate(), m_visibility(QLocation::UnspecifiedVisibility), m_detailsFetched(false)
{
}

QPlacePrivateDefault::QPlacePrivateDefault(const QPlacePrivateDefault &other)
    : QPlacePrivate(other),
    m_categories(other.m_categories),
    m_location(other.m_location),
    m_ratings(other.m_ratings),
    m_supplier(other.m_supplier),
    m_name(other.m_name),
    m_placeId(other.m_placeId),
    m_attribution(other.m_attribution),
    m_extendedAttributes(other.m_extendedAttributes),
    m_contacts(other.m_contacts),
    m_visibility(other.m_visibility),
    m_icon(other.m_icon),
    m_detailsFetched(other.m_detailsFetched)
{
}

QPlacePrivateDefault::~QPlacePrivateDefault()
{
}

QPlacePrivate *QPlacePrivateDefault::clone()
{
    return new QPlacePrivateDefault(*this);
}

QList<QPlaceCategory> QPlacePrivateDefault::categories() const
{
    return m_categories;
}

void QPlacePrivateDefault::setCategories(const QList<QPlaceCategory> &categories)
{
    m_categories = categories;
}

QGeoLocation QPlacePrivateDefault::location() const
{
    return m_location;
}

void QPlacePrivateDefault::setLocation(const QGeoLocation &location)
{
    m_location = location;
}

QPlaceRatings QPlacePrivateDefault::ratings() const
{
    return m_ratings;
}

void QPlacePrivateDefault::setRatings(const QPlaceRatings &ratings)
{
    m_ratings = ratings;
}

QPlaceSupplier QPlacePrivateDefault::supplier() const
{
    return m_supplier;
}

void QPlacePrivateDefault::setSupplier(const QPlaceSupplier &supplier)
{
    m_supplier = supplier;
}

QString QPlacePrivateDefault::name() const
{
    return m_name;
}

void QPlacePrivateDefault::setName(const QString &name)
{
    m_name = name;
}

QString QPlacePrivateDefault::placeId() const
{
    return m_placeId;
}

void QPlacePrivateDefault::setPlaceId(const QString &placeIdentifier)
{
    m_placeId = placeIdentifier;
}

QString QPlacePrivateDefault::attribution() const
{
    return m_attribution;
}

void QPlacePrivateDefault::setAttribution(const QString &attribution)
{
    m_attribution = attribution;
}

QLocation::Visibility QPlacePrivateDefault::visibility() const
{
    return m_visibility;
}

void QPlacePrivateDefault::setVisibility(QLocation::Visibility visibility)
{
    m_visibility = visibility;
}

QPlaceIcon QPlacePrivateDefault::icon() const
{
    return m_icon;
}

void QPlacePrivateDefault::setIcon(const QPlaceIcon &icon)
{
    m_icon = icon;
}

bool QPlacePrivateDefault::detailsFetched() const
{
    return m_detailsFetched;
}

void QPlacePrivateDefault::setDetailsFetched(bool fetched)
{
    m_detailsFetched = fetched;
}

QMap<QString, QPlaceAttribute> QPlacePrivateDefault::extendedAttributes() const
{
    return m_extendedAttributes;
}

QMap<QString, QPlaceAttribute> &QPlacePrivateDefault::extendedAttributes()
{
    return m_extendedAttributes;
}

QMap<QString, QList<QPlaceContactDetail> > QPlacePrivateDefault::contacts() const
{
    return m_contacts;
}

QMap<QString, QList<QPlaceContactDetail> > &QPlacePrivateDefault::contacts()
{
    return m_contacts;
}



QT_END_NAMESPACE
