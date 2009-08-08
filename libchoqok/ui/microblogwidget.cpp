/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include "microblogwidget.h"
#include "microblog.h"
#include "account.h"
#include <KDebug>
#include <QHBoxLayout>
#include <QApplication>
#include "timelinewidget.h"
#include <ktabwidget.h>
#include "composerwidget.h"
#include <QLayout>

namespace Choqok
{

class MicroBlogWidget::MBPrivate
{
public:
    MBPrivate(Account *acc)
    :account(acc), blog(acc->microblog()), composer(0)
    {}
    Account *account;
    MicroBlog *blog;
    ComposerWidget *composer;
};

MicroBlogWidget::MicroBlogWidget( Account *account, QWidget* parent, Qt::WindowFlags f)
    :QWidget(parent, f), d(new MBPrivate(account))
{
    kDebug();
    setupUi();
    connect(qApp, SIGNAL(aboutToQuit()), SLOT(aboutToQuit()));
    connect( this, SIGNAL(markAllAsRead()), SLOT(slotMarkAllAsRead()) );
    connect(d->blog, SIGNAL(timelineDataReceived(QString,QList<Choqok::Post*>)),
             this, SLOT(newTimelineDataRecieved(QString,QList<Choqok::Post*>)) );
    initTimelines();
}

Account * MicroBlogWidget::currentAccount() const
{
    return d->account;
}

void MicroBlogWidget::setupUi()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    if(d->composer)
        layout->addWidget(d->composer);
    timelinesTabWidget = new KTabWidget(this);
    layout->addWidget( timelinesTabWidget );
//     timelinesTabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->layout()->setContentsMargins( 0, 0, 0, 0 );
}

void MicroBlogWidget::setComposerWidget(ComposerWidget *widget)
{
    if(d->composer)
        d->composer->deleteLater();
    d->composer = widget;
    d->composer->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Maximum);
    qobject_cast<QVBoxLayout*>( this->layout() )->insertWidget(0, d->composer);
    foreach(TimelineWidget *mbw, timelines.values()) {
        connect(mbw, SIGNAL(forwardResendPost(QString)), d->composer, SLOT(setText(QString)));
        connect( mbw, SIGNAL(forwardReply(QString,QString)), d->composer, SLOT(setText(QString,QString)) );
    }
}

MicroBlogWidget::~MicroBlogWidget()
{
    delete d;
}

void MicroBlogWidget::settingsChanged()
{
    kDebug();
    foreach(TimelineWidget *wd, timelines.values()){
        wd->settingsChanged();
    }
}

void MicroBlogWidget::updateTimelines()
{
    if(!d->account)
        kError()<<"NIST AMU JAN";
    kDebug()<<d->account->alias();
    d->account->microblog()->updateTimelines();
}

void MicroBlogWidget::aboutToQuit()
{
}

void MicroBlogWidget::removeOldPosts()
{
    foreach(TimelineWidget *wd, timelines.values()) {
        wd->removeOldPosts();
    }
}

void MicroBlogWidget::newTimelineDataRecieved( const QString &type, QList<Post*> data )
{
    kDebug()<<d->account->alias()<<": "<<type;
    if(timelines.contains(type)){
        timelines.value(type)->addNewPosts(data);
    } else {
        if(TimelineWidget *wd = addTimelineWidgetToUi(type) )
            wd->addNewPosts(data);
    }
}

void MicroBlogWidget::initTimelines()
{
    kDebug();
    foreach( QString timeline, d->blog->timelineTypes() ){
        addTimelineWidgetToUi(timeline);
    }
}

TimelineWidget* MicroBlogWidget::addTimelineWidgetToUi(const QString& name)
{
    TimelineWidget *mbw = d->blog->createTimelineWidget(d->account, name, this);
    if(mbw) {
        mbw->setObjectName(name);
        timelines.insert(name, mbw);
        timelineUnreadCount.insert(mbw, 0);
        timelinesTabWidget->addTab(mbw, name);
        connect(this, SIGNAL(markAllAsRead()), mbw, SLOT(markAllAsRead()));
        connect( mbw, SIGNAL(updateUnreadCount(int)),
                    this, SLOT(slotUpdateUnreadCount(int)) );
        if(d->composer) {
            connect( mbw, SIGNAL(forwardResendPost(QString)),
                     d->composer, SLOT(setText(QString)) );
            connect( mbw, SIGNAL(forwardReply(QString,QString)),
                     d->composer, SLOT(setText(QString,QString)) );
        }
        return mbw;
    } else {
        kError()<<"Cannot Create a new TimelineWidget for timeline "<<name;
        return 0L;
    }
    if(timelinesTabWidget->count() == 1)
        timelinesTabWidget->setTabBarHidden(true);
    else
        timelinesTabWidget->setTabBarHidden(false);
}

void MicroBlogWidget::slotUpdateUnreadCount(int count)
{
    kDebug()<<count;
    int sum = count;
    foreach(int n, timelineUnreadCount.values())
        sum += n;
    emit updateUnreadCount(sum);

    if(sender())
        kDebug()<<sender()->objectName();
    TimelineWidget *wd = qobject_cast<TimelineWidget*>(sender());
    if(wd) {
        int cn = timelineUnreadCount[wd] = timelineUnreadCount[wd] + count;
        int tabIndex = timelinesTabWidget->indexOf(wd);
        if(tabIndex == -1)
            return;
        if(cn > 0)
            timelinesTabWidget->setTabText( tabIndex, wd->timelineName() + QString("(%1)").arg(timelineUnreadCount[wd]) );
        else
            timelinesTabWidget->setTabText( tabIndex, wd->timelineName() );
    }
}

void MicroBlogWidget::slotMarkAllAsRead()
{
    foreach(TimelineWidget *wd, timelines.values()) {
        int tabIndex = timelinesTabWidget->indexOf(wd);
        if(tabIndex == -1)
            continue;
        timelinesTabWidget->setTabText( tabIndex, wd->timelineName() );
    }
}

}
