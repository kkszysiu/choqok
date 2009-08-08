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

#include "twitterpostwidget.h"
#include <microblog.h>
#include <klocalizedstring.h>
#include "twitteraccount.h"
#include <kicon.h>
#include <KPushButton>
#include "twittermicroblog.h"
#include <KDebug>
#include <mediamanager.h>
#include <KProcess>
#include <KToolInvocation>
#include <KSharedConfig>

const QRegExp TwitterPostWidget::mUserRegExp("([\\s]|^)@([^\\s\\W]+)");
const QRegExp TwitterPostWidget::mHashtagRegExp("([\\s]|^)#([^\\s\\W]+)");
const KIcon TwitterPostWidget::unFavIcon( Choqok::MediaManager::convertToGrayScale(KIcon("rating").pixmap(16)) );

TwitterPostWidget::TwitterPostWidget(Choqok::Account* account, const Choqok::Post &post, QWidget* parent)
    : PostWidget(account, post, parent)
{
    config = new KConfigGroup(KGlobal::config(), QString::fromLatin1("Behavior"));
    setOpenLinks(false);
    connect(this,SIGNAL(anchorClicked(QUrl)),this,SLOT(checkAnchor(QUrl)));
}

TwitterPostWidget::~TwitterPostWidget()
{
}

void TwitterPostWidget::initUi()
{
    Choqok::PostWidget::initUi();
    if( !mCurrentPost.isPrivate ) {
        btnFav = addButton( "btnFavorite",i18nc( "@info:tooltip", "Favorite" ), "rating" );
        btnFav->setCheckable(true);
        connect( btnFav, SIGNAL(clicked(bool)), SLOT(setFavorite()) );
        updateFavStat();
    }
    if( mCurrentAccount->username() != mCurrentPost.author.userName) {
        KPushButton *btnRe = addButton( "btnReply",i18nc( "@info:tooltip", "Reply" ), "edit-undo" );
        connect( btnRe, SIGNAL(clicked(bool)), SLOT(slotReply()) );
    }
}

QString TwitterPostWidget::prepareStatus(const QString& text)
{
    QString res = Choqok::PostWidget::prepareStatus(text);
    res.replace(mUserRegExp,"\\1@<a href='user://\\2'>\\2</a> <a href='"+
    mCurrentAccount->microblog()->profileUrl("\\2") + "' title='" +
    mCurrentAccount->microblog()->profileUrl("\\2") + "'>&#9794;</a>");
    //<img src=\"icon://web\" />
    res.replace(mHashtagRegExp,"\\1#<a href='tag://\\2'>\\2</a>");

    return res;
}

QString TwitterPostWidget::generateSign()
{
    QString sign;
    sign = "<b><a href='user://"+mCurrentPost.author.userName+"' title=\"" +
    mCurrentPost.author.description + "\">" + mCurrentPost.author.userName +
    "</a> <a href=\"" + mCurrentAccount->microblog()->profileUrl(mCurrentPost.author.userName) + "\" title=\"" +
    mCurrentAccount->microblog()->profileUrl(mCurrentPost.author.userName) + "\">&#9794;</a> - </b>";
    //<img src=\"icon://web\" />
    sign += "<a href=\"" + mCurrentPost.link +
    "\" title=\"" + mCurrentPost.creationDateTime.toString() + "\">%1</a>";
    if ( mCurrentPost.isPrivate ) {
        if( mCurrentPost.replyToUserId == qobject_cast<TwitterAccount*>(mCurrentAccount)->userId() ) {
            sign.prepend( "From " );
        } else {
            sign.prepend( "To " );
        }
    } else {
        if( !mCurrentPost.source.isNull() )
            sign += " - " + mCurrentPost.source;
        if ( !mCurrentPost.replyToPostId.isEmpty() ) {
            QString link = mCurrentAccount->microblog()->postUrl( mCurrentPost.replyToUserName, mCurrentPost.replyToUserId );
            sign += " - <a href='status://" + mCurrentPost.replyToPostId + "'>" +
            i18n("in reply to")+ "</a>&nbsp;<a href=\"" +
            mCurrentAccount->microblog()->postUrl(mCurrentPost.author.userName,
                                                   mCurrentPost.replyToPostId) +  "\">&#9794;</a>";
        }
    }
    sign.prepend("<p dir='ltr'>");
    sign.append( "</p>" );
    return sign;
}

void TwitterPostWidget::slotReply()
{
    emit reply( QString("@%1 ").arg(mCurrentPost.author.userName), mCurrentPost.postId );
}

void TwitterPostWidget::setFavorite()
{
    TwitterMicroBlog *mic = qobject_cast<TwitterMicroBlog*>(mCurrentAccount->microblog());
    if(mCurrentPost.isFavorited){
        connect(mic, SIGNAL(favoriteRemoved(QString)), SLOT(slotSetFavorite(QString)));
        mic->removeFavorite(mCurrentPost.postId);
    } else {
        connect(mic, SIGNAL(favoriteCreated(QString)), SLOT(slotSetFavorite(QString)));
        mic->createFavorite(mCurrentPost.postId);
    }
}

void TwitterPostWidget::slotSetFavorite(const QString& postId)
{
    if(postId == mCurrentPost.postId){
        kDebug()<<postId;
        mCurrentPost.isFavorited = !mCurrentPost.isFavorited;
        updateFavStat();
        TwitterMicroBlog *mic = qobject_cast<TwitterMicroBlog*>(mCurrentAccount->microblog());
        disconnect(mic, SIGNAL(favoriteRemoved(QString)), this, SLOT(slotSetFavorite(QString)));
        disconnect(mic, SIGNAL(favoriteCreated(QString)), this, SLOT(slotSetFavorite(QString)));
        ///TODO Notify!
    }
}

void TwitterPostWidget::updateFavStat()
{
    if(mCurrentPost.isFavorited)
        btnFav->setIcon(KIcon("rating"));
    else
        btnFav->setIcon(unFavIcon);
}

void TwitterPostWidget::checkAnchor(const QUrl & url)
{
    QString scheme = url.scheme();
    int type = 0;
    if(scheme == "tag") {

    } else if(scheme == "user") {

    } else if( scheme == "status" ) {

    } else if (scheme == "thread") {

    } else {
        if( config->readEntry("useCustomBrowser", false) ) {
            QStringList args = config->readEntry("customBrowser", "konqueror").split(' ');
            args.append(url.toString());
            if( KProcess::startDetached( args ) == 0 ) {
                KToolInvocation::invokeBrowser(url.toString());
            }
        } else {
            KToolInvocation::invokeBrowser(url.toString());
        }
        return;
    }
//     emit sigSearch(type,url.host());
}

#include "twitterpostwidget.moc"
