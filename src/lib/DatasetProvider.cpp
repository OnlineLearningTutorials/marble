//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008 Torsten Rahn <tackat@kde.org>"
//


// Own
#include "DatasetProvider.h"

// Qt
#include <QtCore/QDebug>

// Local dir

namespace Marble
{

class DatasetProviderPrivate
{
 public:
    DatasetProviderPrivate()
    {
    }
};

DatasetProvider::DatasetProvider( QObject *parent )
    : QObject(parent),
      d( new DatasetProviderPrivate() )
{
}

DatasetProvider::~DatasetProvider()
{
    delete d;
}


}

#include "DatasetProvider.moc"
