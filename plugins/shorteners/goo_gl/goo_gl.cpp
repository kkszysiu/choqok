/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010 Andrey Esin <gmlastik@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or ( at your option ) version 3 or any later version
    accepted by the membership of KDE e.V. ( or its successor approved
    by the membership of KDE e.V. ), which shall act as a proxy
    defined in Section 14 of version 3 of the license.


    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    aqint64 with this program; if not, see http://www.gnu.org/licenses/
*/

#include "goo_gl.h"
#include <KDebug>
#include <kio/netaccess.h>
#include <KAboutData>
#include <KGenericFactory>
#include <kglobal.h>
#include <math.h>

#include <QHttp>
#include <QtCore/QCoreApplication>

K_PLUGIN_FACTORY( MyPluginFactory, registerPlugin < Goo_gl > (); )
K_EXPORT_PLUGIN( MyPluginFactory( "choqok_goo_gl" ) )

Goo_gl::Goo_gl( QObject* parent, const QVariantList& )
    : Choqok::Shortener( MyPluginFactory::componentData(), parent )
{
    connect(&httpClient, SIGNAL(done(bool)), this, SLOT(slotReadyRead()));
}

Goo_gl::~Goo_gl()
{
}

qint64 Goo_gl::first( QString str ){
    qint64 m = 5381;
    for ( int o = 0; o < str.length(); o++ ){
        QList<qint64> qb;
        qb.append( m << 5 );
        qb.append( m );
        qb.append( ( int )str.at( o ).toAscii() );
        m = third( qb );
    }
    return m;
}

qint64 Goo_gl::second( QString str ){
    qint64 m = 0;
    for ( int o = 0; o < str.length(); o++ ){
        QList<qint64> qb;
        qb.append( ( int )str.at( o ).toAscii() );
        qb.append( m << 6 );
        qb.append( m << 16 );
        qb.append( -m );
        m = third( qb );
    }
    return m;
}

qint64 Goo_gl::third( QList<qint64> &b ){
    qint64 l = 0;
    for ( int i = 0; i < b.length();i++ ){
        qint64 val = b.at( i );
        val &= 4294967295;
        val += val > 2147483647 ? -4294967296 : ( val < -2147483647 ? 4294967296 : 0 );
        l += val;
        l += l > 2147483647 ? -4294967296 : ( l < -2147483647 ? 4294967296 : 0 );
    }
  return l;
}

QString Goo_gl::fourth( qint64 l ){
    l = l > 0 ? l : l + 4294967296;
    QString m = QString::number( l );
    qint64 o = 0;
    bool n = false;
    for ( int p = m.length() - 1;p >= 0;--p ){
        int q = QString( m.at( p ) ).toInt( 0, 10 );
        if ( n ){
            q *= 2;
            o += floor( q / 10 ) + q % 10;
        } else {
            o += q;
        }
        n = !n;
    }
    m = QString::number( o % 10 );
    o = 0;
    if ( m != 0 ){
        o = 10 - m.toInt( 0, 10 );
        if ( QString::number( l ).length() % 2 == 1 ){
            if ( o % 2 == 1 ){
                o += 9;
            }
        o /= 2;
        }
    }
    return QString( "%1%2" ).arg( o ).arg( l );
}

QString Goo_gl::authToken( QString url ){
        qint64 i = first( url );
        i = i >> 2 & 1073741823;
        i = ( i >> 4 & 67108800 ) | ( i & 63 );
        i = ( i >> 4 & 4193280 ) | ( i & 1023 );
        i = ( i >> 4 & 245760 ) | ( i & 16383 );
        QString j = "7";
        qint64 h = second( url );
        qint64 k = ( i >> 2 & 15 ) << 4 | ( h & 15 );
        k |= ( i >> 6 & 15 ) << 12 | ( h >> 8 & 15 ) << 8;
        k |= ( i >> 10 & 15 ) << 20 | ( h >> 16 & 15 ) << 16;
        k |= ( i >> 14 & 15 ) << 28 | ( h >> 24 & 15 ) << 24;
        j.append( fourth( k ) );
        return j;
}

void Goo_gl::slotReadyRead()
{
    data = httpClient.readAll();
    readyToParse = true;
}

QString Goo_gl::shorten( const QString& url )
{
    kDebug() << "Using goo.gl";

    QString req;
    req = QString( "user=toolbar@google.com" ) + 
          QString( "&url=" ) + QUrl::toPercentEncoding( KUrl( url ).url() ) + 
          QString( "&auth_token=" ) + authToken( KUrl( url ).url() );
          
    readyToParse = false;
    QHttpRequestHeader header("POST", "/api/url");
    header.setValue("Host", "goo.gl");
    header.setValue("Accept","*/*");
    header.setValue("Content-Length", QString::number(req.length()));
    header.setValue("Content-Type","application/x-www-form-urlencoded");

    httpClient.setHost("goo.gl");
    httpClient.request(header,req.toUtf8());

    while (!readyToParse){
      qApp->processEvents(); //Wait while buffer will be full.
    }
    if (data.isEmpty()) {
      return url;
    }
    QString output( data );
    QRegExp rx( QString( "\\{\\\"short_url\\\":\\\"(.*)\\\"" ) );
    rx.setMinimal(true);
    rx.indexIn(output);
    output = rx.cap(1);
    kDebug() << "Short url is: " << output;    
    if( !output.isEmpty() ) {
        return output;
    }
    
    return url;
}

#include "goo_gl.moc"
