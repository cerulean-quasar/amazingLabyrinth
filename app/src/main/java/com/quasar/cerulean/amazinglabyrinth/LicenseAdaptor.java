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

import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.LineNumberReader;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;

public class LicenseAdaptor extends RecyclerView.Adapter<RecyclerView.ViewHolder> {
    private static final int TEXT_TYPE = 1;

    private ArrayList<String> m_paragraphs;
    private LineNumberReader m_reader;
    private boolean m_finishedReading;

    private class ViewHolderText extends RecyclerView.ViewHolder {
        public TextView m_text;
        public ViewHolderText(TextView view) {
            super(view);
            m_text = view;
        }
    }

    LicenseAdaptor(InputStream inputStream) {
        m_reader = new LineNumberReader(new InputStreamReader(inputStream, StandardCharsets.UTF_8));
        m_paragraphs = new ArrayList<>();
        m_finishedReading = false;
        while (readParagraph())
            /* do nothing */;
    }

    @Override
    public int getItemViewType(int position) {
        return TEXT_TYPE;
    }

    @Override @NonNull
    public RecyclerView.ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        TextView text = new TextView(parent.getContext());
        text.setPadding(0, 10, 0, 10);
        return new ViewHolderText(text);
    }

    @Override
    public int getItemCount() {
        return m_paragraphs.size();
    }

    @Override
    public void onBindViewHolder(@NonNull RecyclerView.ViewHolder vh, int position) {
        ((ViewHolderText)vh).m_text.setText(m_paragraphs.get(position));

        /*
        if (position == m_paragraphs.size() -1 && readParagraph()) {
            int nbrItems = m_paragraphs.size();
            notifyItemInserted(nbrItems - 2);
            notifyItemMoved(nbrItems - 2, nbrItems - 1);
        }*/
    }

    private boolean readParagraph() {
        boolean close = false;
        boolean ret = false;
        try {
            StringBuilder builder = new StringBuilder();
            String next;
            do {
                next = m_reader.readLine();
                if (next == null) {
                    if (builder.length() > 0) {
                        m_paragraphs.add(builder.toString());
                        ret = true;
                    }
                    close = true;
                    m_finishedReading = true;
                } else {
                    if (next.isEmpty()) {
                        m_paragraphs.add(builder.toString());
                        ret = true;
                        break;
                    } else {
                        if (builder.length() > 0) {
                            builder.append(' ');
                        }

                        builder.append(next);
                    }
                }
            } while (next != null);
        } catch (IOException e) {
            close = true;
        }

        if (close) {
            try {
                m_finishedReading = true;
                m_reader.close();
            } catch (IOException e) {
            }
        }

        return ret;
    }
}
