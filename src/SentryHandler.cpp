/***************************************************************************
 *   Copyright (C) 2025 by Mudlet Development Team                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "SentryHandler.h"
#include <QDebug>
#include <QVariantMap>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCoreApplication>
#include <QSysInfo>
#include <QDir>
#include <QFile>

#ifdef USE_SENTRY
#include <sentry.h>
#endif

class SentryHandler::Private {
public:
    Private() : initialized(false) {}
    bool initialized;
    QString currentDsn;
};

SentryHandler::SentryHandler() 
    : QObject(nullptr)
    , d(std::make_unique<Private>())
    , mInitialized(false)
{
}

SentryHandler::~SentryHandler()
{
    shutdown();
}

SentryHandler& SentryHandler::instance()
{
    static SentryHandler instance;
    return instance;
}

void SentryHandler::initialize(const QString& dsn)
{
#ifdef USE_SENTRY
    if (d->initialized) {
        qDebug() << "Sentry is already initialized";
        return;
    }
    
    QString finalDsn = dsn;
    if (finalDsn.isEmpty()) {
        // Use default Mudlet Sentry DSN - this should be configured during build
        finalDsn = QStringLiteral(MUDLET_SENTRY_DSN);
    }
    
    if (finalDsn.isEmpty()) {
        qWarning() << "Cannot initialize Sentry: no DSN provided";
        return;
    }
    
    sentry_options_t *options = sentry_options_new();
    sentry_options_set_dsn(options, finalDsn.toUtf8().constData());
    
    // Set up the database path for offline caching
    QString dbPath = QDir::tempPath() + QStringLiteral("/mudlet-sentry");
    QDir().mkpath(dbPath);
    sentry_options_set_database_path(options, dbPath.toUtf8().constData());
    
    // Set the release version
    QString release = QStringLiteral("mudlet@") + QCoreApplication::applicationVersion();
    sentry_options_set_release(options, release.toUtf8().constData());
    
    // Set environment based on build type
#ifdef QT_DEBUG
    sentry_options_set_environment(options, "development");
#else
    sentry_options_set_environment(options, "production");
#endif
    
    // Platform-specific configuration
#if defined(Q_OS_WIN)
    // Windows-specific settings
    sentry_options_set_handler_path(options, "./crashpad_handler.exe");
#elif defined(Q_OS_MACOS)
    // macOS-specific settings
    sentry_options_set_handler_path(options, "./crashpad_handler");
#elif defined(Q_OS_LINUX)
    // Linux-specific settings
    // Try to find crashpad handler
    if (QFile::exists("/home/mavrick/.local/bin/crashpad_handler")) {
        sentry_options_set_handler_path(options, "/home/mavrick/.local/bin/crashpad_handler");
    } else if (QFile::exists("./crashpad_handler")) {
        sentry_options_set_handler_path(options, "./crashpad_handler");
    } else {
        // Fall back to in-process backend
        qDebug() << "Crashpad handler not found, using in-process backend";
    }
#endif
    
    // Enable automatic session tracking
    sentry_options_set_auto_session_tracking(options, 1);
    
    // Set sample rate (1.0 = 100% of events are sent)
    sentry_options_set_sample_rate(options, 1.0);
    
    // Enable debug output in debug builds
#ifdef QT_DEBUG
    sentry_options_set_debug(options, 1);
#endif
    
    // Add system context
    sentry_options_set_system_crash_reporter_enabled(options, 1);
    
    // Initialize Sentry
    if (sentry_init(options) == 0) {
        d->initialized = true;
        d->currentDsn = finalDsn;
        mInitialized = true;
        
        // Set default tags
        setTag(QStringLiteral("os"), QSysInfo::productType());
        setTag(QStringLiteral("os_version"), QSysInfo::productVersion());
        setTag(QStringLiteral("arch"), QSysInfo::currentCpuArchitecture());
        setTag(QStringLiteral("qt_version"), QString::fromLatin1(qVersion()));
        
        qDebug() << "Sentry initialized successfully";
    } else {
        qWarning() << "Failed to initialize Sentry";
    }
#else
    Q_UNUSED(dsn)
    qDebug() << "Sentry support not compiled in (USE_SENTRY not defined)";
#endif
}

void SentryHandler::shutdown()
{
#ifdef USE_SENTRY
    if (d->initialized) {
        sentry_close();
        d->initialized = false;
        mInitialized = false;
        qDebug() << "Sentry shutdown complete";
    }
#endif
}

void SentryHandler::setUser(const QString& userId, const QString& username, const QString& email)
{
#ifdef USE_SENTRY
    if (!d->initialized) return;
    
    sentry_value_t user = sentry_value_new_object();
    if (!userId.isEmpty()) {
        sentry_value_set_by_key(user, "id", sentry_value_new_string(userId.toUtf8().constData()));
    }
    if (!username.isEmpty()) {
        sentry_value_set_by_key(user, "username", sentry_value_new_string(username.toUtf8().constData()));
    }
    if (!email.isEmpty()) {
        sentry_value_set_by_key(user, "email", sentry_value_new_string(email.toUtf8().constData()));
    }
    sentry_set_user(user);
#else
    Q_UNUSED(userId)
    Q_UNUSED(username)
    Q_UNUSED(email)
#endif
}

void SentryHandler::clearUser()
{
#ifdef USE_SENTRY
    if (!d->initialized) return;
    sentry_remove_user();
#endif
}

void SentryHandler::addBreadcrumb(const QString& message, const QString& category, const QString& level)
{
#ifdef USE_SENTRY
    if (!d->initialized) return;
    
    sentry_value_t crumb = sentry_value_new_breadcrumb("default", message.toUtf8().constData());
    if (!category.isEmpty()) {
        sentry_value_set_by_key(crumb, "category", sentry_value_new_string(category.toUtf8().constData()));
    }
    if (!level.isEmpty()) {
        sentry_value_set_by_key(crumb, "level", sentry_value_new_string(level.toUtf8().constData()));
    }
    sentry_add_breadcrumb(crumb);
#else
    Q_UNUSED(message)
    Q_UNUSED(category)
    Q_UNUSED(level)
#endif
}

void SentryHandler::captureMessage(const QString& message, const QString& level)
{
#ifdef USE_SENTRY
    if (!d->initialized) return;
    
    sentry_level_t sentryLevel = SENTRY_LEVEL_INFO;
    if (level == QStringLiteral("debug")) {
        sentryLevel = SENTRY_LEVEL_DEBUG;
    } else if (level == QStringLiteral("info")) {
        sentryLevel = SENTRY_LEVEL_INFO;
    } else if (level == QStringLiteral("warning")) {
        sentryLevel = SENTRY_LEVEL_WARNING;
    } else if (level == QStringLiteral("error")) {
        sentryLevel = SENTRY_LEVEL_ERROR;
    } else if (level == QStringLiteral("fatal")) {
        sentryLevel = SENTRY_LEVEL_FATAL;
    }
    
    sentry_capture_event(sentry_value_new_message_event(
        sentryLevel,
        "mudlet",
        message.toUtf8().constData()));
#else
    Q_UNUSED(message)
    Q_UNUSED(level)
#endif
}

void SentryHandler::captureException(const QString& type, const QString& value)
{
#ifdef USE_SENTRY
    if (!d->initialized) return;
    
    sentry_value_t event = sentry_value_new_event();
    sentry_value_t exception = sentry_value_new_exception(type.toUtf8().constData(), value.toUtf8().constData());
    sentry_event_add_exception(event, exception);
    sentry_capture_event(event);
#else
    Q_UNUSED(type)
    Q_UNUSED(value)
#endif
}

void SentryHandler::setTag(const QString& key, const QString& value)
{
#ifdef USE_SENTRY
    if (!d->initialized) return;
    sentry_set_tag(key.toUtf8().constData(), value.toUtf8().constData());
#else
    Q_UNUSED(key)
    Q_UNUSED(value)
#endif
}

void SentryHandler::removeTag(const QString& key)
{
#ifdef USE_SENTRY
    if (!d->initialized) return;
    sentry_remove_tag(key.toUtf8().constData());
#else
    Q_UNUSED(key)
#endif
}

void SentryHandler::setContext(const QString& key, const QVariantMap& value)
{
#ifdef USE_SENTRY
    if (!d->initialized) return;
    
    sentry_value_t context = sentry_value_new_object();
    for (auto it = value.begin(); it != value.end(); ++it) {
        QString strValue = it.value().toString();
        sentry_value_set_by_key(context, it.key().toUtf8().constData(), 
                                 sentry_value_new_string(strValue.toUtf8().constData()));
    }
    sentry_set_context(key.toUtf8().constData(), context);
#else
    Q_UNUSED(key)
    Q_UNUSED(value)
#endif
}

void SentryHandler::removeContext(const QString& key)
{
#ifdef USE_SENTRY
    if (!d->initialized) return;
    sentry_remove_context(key.toUtf8().constData());
#else
    Q_UNUSED(key)
#endif
}

void SentryHandler::setRelease(const QString& release)
{
#ifdef USE_SENTRY
    if (!d->initialized) {
        qWarning() << "Cannot set release: Sentry not initialized";
        return;
    }
    // Note: Release is set during initialization and cannot be changed afterwards in Sentry Native
    qDebug() << "Release version is set during initialization and cannot be changed";
#else
    Q_UNUSED(release)
#endif
}

void SentryHandler::setEnvironment(const QString& environment)
{
#ifdef USE_SENTRY
    if (!d->initialized) {
        qWarning() << "Cannot set environment: Sentry not initialized";
        return;
    }
    // Note: Environment is set during initialization and cannot be changed afterwards in Sentry Native
    qDebug() << "Environment is set during initialization and cannot be changed";
#else
    Q_UNUSED(environment)
#endif
}

void SentryHandler::flush(int timeoutMillis)
{
#ifdef USE_SENTRY
    if (!d->initialized) return;
    sentry_flush(timeoutMillis);
#else
    Q_UNUSED(timeoutMillis)
#endif
}
