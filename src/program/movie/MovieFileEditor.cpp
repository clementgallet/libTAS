/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QSettings>
#include <QString>
#include <string>

#include "MovieFileEditor.h"

MovieFileEditor::MovieFileEditor(Context* c) : context(c) {}

void MovieFileEditor::load()
{
    /* Clear structures */
    input_set.clear();
    nondraw_frames.clear();
    
    /* Load the editor file */
    QString editorfile = context->config.tempmoviedir.c_str();
	editorfile += "/editor.ini";

	QSettings editor(editorfile, QSettings::IniFormat);
	editor.setFallbacksEnabled(false);
    
    int size = editor.beginReadArray("input_names");
    for (int i = 0; i < size; ++i) {
        editor.setArrayIndex(i);
        SingleInput si = editor.value("input").value<SingleInput>();
        std::string name = editor.value("name").toString().toStdString();
        si.description = name;
        input_set.push_back(si);
    }
    editor.endArray();

    size = editor.beginReadArray("nondraw_frames");
    for (int i = 0; i < size; ++i) {
        editor.setArrayIndex(i);
        uint64_t f = static_cast<uint64_t>(editor.value("frame").toULongLong());
        nondraw_frames.insert(f);
    }
    editor.endArray();
    
    /* If we found something, return. Else, try the old location into config.ini */
    if (!input_set.empty())
        return;

    QString configfile = context->config.tempmoviedir.c_str();
    configfile += "/config.ini";

	QSettings config(configfile, QSettings::IniFormat);
	config.setFallbacksEnabled(false);

	size = config.beginReadArray("input_names");
    for (int i = 0; i < size; ++i) {
        config.setArrayIndex(i);
        SingleInput si = config.value("input").value<SingleInput>();
        std::string name = config.value("name").toString().toStdString();
        si.description = name;
        input_set.push_back(si);
    }
    config.endArray();
}

void MovieFileEditor::save()
{
    /* Save some parameters into the editor file */
    QString editorfile = context->config.tempmoviedir.c_str();
	editorfile += "/editor.ini";

	QSettings config(editorfile, QSettings::IniFormat);
	config.setFallbacksEnabled(false);

	config.remove("input_names");
    config.beginWriteArray("input_names");
    int i = 0;
    for (const SingleInput& si : input_set) {
        config.setArrayIndex(i++);
        config.setValue("input", QVariant::fromValue(si));
        config.setValue("name", si.description.c_str());
    }
    config.endArray();

    config.remove("nondraw_frames");
    config.beginWriteArray("nondraw_frames");
    i = 0;
    for (uint64_t f : nondraw_frames) {
        config.setArrayIndex(i++);
        config.setValue("frame", static_cast<unsigned long long>(f));
    }
    config.endArray();

    config.sync();
}

void MovieFileEditor::setLockedInputs(AllInputs& inputs, const AllInputs& movie_inputs)
{
	if (locked_inputs.empty())
		return;

	for (SingleInput si : locked_inputs) {
		int value = movie_inputs.getInput(si);
		inputs.setInput(si, value);
	}
}

void MovieFileEditor::setDraw(bool draw)
{
    if (!draw) {
        nondraw_frames.insert(context->framecount);
    }
    else {
        nondraw_frames.erase(context->framecount);
    }
}

bool MovieFileEditor::isDraw(uint64_t frame)
{
    return !nondraw_frames.count(frame);
}

void MovieFileEditor::close()
{
	locked_inputs.clear();
}
