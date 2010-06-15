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

#include "xmlrpc.hpp"

#include <QDateTime>
#include <QStringList>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QXmlStreamReader>

#include <iostream>

namespace Scaffold
{
    namespace Connectivity
    {
        namespace XmlRpc
        {
            //=========================================================================
            //

            Call::Call (QNetworkReply *r) : 
                reply (r)
            {
                // INFO: Call::on_result is always called before other 
                // listeners if it's always connected before 
                connect (reply, SIGNAL (finished()), this, SLOT (on_result()));
            }

            Call::~Call ()
            {
                reply->deleteLater();
            }

            void Call::on_result ()
            {
                result = parse_result (reply);
            }

            //=========================================================================
            //

            Client::Client (const QUrl &service, QNetworkAccessManager *http) : 
                service_ (service), http_ ((http)? http : new QNetworkAccessManager (this))
            {}

            Call *Client::call (QString method, QVariantList arguments)
            {
                QByteArray data = 
                    "<?xml version=\"1.0\" encoding=\"UTF-8\"?><methodCall><methodName>" + 
                    method.toUtf8() + "</methodName><params>";

                foreach(QVariant i, arguments)
                    data += "<param>" + serialize(i).toUtf8() + "</param>";
                data += "</params></methodCall>";

                QNetworkRequest request (service_);
                request.setHeader (QNetworkRequest::ContentTypeHeader, "text/xml");
                request.setRawHeader ("Connection", "close");

                return new Call (http_->post (request, data));
            }

            //=========================================================================
            // Serialize QVariant to XMLRPC markup

            QString serialize (QVariant data)
            {
                if (data.isNull())
                    return "<nil/>";

                int t = data.type();

                if (t == QVariant::String)
                    return "<string>" + entity_encode(data.toString()) + "</string>";

                else if (t == QVariant::Bool)
                    return "<boolean>" + (data.toBool() ? QString("1") : QString("0")) + 
                        "</boolean>";

                else if (t ==  QVariant::Int)
                    return "<int>" + QString::number(data.toInt()) + "</int>";

                else if (t == QVariant::Double)
                    return "<double>" + QString::number(data.toDouble()) + "</double>";

                else if (t == QVariant::DateTime)
                    return "<dateTime.iso8601>" + data.toDateTime().toString(Qt::ISODate) + 
                        "</dateTime.iso8601>";

                else if (t == QVariant::ByteArray)
                    return "<base64>" + data.toByteArray().toBase64() + "</base64>";

                else if (t == QVariant::Map)
                {
                    QString ret = "<struct>";
                    QMap<QString, QVariant> map = data.toMap();
                    QMapIterator<QString, QVariant> i(map);

                    while (i.hasNext())
                    {
                        i.next();
                        ret += "<member><name>" + i.key() + "</name><value>" + 
                            serialize(i.value()) + "</value></member>";
                    }

                    ret += "</struct>";
                    return ret;
                }
                else if (t == QVariant::Hash)
                {
                    QString ret = "<struct>";
                    QHash<QString, QVariant> map = data.toHash();
                    QHashIterator<QString, QVariant> i(map);

                    while (i.hasNext())
                    {
                        i.next();
                        ret += "<member><name>" + i.key() + "</name><value>" + 
                            serialize(i.value()) + "</value></member>";
                    }

                    ret += "</struct>";
                    return ret;
                }
                else if (t == QVariant::StringList)
                {
                    QString ret = "<array><data>";
                    QStringList l = data.toStringList();

                    foreach(QString i, l)
                        ret += "<value>" + entity_encode(i) + "</value>";

                    ret += "</data></array>";
                    return ret;
                }
                else if (t == QVariant::List)
                {
                    QString ret = "<array><data>";
                    QVariantList l = data.toList();

                    foreach(QVariant i, l)
                        ret += "<value>" + serialize(i) + "</value>";

                    ret += "</data></array>";
                    return ret;
                }
                else
                    return "";
            }

            //=========================================================================
            // Replace markup symbols with entities

            QString entity_encode (QString str)
            {
                return str.replace('&', "&amp;")
                    .replace('<', "&lt;")
                    .replace('>', "&gt;");
            }

            //=========================================================================
            // parse XMLRPC reply into a QVariant

            QVariant parse_result (QNetworkReply *reply)
            {
                QVariant result;
                bool fault;

                if (!reply->error())
                {
                    int step = 0;

                    QByteArray data (reply->readAll());
                    QXmlStreamReader xml(data);
                    while (!xml.atEnd())
                    {
                        xml.readNext();
                        if (xml.isStartElement())
                        {
                            if (step == 0)
                            {
                                if (xml.name().toString() == "methodResponse")
                                    step = 1;

                                else
                                    xml.raiseError("expected <methodResponse>,  got:<" + 
                                            xml.name().toString() + ">");
                            }
                            else if (step == 1)
                            {
                                if (xml.name().toString() == "params")
                                    step = 2;

                                else if (xml.name().toString() == "fault")
                                {
                                    fault = true;
                                    step = 3;
                                }
                                else
                                    xml.raiseError("expected <params> or <fault>,  got:<" + 
                                            xml.name().toString() + ">");
                            }
                            else if (step == 2)
                            {
                                if (xml.name().toString() == "param")
                                    step = 3;

                                else
                                    xml.raiseError("expected <param>,  got:<" + 
                                            xml.name().toString() + ">");
                            }
                            else if (step == 3)
                            {
                                if (xml.name().toString() == "value")
                                {
                                    result = deserialize(xml);
                                    step = 4;
                                }
                                else
                                    xml.raiseError("expected <value>,  got:<" + 
                                            xml.name().toString() + ">");
                            }
                        }
                    }

                    if (xml.hasError())
                        qWarning(QString("XmlRpc: " + xml.errorString() + " at line " +
                                    QString::number(xml.lineNumber()) + " column " +
                                    QString::number(xml.columnNumber())).toLocal8Bit().data());
                }

                return result;
            }

