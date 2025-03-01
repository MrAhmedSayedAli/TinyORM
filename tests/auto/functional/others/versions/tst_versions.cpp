#include <QCoreApplication>
#include <QtTest>

#if defined(_WIN32) && defined(TINYTEST_VERSIONS_IS_SHARED_BUILD)
#  include <qt_windows.h>
#endif

#ifdef _WIN32
#  include "fs.hpp"
#endif

// TinyORM
#include "orm/constants.hpp"
#include "orm/version.hpp"
// TinyUtils
#include "version.hpp"
// Tom example (defined on windows only)
#ifdef TINYTOM_EXAMPLE
#  include "tom/version.hpp"
#endif

// Used by checkFileVersion_*() tests
#if defined(_WIN32) && defined(TINYTEST_VERSIONS_IS_SHARED_BUILD)
#  ifdef TINYTEST_VERSIONS_IS_CMAKE
#    include TINYTEST_VERSIONS_VERSIONSDEBUG
#  elif defined(TINYTEST_VERSIONS_IS_QMAKE)
#    include "versionsdebug_qmake.hpp"
#  endif
#else
#  define TINYTEST_VERSIONS_TINYORM_PATH
#  define TINYTEST_VERSIONS_TINYUTILS_PATH
#  define TINYTEST_VERSIONS_TOMEXAMPLE_PATH
#endif

using Orm::Constants::DOT;

#ifdef _WIN32
using TestUtils::Fs;
#endif

class tst_Versions : public QObject // clazy:exclude=ctor-missing-parent-argument
{
    Q_OBJECT

private Q_SLOTS:
    void versions_TinyOrm() const;
    void versions_TinyUtils() const;
#ifdef TINYTOM_EXAMPLE
    void versions_TomExample() const;
#endif

    void checkFileVersion_TinyOrm() const;
    void checkFileVersion_TinyUtils() const;
#ifdef TINYTOM_EXAMPLE
    void checkFileVersion_TomExample() const;
#endif

#if defined(_WIN32) && defined(TINYTEST_VERSIONS_IS_SHARED_BUILD)
// NOLINTNEXTLINE(readability-redundant-access-specifiers)
private:
    /*! Return value for the getExeVersionString(). */
    struct FileVersions
    {
        /*! ProductVersion. */
        QString productVersion;
        /*! FileVersion. */
        QString fileVersion;
        /*! LegalCopyright. */
        QString copyright;
    };

    /*! Obtain a ProductVersion and FileVersion strings from an exe/dll. */
    [[nodiscard]] static FileVersions getExeVersionString(const QString &fileName);
#endif
};

/* private slots */

#if defined(_WIN32) && defined(TINYTEST_VERSIONS_IS_SHARED_BUILD)
/*! Executables copyright, all executables have the same LegalCopyright. */
Q_GLOBAL_STATIC_WITH_ARGS(QString, CopyRight, ("Copyright (©) 2022 Crystal Studio")) // NOLINT(misc-use-anonymous-namespace)
#endif

// NOLINTBEGIN(readability-convert-member-functions-to-static)
void tst_Versions::versions_TinyOrm() const
{
    // Test types
    QCOMPARE(typeid (TINYORM_VERSION_MAJOR), typeid (int));
    QCOMPARE(typeid (TINYORM_VERSION_MINOR), typeid (int));
    QCOMPARE(typeid (TINYORM_VERSION_BUGFIX), typeid (int));
    QCOMPARE(typeid (TINYORM_VERSION_BUILD), typeid (int));

    // Individual version numbers have to be greater than zero
    QVERIFY(TINYORM_VERSION_MAJOR >= 0);
    QVERIFY(TINYORM_VERSION_MINOR >= 0);
    QVERIFY(TINYORM_VERSION_BUGFIX >= 0);
    QVERIFY(TINYORM_VERSION_BUILD >= 0);

    // Project and File Version strings
    QString versionStr = QString::number(TINYORM_VERSION_MAJOR) + DOT +
                         QString::number(TINYORM_VERSION_MINOR) + DOT +
                         QString::number(TINYORM_VERSION_BUGFIX);
    QString fileVersionStr = versionStr + DOT +
                             QString::number(TINYORM_VERSION_BUILD);
    if constexpr (TINYORM_VERSION_BUILD > 0)
        versionStr += DOT + QString::number(TINYORM_VERSION_BUILD);
    versionStr += TINYORM_VERSION_STATUS;

    QCOMPARE(TINYORM_FILEVERSION_STR, fileVersionStr);
    QCOMPARE(TINYORM_VERSION_STR, versionStr);
    QCOMPARE(TINYORM_VERSION_STR_2, QLatin1Char('v') + versionStr);

    // Project Version number, to check API compatibility
    const auto version = TINYORM_VERSION_MAJOR * 10000 +
                         TINYORM_VERSION_MINOR * 100 +
                         TINYORM_VERSION_BUGFIX;
    QCOMPARE(TINYORM_VERSION, version);
}

