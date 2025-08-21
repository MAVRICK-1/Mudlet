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

#ifndef DEFAULTCLIENTUTILS_H
#define DEFAULTCLIENTUTILS_H

#include <QString>

class DefaultClientUtils
{
public:
    // Get the current default telnet:// handler application
    static QString getCurrentTelnetHandler();
    
    // Set Mudlet as the default telnet:// handler
    static bool setMudletAsDefaultTelnetHandler();
    
    // Check if Mudlet is the default telnet:// handler
    static bool isMudletDefaultTelnetHandler();
    
    // Get the Mudlet executable path appropriate for URI registration
    static QString getMudletExecutablePath();
    
private:
    // Platform-specific implementation methods
#ifdef Q_OS_LINUX
    static QString getCurrentTelnetHandlerLinux();
    static bool setMudletAsDefaultTelnetHandlerLinux();
#endif
    
#ifdef Q_OS_MACOS
    static QString getCurrentTelnetHandlerMacOS();
    static bool setMudletAsDefaultTelnetHandlerMacOS();
#endif
    
#ifdef Q_OS_WIN
    static QString getCurrentTelnetHandlerWindows();
    static bool setMudletAsDefaultTelnetHandlerWindows();
#endif
};

#endif // DEFAULTCLIENTUTILS_H
