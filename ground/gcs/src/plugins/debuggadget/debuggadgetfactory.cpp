/**
 ******************************************************************************
 *
 * @file       debuggadgetfactory.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup DebugGadgetPlugin Debug Gadget Plugin
 * @{
 * @brief A place holder gadget plugin
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#include "debuggadgetfactory.h"
#include "debuggadgetwidget.h"
#include "debuggadget.h"
#include <coreplugin/iuavgadget.h>

void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context)
    switch (type) {
    case QtDebugMsg:
        debugengine::getInstance()->writeDebug(msg);
        break;
    case QtWarningMsg:
        debugengine::getInstance()->writeWarning(msg);
        break;
    case QtCriticalMsg:
        debugengine::getInstance()->writeCritical(msg);
        break;
    case QtFatalMsg:
        debugengine::getInstance()->writeFatal(msg);
    default:
        debugengine::getInstance()->writeDebug(msg);
    }
}

DebugGadgetFactory::DebugGadgetFactory(QObject *parent) :
    IUAVGadgetFactory(QString("DebugGadget"),
                      tr("DebugGadget"),
                      parent)
{}

DebugGadgetFactory::~DebugGadgetFactory()
{}

IUAVGadget *DebugGadgetFactory::createGadget(QWidget *parent)
{
    qInstallMessageHandler(customMessageHandler);

    DebugGadgetWidget *gadgetWidget = new DebugGadgetWidget(parent);

    return new DebugGadget(QString("DebugGadget"), gadgetWidget, parent);
}
