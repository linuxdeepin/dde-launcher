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

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QtGlobal>

namespace DLauncher {

static const int APP_ITEM_ICON_SIZE = 24;                                            // 小窗口应用图标大小
static const int APP_CATEGORY_ICON_SIZE = 18;                                        // 小窗口分类项图标大小
static const int APP_DRAG_ICON_SIZE = 48;
static const int APP_DRAG_SWAP_THRESHOLD = 10;
static const int APP_DRAG_SCROLL_THRESHOLD = 150;
static const int APP_DRAG_MININUM_TIME = 300;

static const int DRAG_THRESHOLD = 15;

static const int CATEGORY_TITLE_WIDGET_HEIGHT = 70;

static const int VIEWLIST_BOTTOM_MARGIN = 140;
static const int PAGEVIEW_BOTTOM_MARGIN = 70;

static const int NAVIGATION_ICON_SIZE = 32;
static const int NAVIGATION_BUTTON_SIZE = 60;

static const int APPHBOX_SPACING = 60;

static const int APPHBOX_ALPHA = 30;

static const int APPHBOX_RADIUS = 36;

static const int TOP_BOTTOM_GRADIENT_HEIGHT = 30;

static const int APPS_AREA_TOP_MARGIN = 20;
static const int APPS_AREA_BOTTOM_MARGIN = 20;
static const int APPS_AREA_ENSURE_VISIBLE_MARGIN_Y = 200;

#ifndef DISABLE_DRAG_ANIMATION
static const int APPS_AREA_AUTO_SCROLL_STEP = 5;
#else
// 130 is app item fix size
static const int APPS_AREA_AUTO_SCROLL_STEP = 130;
#endif

static const int APPS_AREA_AUTO_SCROLL_TIMER = 15;

static const int CLEAR_CACHE_TIMER = 60;

static const int WHOOLTIME_TO_SCROOL = 1;

static const int APPS_AREA_CATEGORY_INDEX = 255;

static const int APPS_CATEGORY_TITLE_SPACING = 60;

static const int MOUSE_PRESS_TIME_DIFF = 600;

static const int MOUSE_MOVE_TO_NEXT= 100;

static const int TOUCH_DIFF_THRESH = 15;

static const int SLIDE_DIFF_THRESH = 15;                // 按住鼠标滑动翻页的最小间距

static const int MINI_FRAME_LAYOUT_SPACE1 = 10;         // 小窗口布局间隔距离1
static const int MINI_FRAME_LAYOUT_SPACE2 = 8;          // 小窗口布局间隔距离2

static const qreal SIDES_SPACE_SCALE = 0.10;            // 全屏模式下设置左右边距的系数

static const int DOCK_POS_TOP = 0;                      // 任务栏位置: 顶部
static const int DOCK_POS_RIGHT = 1;                    // 任务栏位置: 右边
static const int DOCK_POS_BOTTOM = 2;                   // 任务栏位置: 底部
static const int DOCK_POS_LEFT = 3;                     // 任务栏位置: 左边
}

#endif // CONSTANTS_H
