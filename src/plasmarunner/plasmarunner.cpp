// Copyright 2011 Friedrich W. H. Kossebau <kossebau@kde.org>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program. If not, see <http://www.gnu.org/licenses/>.

#include "plasmarunner.h"

// Marble
#include <GeoDataCoordinates.h>
#include <GeoDataFolder.h>
#include <GeoDataPlacemark.h>
#include <BookmarkManager.h>
#include <GeoDataTreeModel.h>

// KF5
#include <KLocalizedString>

// Qt
#include <QProcess>


#define TRANSLATION_DOMAIN "plasma_runner_marble"

namespace Marble
{

static const int minContainsMatchLength = 3;

PlasmaRunner::PlasmaRunner(QObject *parent, const QVariantList &args)
  : AbstractRunner(parent, args)
{
    setIgnoredTypes(Plasma::RunnerContext::NetworkLocation |
                    Plasma::RunnerContext::FileSystem |
                    Plasma::RunnerContext::Help);

    QList<Plasma::RunnerSyntax> syntaxes;
    syntaxes << Plasma::RunnerSyntax(QStringLiteral(":q:"),
                                     i18n("Shows the coordinates :q: in OpenStreetMap with Marble."));
    syntaxes << Plasma::RunnerSyntax(QStringLiteral(":q:"),
                                     i18n("Shows the geo bookmark containing :q: in OpenStreetMap with Marble."));
    setSyntaxes(syntaxes);
}

void PlasmaRunner::match(Plasma::RunnerContext &context)
{
    QList<Plasma::QueryMatch> matches;

    const QString query = context.query();

    bool success = false;
    // TODO: how to estimate that input is in Degree, not Radian?
    GeoDataCoordinates coordinates = GeoDataCoordinates::fromString(query, success);

    if (success) {
        const QVariant coordinatesData = QVariantList()
            << QVariant(coordinates.longitude(GeoDataCoordinates::Degree))
            << QVariant(coordinates.latitude(GeoDataCoordinates::Degree))
            << QVariant(0.1); // TODO: make this distance value configurable

        Plasma::QueryMatch match(this);
        match.setIcon(QIcon::fromTheme(QStringLiteral("marble")));
        match.setText(i18n("Show the coordinates %1 in OpenStreetMap with Marble", query));
        match.setData(coordinatesData);
        match.setId(query);
        match.setRelevance(1.0);
        match.setType(Plasma::QueryMatch::ExactMatch);

        matches << match;
    }

    // TODO: BookmarkManager does not yet listen to updates, also does not sync between processes :(
    // So for now always load on demand, even if expensive possibly
    BookmarkManager bookmarkManager(new GeoDataTreeModel);
    bookmarkManager.loadFile( QStringLiteral("bookmarks/bookmarks.kml") );

    foreach (GeoDataFolder* folder, bookmarkManager.folders()) {
        collectMatches(matches, query, folder);
    }

    if ( ! matches.isEmpty() ) {
        context.addMatches(matches);
    }
}

void PlasmaRunner::collectMatches(QList<Plasma::QueryMatch> &matches,
                                  const QString &query, const GeoDataFolder *folder)
{
    const QString queryLower = query.toLower();

    QVector<GeoDataFeature*>::const_iterator it = folder->constBegin();
    QVector<GeoDataFeature*>::const_iterator end = folder->constEnd();

    for (; it != end; ++it) {
        GeoDataFolder *folder = dynamic_cast<GeoDataFolder*>(*it);
        if ( folder ) {
            collectMatches(matches, query, folder);
            continue;
        }

        GeoDataPlacemark *placemark = dynamic_cast<GeoDataPlacemark*>( *it );
        if ( placemark ) {
            // For short query strings only match exactly, to get a sane number of matches
            if (query.length() < minContainsMatchLength) {
                if ( placemark->name().toLower() != queryLower &&
                     ( placemark->descriptionIsCDATA() || // TODO: support also with CDATA
                       placemark->description().toLower() != queryLower ) ) {
                    continue;
                }
            } else {
                if ( ! placemark->name().toLower().contains(queryLower) &&
                     ( placemark->descriptionIsCDATA() || // TODO: support also with CDATA
                       ! placemark->description().toLower().contains(queryLower) ) ) {
                    continue;
                }
            }

            const GeoDataCoordinates coordinates = placemark->coordinate();
            const qreal lon = coordinates.longitude(GeoDataCoordinates::Degree);
            const qreal lat = coordinates.latitude(GeoDataCoordinates::Degree);
            const QVariant coordinatesData = QVariantList()
                << QVariant(lon)
                << QVariant(lat)
                << QVariant(placemark->lookAt()->range()*METER2KM);

            Plasma::QueryMatch match(this);
            match.setIcon(QIcon::fromTheme(QStringLiteral("marble")));
            match.setText(placemark->name());
            match.setSubtext(i18n("Show in OpenStreetMap with Marble"));
            match.setData(coordinatesData);
            match.setId(placemark->name()+QString::number(lat)+QString::number(lon));
            match.setRelevance(1.0);
            match.setType(Plasma::QueryMatch::ExactMatch);

            matches << match;
        }
    }
}

void PlasmaRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)

    const QVariantList data = match.data().toList();

    // pass in C locale, should be always understood
    const QString latLon =
        QString::number(data.at(1).toReal()) + QLatin1Char(' ') + QString::number(data.at(0).toReal());

    const QString distance = data.at(2).toString();

    const QStringList parameters = QStringList()
        << QStringLiteral( "--latlon" )
        << latLon
        << QStringLiteral( "--distance" )
        << distance
        << QStringLiteral( "--map" )
        << QStringLiteral( "earth/openstreetmap/openstreetmap.dgml" );
    QProcess::startDetached( QStringLiteral("marble"), parameters );
}

}
