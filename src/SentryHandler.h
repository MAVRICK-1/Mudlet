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

#ifndef MUDLET_SENTRYHANDLER_H
#define MUDLET_SENTRYHANDLER_H

#include <QString>
#include <QObject>
#include <memory>

class SentryHandler : public QObject
{
    Q_OBJECT
public:
    static SentryHandler& instance();
    
    void initialize(const QString& dsn = QString());
    void shutdown();
    
    void setUser(const QString& userId, const QString& username = QString(), const QString& email = QString());
    void clearUser();
    
    void addBreadcrumb(const QString& message, const QString& category = QString(), const QString& level = QStringLiteral("info"));
    void captureMessage(const QString& message, const QString& level = QStringLiteral("info"));
    void captureException(const QString& type, const QString& value);
    
    void setTag(const QString& key, const QString& value);
    void removeTag(const QString& key);
    
    void setContext(const QString& key, const QVariantMap& value);
    void removeContext(const QString& key);
    
    void setRelease(const QString& release);
    void setEnvironment(const QString& environment);
    
    bool isInitialized() const { return mInitialized; }
    
    void flush(int timeoutMillis = 2000);

private:
    SentryHandler();
    ~SentryHandler();
    SentryHandler(const SentryHandler&) = delete;
    SentryHandler& operator=(const SentryHandler&) = delete;
    
    class Private;
    std::unique_ptr<Private> d;
    bool mInitialized;
};

#endif // MUDLET_SENTRYHANDLER_H
