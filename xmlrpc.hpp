/****************************************************************************
 **
 ** Copyright (C) Ryan McDougall, Qxt Foundation. Some rights reserved.
 **
 ** This library is free software; you can redistribute it and/or modify it
 ** under the terms of the Common Public License, version 1.0, as published
 ** by IBM, and/or under the terms of the GNU Lesser General Public License,
 ** version 2.1, as published by the Free Software Foundation.
 **
 ** This file is provided "AS IS", without WARRANTIES OR CONDITIONS OF ANY
 ** KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 ** WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY OR
 ** FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** You should have received a copy of the CPL and the LGPL along with this
 ** file. See the LICENSE file and the cpl1.0.txt/lgpl-2.1.txt files
 ** included with the source distribution for more information.
 ** If you did not receive a copy of the licenses, contact the Qxt Foundation.
 **
 ****************************************************************************/

#ifndef XMLRPC_H_
#define XMLRPC_H_

#include <QObject>
#include <QUrl>
#include <QVariant>
#include <QNetworkAccessManager>

class QXmlStreamReader;

namespace Scaffold
{
    namespace Connectivity
    {
        namespace XmlRpc
        {
            QString serialize (QVariant data);
            QString entity_encode (QString str);
            QVariant parse_result (QNetworkReply *reply);
            QVariant deserialize (QXmlStreamReader &xml);
            QVariant deserialize_array (QXmlStreamReader &xml);
            QVariant deserialize_struct (QXmlStreamReader &xml);

            //=========================================================================
            // Parse XMLRPC call automatically on finish

            class Call : public QObject
            {
                Q_OBJECT

                public:
                    Call (QNetworkReply *r);
                    ~Call ();

                    QNetworkReply   *reply;
                    QVariant        result;

                public slots:
                    void on_result ();
            };

            //=========================================================================
            // Format XMLRPC call and deliver of HTTP

            class Client : public QObject
            {
                public:
                    Client (const QUrl &service, QNetworkAccessManager *http = 0);

                    Call *call (QString method, QVariantList arguments);

                private:
                    QUrl                    service_;
                    QNetworkAccessManager   *http_;
            };
        }
    }
}

#endif
