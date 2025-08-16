#ifndef MUDLET_DLGMXPRECOVERY_H
#define MUDLET_DLGMXPRECOVERY_H

/***************************************************************************
 *   Copyright (C) 2025 by Rishi Mondal - mavrickrishi@gmail.com          *
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

#include "pre_guard.h"
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "post_guard.h"

class Host;

class dlgMxpRecovery : public QDialog
{
    Q_OBJECT

public:
    explicit dlgMxpRecovery(Host* pHost, const QString& problematicContent, QWidget* parent = nullptr);
    
    enum RecoveryAction {
        DisableMXP,
        ContinueWithMXP,
        DismissDialog
    };
    
    RecoveryAction getSelectedAction() const { return mSelectedAction; }

private slots:
    void slot_disableMXP();
    void slot_continueWithMXP();
    void slot_dismiss();

private:
    void setupUi();
    
    Host* mpHost;
    QString mProblematicContent;
    RecoveryAction mSelectedAction;
    
    QLabel* mpMessageLabel;
    QLabel* mpContentLabel;
    QPushButton* mpDisableButton;
    QPushButton* mpContinueButton;
    QPushButton* mpDismissButton;
};

#endif // MUDLET_DLGMXPRECOVERY_H
