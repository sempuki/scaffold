/* capabilities.cpp -- Qt-based implementation of Capabilities
 *
 */

#include <QXmlStreamReader>
#include <QNetworkRequest> 
#include <QNetworkReply> 

#include "capabilities.hpp"

namespace Scaffold
{
    namespace Connectivity
    {
        namespace Capabilities
        {
            //=========================================================================
            // Parse caps from LLSD map to QMap

            QMap <QString, QUrl> parse_capabilities (QNetworkReply *reply)
            {
                QMap <QString, QUrl> result;
                bool fault;

                if (!reply->error())
                {
                    int step = 0;

                    QString key; QUrl url;
                    QXmlStreamReader xml(reply->readAll());

                    while (!xml.atEnd())
                    {
                        xml.readNext();

                        if (xml.isStartDocument())
                            continue;

                        else if (xml.isEndDocument())
                            break;

                        else if (xml.isStartElement())
                        {
                            switch (step)
                            {
                                case 0:
                                    if (xml.name().toString() == "llsd")
                                        step = 1;

                                    else
                                        xml.raiseError ("expected <llsd>. got: <" + 
                                                xml.name().toString() + ">");
                                    break;

                                case 1:
                                    if (xml.name().toString() == "map")
                                        step = 2;

                                    else
                                        xml.raiseError ("expected <map>. got: <" + 
                                                xml.name().toString() + ">");
                                    break;

                                case 2:
                                    if (xml.name().toString() == "key")
                                    {
                                        step = 3;
                                        key = xml.readElementText();
                                    }

                                    else
                                        xml.raiseError ("expected <key>. got: <" + 
                                                xml.name().toString() + ">");
                                    break;

                                case 3:
                                    if (xml.name().toString() == "string")
                                    {
                                        step = 2;
                                        url = xml.readElementText();

                                        result.insert (key, url);
                                        key.clear(); url.clear();
                                    }

                                    else
                                        xml.raiseError ("expected <string>. got: <" + 
                                                xml.name().toString() + ">");
                                    break;

                                default:
                                    xml.raiseError ("unable to parse: " + QString::number (step));
                            }

                        }
                        else if (xml.isEndElement())
                        {
                            switch (step)
                            {
                                case 1:
                                    if (xml.name().toString() == "llsd")
                                        step = 0;

                                    else
                                        xml.raiseError ("expected </llsd>. got: <" + 
                                                xml.name().toString() + ">");
                                    break;

                                case 2:
                                    if (xml.name().toString() == "map")
                                        step = 1;

                                    else
                                        xml.raiseError ("expected </map>. got: <" + 
                                                xml.name().toString() + ">");
                                    break;

                                default:
                                    xml.raiseError ("unable to parse: " + QString::number (step));
                            }
                        }

                        if (xml.hasError())
                            qWarning(QString("Capabilities: " + xml.errorString() + " at line " +
                                        QString::number(xml.lineNumber()) + " column " +
                                        QString::number(xml.columnNumber())).toLocal8Bit().data());
                    }
                }

                return result;
            }

            //=========================================================================
            //

            Caps::Caps (QNetworkReply *r)
                : reply (r)
            {
                // INFO: Caps::on_result is always called before other 
                // listeners if it's always connected before 
                connect (reply, SIGNAL (finished()), this, SLOT (on_result()));
            }

            Caps::~Caps ()
            {
                reply-> deleteLater();
            }

            void Caps::on_result ()
            {
                result = parse_capabilities (reply);
            }

            //=========================================================================
            //

            Client::Client (const QUrl &seed, QNetworkAccessManager *http)
                : seed_ (seed), http_ ((http)? http : new QNetworkAccessManager (this))
                {
                }

            Caps *Client::request ()
            {
                QByteArray caps = 
                    "<llsd><array>"
                    "<string>ChatSessionRequest</string>"
                    "<string>CopyInventoryFromNotecard</string>"
                    "<string>DispatchRegionInfo</string>"
                    "<string>EstateChangeInfo</string>"
                    "<string>EventQueueGet</string>"
                    "<string>FetchInventoryDescendents</string>"
                    "<string>GroupProposalBallot</string>"
                    "<string>MapLayer</string>"
                    "<string>MapLayerGod</string>"
                    "<string>NewFileAgentInventory</string>"
                    "<string>ParcelPropertiesUpdate</string>"
                    "<string>ParcelVoiceInfoRequest</string>"
                    "<string>ProvisionVoiceAccountRequest</string>"
                    "<string>RemoteParcelRequest</string>"
                    "<string>RequestTextureDownload</string>"
                    "<string>SearchStatRequest</string>"
                    "<string>SearchStatTracking</string>"
                    "<string>SendPostcard</string>"
                    "<string>SendUserReport</string>"
                    "<string>SendUserReportWithScreenshot</string>"
                    "<string>ServerReleaseNotes</string>"
                    "<string>StartGroupProposal</string>"
                    "<string>UpdateGestureAgentInventory</string>"
                    "<string>UpdateNotecardAgentInventory</string>"
                    "<string>UpdateScriptAgentInventory</string>"
                    "<string>UpdateGestureTaskInventory</string>"
                    "<string>UpdateNotecardTaskInventory</string>"
                    "<string>UpdateScriptTaskInventory</string>"
                    "<string>ViewerStartAuction</string>"
                    "<string>UntrustedSimulatorMessage</string>"
                    "<string>ViewerStats</string>"
                    "</array></llsd>";

                QNetworkRequest request (seed_);
                request.setHeader (QNetworkRequest::ContentTypeHeader, "text/xml");
                request.setRawHeader ("Connection", "close");

                return new Caps (http_-> post (request, caps));
            }
        }
    }
}
