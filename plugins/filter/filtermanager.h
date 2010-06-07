/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.


    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see http://www.gnu.org/licenses/

*/
#ifndef FILTERMANAGER_H
#define FILTERMANAGER_H

#include <plugin.h>
#include <QPixmap>
#include <QQueue>
#include <QPointer>
#include "filter.h"

namespace Choqok {
namespace UI {
class PostWidget;
}
class Account;
}

class KConfigGroup;
/**
Filter Manager

@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class FilterManager : public Choqok::Plugin
{
    Q_OBJECT
public:
    enum FilterAction { None = 0, Remove};

    FilterManager( QObject* parent, const QList< QVariant >& args );
    ~FilterManager();

protected slots:
    void slotAddNewPostWidget( Choqok::UI::PostWidget* newWidget );
    void startParsing();
    void slotConfigureFilters();

private:
    enum ParserState{ Stopped = 0, Running };
    ParserState state;

    FilterAction filterText(const QString &textToCheck, Filter * filter);
    void doFiltering(Choqok::UI::PostWidget *postToFilter, FilterAction action );

    void parse( Choqok::UI::PostWidget *postToParse );
    QQueue< QPointer<Choqok::UI::PostWidget> > postsQueue;
};

#endif