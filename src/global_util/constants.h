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
#include <QString>

#include <DSysInfo>

DCORE_USE_NAMESPACE

namespace DLauncher {

static const int APP_DRAG_ICON_SIZE = 48;
static const int APP_DRAG_SWAP_THRESHOLD = 80;
static const int APP_DRAG_SCROLL_THRESHOLD = 150;

static const int DRAG_THRESHOLD = 15;

static const int CATEGORY_TITLE_WIDGET_HEIGHT = 70;

static const int VIEWLIST_BOTTOM_MARGIN = 140;

static const int NAVIGATION_ICON_SIZE = 32;
static const int NAVIGATION_BUTTON_SIZE = 60;

static const int APPHBOX_SPACING = 60;

static const int APPHBOX_ALPHA = 100;

static const int TOP_BOTTOM_GRADIENT_HEIGHT = 60;

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

const QString SystemTypeName = DSysInfo::deepinTypeDisplayName(QLocale::c());

const DSysInfo::DeepinType DeepinType = DSysInfo::deepinType();
const bool IsServerSystem = (DSysInfo::DeepinServer == DeepinType);
const bool IsDesktopSystem = DSysInfo::isCommunityEdition();
const bool IsProfessionalSystem = (DSysInfo::DeepinProfessional == DeepinType);

static const int WHOOLTIME_TO_SCROOL = 10;

}

#endif // CONSTANTS_H
