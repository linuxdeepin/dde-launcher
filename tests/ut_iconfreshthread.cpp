/*
 * Copyright (C) 2018 ~ 2021 Uniontech Technology Co., Ltd.
 *
 * Author:     songwentao <songwentao@uniontech.com>
 *
 * Maintainer: songwentao <songwentao@uniontech.com>
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

#define private public
#include "iconfreshthread.h"
#undef private

#include <QEventLoop>
#include <QTimer>

#include <gtest/gtest.h>

class Tst_IconFreshThread : public testing::Test
{
};

TEST_F(Tst_IconFreshThread, function_test)
{
    IconFreshThread thread;
    thread.getThreadState();

    thread.refreshIcon();

    ItemInfo item;
    item.m_key = "dde-calendar";
    item.m_name = "dde-calendar";
    thread.createPixmap(item, 16);

    QQueue<ItemInfo> itemQueue;
    itemQueue.append(item);
    thread.setQueue(itemQueue);

    QEventLoop loop;
    thread.start();

    QTimer::singleShot(2000, [ & ]() {
        loop.exit();
    });

    loop.exec();

    thread.releaseThread();
    thread.releaseInstallAppSemo();
    thread.releaseReloadAppSemo();
    thread.releaseSemo();
}