void tst_Versions::versions_TinyUtils() const
{
    // Test types
    QCOMPARE(typeid (TINYUTILS_VERSION_MAJOR), typeid (int));
    QCOMPARE(typeid (TINYUTILS_VERSION_MINOR), typeid (int));
    QCOMPARE(typeid (TINYUTILS_VERSION_BUGFIX), typeid (int));
    QCOMPARE(typeid (TINYUTILS_VERSION_BUILD), typeid (int));

    // Individual version numbers have to be greater than zero
    QVERIFY(TINYUTILS_VERSION_MAJOR >= 0);
    QVERIFY(TINYUTILS_VERSION_MINOR >= 0);
    QVERIFY(TINYUTILS_VERSION_BUGFIX >= 0);
    QVERIFY(TINYUTILS_VERSION_BUILD >= 0);

    // Project and File Version strings
    QString versionStr = QString::number(TINYUTILS_VERSION_MAJOR) + DOT +
                         QString::number(TINYUTILS_VERSION_MINOR) + DOT +
                         QString::number(TINYUTILS_VERSION_BUGFIX);
    QString fileVersionStr = versionStr + DOT +
                             QString::number(TINYUTILS_VERSION_BUILD);
    if constexpr (TINYUTILS_VERSION_BUILD > 0)
        versionStr += DOT + QString::number(TINYUTILS_VERSION_BUILD);

    QCOMPARE(TINYUTILS_FILEVERSION_STR, fileVersionStr);
    QCOMPARE(TINYUTILS_VERSION_STR, versionStr);
    QCOMPARE(TINYUTILS_VERSION_STR_2, QLatin1Char('v') + versionStr);

    // Project Version number, to check API compatibility
    const auto version = TINYUTILS_VERSION_MAJOR * 10000 +
                         TINYUTILS_VERSION_MINOR * 100 +
                         TINYUTILS_VERSION_BUGFIX;
    QCOMPARE(TINYUTILS_VERSION, version);
}

#ifdef TINYTOM_EXAMPLE
void tst_Versions::versions_TomExample() const
{
    // Test types
    QCOMPARE(typeid (TINYTOM_VERSION_MAJOR), typeid (int));
    QCOMPARE(typeid (TINYTOM_VERSION_MINOR), typeid (int));
    QCOMPARE(typeid (TINYTOM_VERSION_BUGFIX), typeid (int));
    QCOMPARE(typeid (TINYTOM_VERSION_BUILD), typeid (int));

    // Individual version numbers have to be greater than zero
    QVERIFY(TINYTOM_VERSION_MAJOR >= 0);
    QVERIFY(TINYTOM_VERSION_MINOR >= 0);
    QVERIFY(TINYTOM_VERSION_BUGFIX >= 0);
    QVERIFY(TINYTOM_VERSION_BUILD >= 0);

    // Project and File Version strings
    QString versionStr = QString::number(TINYTOM_VERSION_MAJOR) + DOT +
                         QString::number(TINYTOM_VERSION_MINOR) + DOT +
                         QString::number(TINYTOM_VERSION_BUGFIX);
    QString fileVersionStr = versionStr + DOT +
                             QString::number(TINYTOM_VERSION_BUILD);
    if constexpr (TINYTOM_VERSION_BUILD > 0)
        versionStr += DOT + QString::number(TINYTOM_VERSION_BUILD);
    versionStr += TINYTOM_VERSION_STATUS;

    QCOMPARE(TINYTOM_FILEVERSION_STR, fileVersionStr);
    QCOMPARE(TINYTOM_VERSION_STR, versionStr);
    QCOMPARE(TINYTOM_VERSION_STR_2, QLatin1Char('v') + versionStr);

    // Project Version number, to check API compatibility
    const auto version = TINYTOM_VERSION_MAJOR * 10000 +
                         TINYTOM_VERSION_MINOR * 100 +
                         TINYTOM_VERSION_BUGFIX;
    QCOMPARE(TINYTOM_VERSION, version);
}
#endif

void tst_Versions::checkFileVersion_TinyOrm() const
{
#ifndef _WIN32
    QSKIP("checkFileVersion_*() related tests are supported on MSVC only.", );
#elif !defined(TINYTEST_VERSIONS_IS_SHARED_BUILD)
    QSKIP("checkFileVersion_*() related tests are enabled for shared builds only.", );
#else
    const auto fileVersions =
            getExeVersionString(Fs::absolutePath(TINYTEST_VERSIONS_TINYORM_PATH));

    // Project and File Version strings
    const QString versionStr = QString::number(TINYORM_VERSION_MAJOR) + DOT +
                               QString::number(TINYORM_VERSION_MINOR) + DOT +
                               QString::number(TINYORM_VERSION_BUGFIX) + DOT +
                               QString::number(TINYORM_VERSION_BUILD);

    QCOMPARE(fileVersions.productVersion, versionStr);
    QCOMPARE(fileVersions.fileVersion, fileVersions.productVersion);
    QCOMPARE(fileVersions.copyright, *CopyRight);
#endif
}

