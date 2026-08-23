#pragma once

#include <QSharedPointer>

#include "data/TreeRepository.h"

namespace TestDatabaseUtility
{
    QSharedPointer<TreeRepository> createTestRepository();
}
