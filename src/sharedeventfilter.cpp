/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "sharedeventfilter.h"

#include <QProcess>
#include <QDebug>

/*! \class SharedEventFilter
 *
 * FullscreenFrame and MiniFrame shared event handler.
 *
 */

SharedEventFilter::SharedEventFilter(QObject *parent)
    :QObject(parent)
{
    m_frame = dynamic_cast<LauncherInterface*>(parent);

    Q_ASSERT_X(m_frame, "SharedEventFilter",
               "SharedEventFilter is supposed used together only with LauncherInterface instances.");
}

bool SharedEventFilter::eventFilter(QObject *watched, QEvent *event)
{
//    if (watched == parent())
//    {
//        if (event->type() == QEvent::WindowDeactivate && m_frame->visible())
//            return m_frame->windowDeactiveEvent();
//    }

    if (event->type() == QEvent::KeyPress)
        return handleKeyEvent(static_cast<QKeyEvent *>(event));

    return false;
}

bool SharedEventFilter::handleKeyEvent(QKeyEvent *e)
{
    bool ctrlPressed = e->modifiers() & Qt::ControlModifier;
    switch (e->key())
    {
    case Qt::Key_F1:            QProcess::startDetached("dman dde");            return true;
    case Qt::Key_Enter:
    case Qt::Key_Return:        m_frame->launchCurrentApp();                    return true;
    case Qt::Key_Escape:        m_frame->hideLauncher();                        return true;
    case Qt::Key_Space:
    case Qt::Key_Tab:
    case Qt::Key_Backtab:
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_Left:
    case Qt::Key_Right:         m_frame->moveCurrentSelectApp(e->key());        return true;
    case Qt::Key_Backspace:     m_frame->appendToSearchEdit(-1);                return true;
    }

    // handle normal keys
    if ((e->key() <= Qt::Key_Z && e->key() >= Qt::Key_A) ||
        (e->key() <= Qt::Key_9 && e->key() >= Qt::Key_0))
    {
        // handle the emacs key bindings
        if(ctrlPressed) {
            switch (e->key()) {
            case Qt::Key_P:
                m_frame->moveCurrentSelectApp(Qt::Key_Up);
                return true;
            case Qt::Key_N:
                m_frame->moveCurrentSelectApp(Qt::Key_Down);
                return true;
            case Qt::Key_F:
                m_frame->moveCurrentSelectApp(Qt::Key_Right);
                return true;
            case Qt::Key_B:
                m_frame->moveCurrentSelectApp(Qt::Key_Left);
                return true;
            default:
                return false;
            }
        }

        m_frame->appendToSearchEdit(e->text()[0].toLatin1());

        return true;
    }

    return false;
}