void tst_Versions::checkFileVersion_TinyUtils() const
{
#ifndef _WIN32
    QSKIP("checkFileVersion_*() related tests are supported on MSVC only.", );
#elif !defined(TINYTEST_VERSIONS_IS_SHARED_BUILD)
    QSKIP("checkFileVersion_*() related tests are enabled for shared builds only.", );
#else
    const auto fileVersions =
            getExeVersionString(Fs::absolutePath(TINYTEST_VERSIONS_TINYUTILS_PATH));

    // Project and File Version strings
    const QString versionStr = QString::number(TINYUTILS_VERSION_MAJOR) + DOT +
                               QString::number(TINYUTILS_VERSION_MINOR) + DOT +
                               QString::number(TINYUTILS_VERSION_BUGFIX) + DOT +
                               QString::number(TINYUTILS_VERSION_BUILD);

    QCOMPARE(fileVersions.productVersion, versionStr);
    QCOMPARE(fileVersions.fileVersion, fileVersions.productVersion);
    QCOMPARE(fileVersions.copyright, *CopyRight);
#endif
}

#ifdef TINYTOM_EXAMPLE
void tst_Versions::checkFileVersion_TomExample() const
{
#ifndef _WIN32
    QSKIP("checkFileVersion_*() related tests are supported on MSVC only.", );
#elif !defined(TINYTEST_VERSIONS_IS_SHARED_BUILD)
    QSKIP("checkFileVersion_*() related tests are enabled for shared builds only.", );
#else
    const auto fileVersions =
            getExeVersionString(Fs::absolutePath(TINYTEST_VERSIONS_TOMEXAMPLE_PATH));

    // Project and File Version strings
    const QString versionStr = QString::number(TINYTOM_VERSION_MAJOR) + DOT +
                               QString::number(TINYTOM_VERSION_MINOR) + DOT +
                               QString::number(TINYTOM_VERSION_BUGFIX) + DOT +
                               QString::number(TINYTOM_VERSION_BUILD);

    QCOMPARE(fileVersions.productVersion, versionStr);
    QCOMPARE(fileVersions.fileVersion, fileVersions.productVersion);
    QCOMPARE(fileVersions.copyright, *CopyRight);
#endif
}
#endif
// NOLINTEND(readability-convert-member-functions-to-static)

/* private */

#if defined(_WIN32) && defined(TINYTEST_VERSIONS_IS_SHARED_BUILD)
tst_Versions::FileVersions
tst_Versions::getExeVersionString(const QString &fileName)
{
    // first of all, GetFileVersionInfoSize
    DWORD dwHandle = 1;
    DWORD dwLen = GetFileVersionInfoSize(fileName.toStdWString().c_str(), &dwHandle);
    if (dwLen == 0) {
        qWarning() << "Error in GetFileVersionInfoSize().";
        return {};
    }

    // GetFileVersionInfo
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    auto lpData = std::make_unique<BYTE[]>(dwLen);
    if (!GetFileVersionInfo(fileName.toStdWString().c_str(), 0, dwLen,
                            static_cast<LPVOID>(lpData.get()))) {
        qWarning() << "Error in GetFileVersionInfo().";
        return {};
    }

    // Get the copyright
    LPTSTR copyrightPtr = nullptr;
    UINT uLen = 0;

    if (!VerQueryValue(lpData.get(), LR"(\StringFileInfo\040904b0\LegalCopyright)",
                       reinterpret_cast<LPVOID *>(&copyrightPtr), &uLen)
    ) {
        qWarning() << "Error in VerQueryValue() for the LegalCopyright.";
        return {};
    }

    // -1 to exclude null character
    auto copyright = QString::fromWCharArray(copyrightPtr,
                                             static_cast<QString::size_type>(uLen) - 1);

    // VerQueryValue
    VS_FIXEDFILEINFO *lpBuffer = nullptr;
    uLen = 0;

    if (!VerQueryValue(lpData.get(), QString("\\").toStdWString().c_str(),
                       reinterpret_cast<LPVOID *>(&lpBuffer), &uLen)
    ) {
        qWarning() << "Error in VerQueryValue() for the version-information resource.";
        return {};
    }

    return {
        // Product Version
        QStringLiteral("%1.%2.%3.%4")
                .arg(HIWORD(lpBuffer->dwProductVersionMS))
                .arg(LOWORD(lpBuffer->dwProductVersionMS))
                .arg(HIWORD(lpBuffer->dwProductVersionLS))
                .arg(LOWORD(lpBuffer->dwProductVersionLS)),
        // File Version
        QStringLiteral("%1.%2.%3.%4")
                .arg(HIWORD(lpBuffer->dwFileVersionMS))
                .arg(LOWORD(lpBuffer->dwFileVersionMS))
                .arg(HIWORD(lpBuffer->dwFileVersionLS))
                .arg(LOWORD(lpBuffer->dwFileVersionLS)),
        // LegalCopyright
        std::move(copyright)
    };
}
#endif

QTEST_MAIN(tst_Versions)

#include "tst_versions.moc"
