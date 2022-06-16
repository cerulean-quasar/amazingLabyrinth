/**
 * Copyright 2022 Cerulean Quasar. All Rights Reserved.
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

import android.content.Intent;
import android.util.JsonReader;
import android.util.JsonWriter;

import androidx.databinding.BaseObservable;
import androidx.databinding.Bindable;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.nio.charset.StandardCharsets;

public class Settings extends BaseObservable {
    private static final String m_keyTryVulkan = "tryVulkan";
    private static final String m_keyUseShadows = "useShadows";

    private static final boolean m_tryVulkanDefault = true;
    private static final boolean m_useShadowsDefault = true;

    public class File {
        public void save(OutputStream file) {
            try {
                JsonWriter writer = new JsonWriter(new OutputStreamWriter(file, StandardCharsets.UTF_8));
                writer.setIndent(" ");
                writeSettings(writer);
                writer.close();
            } catch (IOException e) {
                // nothing to do
            }
        }

        public void open(InputStream file) {
            try {
                JsonReader reader = new JsonReader(new InputStreamReader(file, StandardCharsets.UTF_8));
                readSettings(reader);
                reader.close();
            } catch (IOException e) {
                // nothing to do
            }
        }

        private void writeSettings(JsonWriter writer) throws IOException {
            writer.beginObject();
            writer.name(m_keyTryVulkan).value(m_tryVulkan);
            writer.name(m_keyUseShadows).value(m_useShadows);
            writer.endObject();
        }

        private void readSettings(JsonReader reader) throws IOException {
            reader.beginObject();
            while (reader.hasNext()) {
                String name = reader.nextName();
                if (name.equals(m_keyTryVulkan)) {
                    m_tryVulkan = reader.nextBoolean();
                    m_tryVulkanReadFromFile = true;
                } else if (name.equals(m_keyUseShadows)) {
                    m_useShadows = reader.nextBoolean();
                } else {
                    reader.skipValue();
                }
            }
            reader.endObject();
        }
    }

    private boolean m_changesMade;
    private boolean m_tryVulkanReadFromFile;
    private boolean m_tryVulkan;
    private boolean m_useShadows;

    public Settings() {
        m_changesMade = false;
        m_tryVulkanReadFromFile = false;
        m_tryVulkan = m_tryVulkanDefault;
        m_useShadows = m_useShadowsDefault;
    }

    public Settings(boolean inTryVulkan, boolean inUseShadows) {
        m_changesMade = false;
        m_tryVulkanReadFromFile = false;
        m_tryVulkan = inTryVulkan;
        m_useShadows = inUseShadows;
    }

    public Settings(Intent intent) {
        m_changesMade = false;
        m_tryVulkanReadFromFile = false;
        m_tryVulkan = intent.getBooleanExtra(m_keyTryVulkan, m_tryVulkanDefault);
        m_useShadows = intent.getBooleanExtra(m_keyUseShadows, m_useShadowsDefault);
    }

    public void overrideFromIntent(Intent intent) {
        m_tryVulkan = intent.getBooleanExtra(m_keyTryVulkan, m_tryVulkanDefault);
        m_useShadows = intent.getBooleanExtra(m_keyUseShadows, m_useShadowsDefault);
    }

    public boolean haveChangesBeenMade() {
        return m_changesMade;
    }

    public void addToIntent(Intent intent) {
        intent.putExtra(m_keyTryVulkan, m_tryVulkan);
        intent.putExtra(m_keyUseShadows, m_useShadows);
    }

    public boolean isTryVulkanReadFromFile() {
        return m_tryVulkanReadFromFile;
    }

    @Bindable
    public Boolean getTryVulkan() {
        return m_tryVulkan;
    }

    public void setTryVulkan(Boolean inTryVulkan) {
        m_changesMade = true;
        m_tryVulkan = inTryVulkan;
    }

    @Bindable
    public  Boolean getUseShadows() { return m_useShadows; }

    public void setUseShadows(Boolean inUseShadows) {
        m_changesMade = true;
        m_useShadows = inUseShadows;
    }
}