            //=========================================================================
            // Deserialize XMLRPC markup to QVariant

            QVariant deserialize (QXmlStreamReader &xml)
            {
                while (!xml.atEnd())
                {
                    xml.readNext();
                    if (xml.isStartElement())
                    {
                        if (xml.name().toString() == "array")
                            return deserialize_array(xml);

                        else if (xml.name().toString() == "base64")
                            return QByteArray::fromBase64(xml.readElementText().toAscii());

                        else if (xml.name().toString() == "boolean")
                            return (xml.readElementText().toInt() == 1);

                        else if (xml.name().toString() == "dateTime.iso8601")
                            return QDateTime::fromString(xml.readElementText(), Qt::ISODate);

                        else if (xml.name().toString() == "double")
                            return xml.readElementText().toDouble();

                        else if (xml.name().toString() == "integer" || xml.name().toString() == "i4")
                            return xml.readElementText().toInt();

                        else if (xml.name().toString() == "string")
                            return xml.readElementText();

                        else if (xml.name().toString() == "struct")
                            return deserialize_struct(xml);
                    }

                    else if (xml.isEndElement())
                        break;
                }

                return QVariant();
            }

            QVariant deserialize_array (QXmlStreamReader &xml)
            {
                QVariantList list;
                int step = 0;
                while (!xml.atEnd())
                {
                    xml.readNext();
                    if (xml.isStartElement())
                    {
                        if (step == 0)
                        {
                            if (xml.name().toString() == "data")
                                step = 1;

                            else
                                xml.raiseError("expected <data>.   got : <" + 
                                        xml.name().toString() + ">");
                        }
                        else if (step == 1)
                        {
                            if (xml.name().toString() == "value")
                            {
                                list += deserialize(xml);
                                step = 2;

                                if (xml.isEndElement() && xml.name().toString() == "value")
                                    step=1;
                            }
                            else
                                xml.raiseError("expected <value>.   got : <" + 
                                        xml.name().toString() + ">");
                        }
                        else if (step == 2)
                            xml.raiseError("expected </value>.   got : <" + 
                                    xml.name().toString() + ">");
                    }
                    else if (xml.isEndElement())
                    {
                        if (step == 2)
                        {
                            if (xml.name().toString() == "value")
                                step = 1;

                            else
                                xml.raiseError("expected </value>.   got : </" + 
                                        xml.name().toString() + ">");
                        }
                        else if (step == 1)
                        {
                            if (xml.name().toString() == "data")
                                step = -1;

                            else
                                xml.raiseError("expected </data>.   got : </" + 
                                        xml.name().toString() + ">");
                        }
                        else if (step == -1)
                        {
                            if (xml.name().toString() == "array")
                                return list;

                            else
                                xml.raiseError("expected </array>.   got : </" + 
                                        xml.name().toString() + ">");
                        }
                        else if (step == 0)
                            xml.raiseError("expected <data>.   got : </" + 
                                    xml.name().toString() + ">");
                    }
                }

                return QVariant();
            }

            QVariant deserialize_struct (QXmlStreamReader &xml)
            {
                QString key;
                QVariantMap map;
                QVariant value;
                int step = 0;

                while (!xml.atEnd())
                {
                    xml.readNext();
                    if (xml.isStartElement())
                    {
                        if (step == 0)
                        {
                            if (xml.name().toString() == "member")
                                step = 1;

                            else
                                xml.raiseError("expected <member>.   got : <" + 
                                        xml.name().toString() + ">");
                        }
                        else if (step == 1)
                        {
                            if (xml.name().toString() == "name")
                                key = xml.readElementText();

                            else if (xml.name().toString() == "value")
                            {
                                value = deserialize(xml);
                                step = 2;

                                if (xml.isEndElement() && xml.name().toString() == "value")
                                    step=1;
                            }
                            else
                                xml.raiseError("expected <name> or <value>.   got : <" + 
                                        xml.name().toString() + ">");
                        }
                        else if (step == 2)
                            xml.raiseError("expected </name>.   got : <" + 
                                    xml.name().toString() + ">");

                        else if (step == 3)
                            xml.raiseError("expected </value>.   got : <" + 
                                    xml.name().toString() + ">");

                    }
                    else if (xml.isEndElement())
                    {
                        if (step == 2)
                        {
                            if (xml.name().toString() == "value")
                                step = 1;

                            else
                                xml.raiseError("expected </value>.   got : </" + 
                                        xml.name().toString() + ">");
                        }
                        else if (step == 1)
                        {
                            if (xml.name().toString() == "member")
                            {
                                map[key] = value;
                                step = 0;
                            }
                            else
                                xml.raiseError("expected </member>.   got : </" + 
                                        xml.name().toString() + ">");
                        }
                        else if (step == 0)
                        {
                            if (xml.name().toString() == "struct")
                                return map;

                            else
                                xml.raiseError("expected </struct>.   got : </" + 
                                        xml.name().toString() + ">");
                        }
                    }
                }

                return QVariant();
            }
        }
    }
}
