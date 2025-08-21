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

#include "DefaultClientUtils.h"
#include <QCoreApplication>
#include <QProcess>
#include <QDebug>
#include <QDir>

#ifdef Q_OS_WIN
#include <QSettings>
#endif

QString DefaultClientUtils::getMudletExecutablePath()
{
    QString path = QCoreApplication::applicationFilePath();
#ifdef Q_OS_WIN
    // Windows needs backslashes in registry paths
    path = path.replace("/", "\\");
#endif
    return path;
}

QString DefaultClientUtils::getCurrentTelnetHandler()
{
#ifdef Q_OS_LINUX
    return getCurrentTelnetHandlerLinux();
#elif defined(Q_OS_MACOS)
    return getCurrentTelnetHandlerMacOS();
#elif defined(Q_OS_WIN)
    return getCurrentTelnetHandlerWindows();
#else
    return QString();
#endif
}

bool DefaultClientUtils::setMudletAsDefaultTelnetHandler()
{
#ifdef Q_OS_LINUX
    return setMudletAsDefaultTelnetHandlerLinux();
#elif defined(Q_OS_MACOS)
    return setMudletAsDefaultTelnetHandlerMacOS();
#elif defined(Q_OS_WIN)
    return setMudletAsDefaultTelnetHandlerWindows();
#else
    return false;
#endif
}

bool DefaultClientUtils::isMudletDefaultTelnetHandler()
{
    QString currentHandler = getCurrentTelnetHandler();
    if (currentHandler.isEmpty()) {
        return false;
    }
    
    QString mudletPath = getMudletExecutablePath();
    return currentHandler.contains(mudletPath, Qt::CaseInsensitive);
}

#ifdef Q_OS_LINUX
QString DefaultClientUtils::getCurrentTelnetHandlerLinux()
{
    QProcess process;
    process.start("xdg-mime", QStringList() << "query" << "default" << "x-scheme-handler/telnet");
    if (!process.waitForFinished(2000)) {
        return QString();
    }
    
    QString result = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
    return result;
}

bool DefaultClientUtils::setMudletAsDefaultTelnetHandlerLinux()
{
    // First, ensure the desktop file is installed in the user's applications directory
    QString userAppsDir = QDir::homePath() + "/.local/share/applications";
    QDir().mkpath(userAppsDir);
    
    QString desktopFilePath = userAppsDir + "/mudlet.desktop";
    QString sourceDesktopFile = QCoreApplication::applicationDirPath() + "/../mudlet.desktop";
    
    // Check if source desktop file exists, if not try in current directory
    if (!QFile::exists(sourceDesktopFile)) {
        sourceDesktopFile = QCoreApplication::applicationDirPath() + "/mudlet.desktop";
    }
    
    // Copy desktop file to user's applications directory if it doesn't exist
    if (!QFile::exists(desktopFilePath) && QFile::exists(sourceDesktopFile)) {
        QFile::copy(sourceDesktopFile, desktopFilePath);
    }
    
    // Update MIME database
    QProcess updateDb;
    updateDb.start("update-desktop-database", QStringList() << userAppsDir);
    updateDb.waitForFinished(2000);
    
    // Set Mudlet as default handler for telnet://
    QProcess process;
    process.start("xdg-mime", QStringList() << "default" << "mudlet.desktop" << "x-scheme-handler/telnet");
    if (!process.waitForFinished(2000)) {
        return false;
    }
    
    return process.exitCode() == 0;
}
#endif

#ifdef Q_OS_MACOS
QString DefaultClientUtils::getCurrentTelnetHandlerMacOS()
{
    // Use swift to determine the default telnet handler
    QString swiftScript = R"(
import Foundation
import CoreServices

if let url = URL(string: "telnet://example.com"),
   let appUrl = LSCopyDefaultApplicationURLForURL(url as CFURL, .all, nil) {
    let bundleId = (appUrl.takeRetainedValue() as URL).lastPathComponent
    print(bundleId)
}
)";
    
    QProcess process;
    process.start("swift", QStringList() << "-");
    process.write(swiftScript.toUtf8());
    process.closeWriteChannel();
    
    if (!process.waitForFinished(2000)) {
        return QString();
    }
    
    QString result = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
    return result;
}

bool DefaultClientUtils::setMudletAsDefaultTelnetHandlerMacOS()
{
    // On macOS, this is typically handled by the Info.plist file
    // and the user needs to manually set default handlers
    // We can trigger the system dialog
    QProcess process;
    process.start("open", QStringList() << "-a" << QCoreApplication::applicationFilePath() << "telnet://trigger-default-handler");
    return process.waitForFinished(2000) && process.exitCode() == 0;
}
#endif

#ifdef Q_OS_WIN
QString DefaultClientUtils::getCurrentTelnetHandlerWindows()
{
    QSettings registrySettings("HKEY_CLASSES_ROOT\\telnet\\shell\\open\\command", QSettings::NativeFormat);
    QString handler = registrySettings.value("Default", QString()).toString();
    return handler;
}

bool DefaultClientUtils::setMudletAsDefaultTelnetHandlerWindows()
{
    QSettings registrySettings("HKEY_CLASSES_ROOT", QSettings::NativeFormat);
    
    QString mudletPath = getMudletExecutablePath();
    
    // Set up telnet:// protocol handler
    registrySettings.setValue("telnet/.", "URL:Telnet Protocol");
    registrySettings.setValue("telnet/URL Protocol", "");
    registrySettings.setValue("telnet/DefaultIcon/.", mudletPath + ",0");
    registrySettings.setValue("telnet/shell/open/command/.", QString("\"%1\" \"%2\"").arg(mudletPath).arg("%1"));
    
    // Also register mudlet:// protocol for future use
    registrySettings.setValue("mudlet/.", "URL:Mudlet Protocol");
    registrySettings.setValue("mudlet/URL Protocol", "");
    registrySettings.setValue("mudlet/DefaultIcon/.", mudletPath + ",0");
    registrySettings.setValue("mudlet/shell/open/command/.", QString("\"%1\" \"%2\"").arg(mudletPath).arg("%1"));
    
    return true;
}
#endif
