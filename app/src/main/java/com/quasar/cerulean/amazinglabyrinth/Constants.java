/**
 * Copyright 2020 Cerulean Quasar. All Rights Reserved.
 *
 *  This file is part of AmazingLabyrinth.
 *
 *  AmazingLabyrinth is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  AmazingLabyrinth is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with AmazingLabyrinth.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

package com.quasar.cerulean.amazinglabyrinth;

public class Constants {
    public static final String KeyError = "errorString";
    public static final String KeyEnableKeepAlive = "keepAlive";

    // for About Activity
    public static final int AMAZING_LABYRINTH_ABOUT_ACTIVITY = 1;
    public static final String KeyGraphicsName = "graphicsName";
    public static final String KeyDeviceName = "deviceName";
    public static final String KeyVersionName = "versionName";
    public static final String KeyHasAccelerometer = "hasAccelerometer";
    public static final String KeyBugInfo = "bugInfo";

    // for Show License Activity
    public static final int AMAZING_LABYRINTH_LICENSE_ACTIVITY = 2;
    public static final String KeyLicenseFile = "licenseFile";

    // for Choose Level Activity
    public static final int AMAZING_LABYRINTH_CHOOSE_LEVEL_ACTIVITY = 3;
    public static final String KeySelectedLevel = "selectedLevel";
    public static final String KeyLevels = "levels";

    // File names
    public static final String LevelTableFileName = "configs/levels.json";
}
