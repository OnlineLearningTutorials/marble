//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013      Mohammed Nafees <nafees.technocool@gmail.com>
//

#include "KmlLinkDescriptionTagHandler.h"

#include "KmlElementDictionary.h"
#include "GeoDataNetworkLinkControl.h"
#include "GeoDataParser.h"

namespace Marble
{
namespace kml
{
KML_DEFINE_TAG_HANDLER( linkDescription )

GeoNode* KmllinkDescriptionTagHandler::parse( GeoParser& parser ) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_linkDescription)));

    GeoStackItem parentItem = parser.parentElement();

    if ( parentItem.represents( kmlTag_NetworkLinkControl ) )
    {
        QString linkDescription = parser.readElementText();

        parentItem.nodeAs<GeoDataNetworkLinkControl>()->setLinkDescription( linkDescription );
    }

    return nullptr;
}

}
}
