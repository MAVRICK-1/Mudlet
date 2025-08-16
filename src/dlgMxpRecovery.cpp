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

#include "dlgMxpRecovery.h"
#include "Host.h"

#include "pre_guard.h"
#include <QApplication>
#include <QIcon>
#include <QPixmap>
#include <QStyleOption>
#include "post_guard.h"

dlgMxpRecovery::dlgMxpRecovery(Host* pHost, const QString& problematicContent, QWidget* parent)
: QDialog(parent)
, mpHost(pHost)
, mProblematicContent(problematicContent)
, mSelectedAction(DismissDialog)
, mpMessageLabel(nullptr)
, mpContentLabel(nullptr)
, mpDisableButton(nullptr)
, mpContinueButton(nullptr)
, mpDismissButton(nullptr)
{
    setupUi();
    setModal(true);
    setWindowTitle(tr("MXP Parsing Issue Detected"));
    resize(500, 200);
}

void dlgMxpRecovery::setupUi()
{
    auto* pMainLayout = new QVBoxLayout(this);
    
    // Warning icon and main message
    auto* pHeaderLayout = new QHBoxLayout();
    
    // Warning icon
    auto* pIconLabel = new QLabel();
    QIcon warningIcon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning);
    pIconLabel->setPixmap(warningIcon.pixmap(32, 32));
    pIconLabel->setAlignment(Qt::AlignTop);
    pHeaderLayout->addWidget(pIconLabel);
    
    // Message text
    mpMessageLabel = new QLabel();
    mpMessageLabel->setText(tr("Mudlet has detected an issue with MXP (Mud eXtension Protocol) parsing that may cause the game output to freeze.\n\n"
                              "This typically occurs when the game sends improperly formatted MXP tags with unescaped characters like '<', '>', or '&'."));
    mpMessageLabel->setWordWrap(true);
    mpMessageLabel->setAlignment(Qt::AlignTop);
    pHeaderLayout->addWidget(mpMessageLabel, 1);
    
    pMainLayout->addLayout(pHeaderLayout);
    
    // Problematic content
    if (!mProblematicContent.isEmpty()) {
        auto* pContentGroup = new QLabel(tr("Problematic content:"));
        pContentGroup->setStyleSheet("font-weight: bold; margin-top: 10px;");
        pMainLayout->addWidget(pContentGroup);
        
        mpContentLabel = new QLabel();
        mpContentLabel->setText(QString("\"") + mProblematicContent.toHtmlEscaped() + QString("\""));
        mpContentLabel->setStyleSheet("font-family: monospace; background-color: #f0f0f0; padding: 5px; border: 1px solid #ccc;");
        mpContentLabel->setWordWrap(true);
        pMainLayout->addWidget(mpContentLabel);
    }
    
    // Buttons
    auto* pButtonLayout = new QHBoxLayout();
    
    mpDisableButton = new QPushButton(tr("&Disable MXP"));
    mpDisableButton->setToolTip(tr("Disable MXP processing to prevent further issues. You can re-enable it later in Settings."));
    connect(mpDisableButton, &QPushButton::clicked, this, &dlgMxpRecovery::slot_disableMXP);
    pButtonLayout->addWidget(mpDisableButton);
    
    mpContinueButton = new QPushButton(tr("&Continue with MXP"));
    mpContinueButton->setToolTip(tr("Continue using MXP processing. The issue may persist and cause display problems."));
    connect(mpContinueButton, &QPushButton::clicked, this, &dlgMxpRecovery::slot_continueWithMXP);
    pButtonLayout->addWidget(mpContinueButton);
    
    mpDismissButton = new QPushButton(tr("&Not Now"));
    mpDismissButton->setToolTip(tr("Dismiss this dialog without making changes. The issue may continue."));
    connect(mpDismissButton, &QPushButton::clicked, this, &dlgMxpRecovery::slot_dismiss);
    pButtonLayout->addWidget(mpDismissButton);
    
    pMainLayout->addLayout(pButtonLayout);
    
    // Set default button
    mpDisableButton->setDefault(true);
    mpDisableButton->setFocus();
}

void dlgMxpRecovery::slot_disableMXP()
{
    mSelectedAction = DisableMXP;
    accept();
}

void dlgMxpRecovery::slot_continueWithMXP()
{
    mSelectedAction = ContinueWithMXP;
    accept();
}

void dlgMxpRecovery::slot_dismiss()
{
    mSelectedAction = DismissDialog;
    reject();
}