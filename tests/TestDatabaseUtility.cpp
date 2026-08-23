#include "TestDatabaseUtility.h"

#include <QtGlobal>

namespace TestDatabaseUtility
{
    QSharedPointer<TreeRepository> createTestRepository()
    {
        QSharedPointer<TreeRepository> repository = QSharedPointer<TreeRepository>::create();

        const QString databaseName = qEnvironmentVariable(
            "ADEPT_TEST_DB_NAME",
            QStringLiteral("adept_tree_test"));

        const QString hostName = qEnvironmentVariable(
            "ADEPT_TEST_DB_HOST",
            QStringLiteral("localhost"));

        const QString userName = qEnvironmentVariable(
            "ADEPT_TEST_DB_USER",
            QStringLiteral("adept_user"));

        const QString password = qEnvironmentVariable(
            "ADEPT_TEST_DB_PASSWORD",
            QStringLiteral("adept_pass"));

        bool portConverted = false;

        int port = qEnvironmentVariable(
            "ADEPT_TEST_DB_PORT",
            QStringLiteral("5432")).toInt(&portConverted);

        if (!portConverted)
        {
            port = 5432;
        }

        if (!repository->open(databaseName,
                              hostName,
                              port,
                              userName,
                              password))
        {
            return QSharedPointer<TreeRepository>();
        }

        if (!repository->ensureSchema())
        {
            return QSharedPointer<TreeRepository>();
        }

        if (!repository->clearAll())
        {
            return QSharedPointer<TreeRepository>();
        }

        return repository;
    }
}
