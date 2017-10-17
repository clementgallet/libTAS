/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "RamSearchWindow.h"
// #include "MainWindow.h"
#include <iostream>
#include <sstream>
// #include <X11/XKBlib.h>
// #include <FL/names.h>
// #include <FL/x.H>
#include "../ramsearch/CompareEnums.h"

static Fl_Callback new_cb;
static Fl_Callback search_cb;

RamSearchWindow::RamSearchWindow(Context* c) : context(c)
{
    window = new Fl_Double_Window(800, 700, "Ram Search");

    /* Browsers */
    address_browser = new Fl_Multi_Browser(10, 10, 480, 630, "");
    // address_browser->callback(select_cb, this);

    /* Set three columns */
    static int col_width[] = {160, 160, 160, 0};
    address_browser->column_widths(col_width);
    address_browser->column_char('\t');

    watch_count = new Fl_Box(10, 640, 480, 30);
    watch_count->box(FL_NO_BOX);
    watch_count->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    config_pack = new Fl_Pack(510, 50, 280, 680, "");
    config_pack->type(Fl_Pack::VERTICAL);
    config_pack->spacing(50);

    mem_pack = new Fl_Pack(0, 0, 280, 160, "Included Memory Regions");
    mem_pack->type(Fl_Pack::HORIZONTAL);

    mem_col1 = new Fl_Pack(0, 0, 140, 160, "");
    mem_col1->type(Fl_Pack::VERTICAL);

    mem_text = new Fl_Check_Button(0, 0, 140, 30, "Text");
    mem_data_ro = new Fl_Check_Button(0, 0, 140, 30, "RO Data");
    mem_data_rw = new Fl_Check_Button(0, 0, 140, 30, "RW Data");
    mem_bss = new Fl_Check_Button(0, 0, 140, 30, "BSS");
    mem_heap = new Fl_Check_Button(0, 0, 140, 30, "Heap");

    mem_col1->end();
    mem_col2 = new Fl_Pack(0, 0, 140, 200, "");
    mem_col2->type(Fl_Pack::VERTICAL);

    mem_file_mapping = new Fl_Check_Button(0, 0, 140, 30, "File Mapping");
    mem_anonymous_mapping_ro = new Fl_Check_Button(0, 0, 140, 30, "Anon RO Mapping");
    mem_anonymous_mapping_rw = new Fl_Check_Button(0, 0, 140, 30, "Anon RW Mapping");
    mem_stack = new Fl_Check_Button(0, 0, 140, 30, "Stack");
    mem_special = new Fl_Check_Button(0, 0, 140, 30, "Special");

    mem_col2->end();
    mem_pack->end();

    compare_pack = new Fl_Pack(0, 0, 280, 100, "Compare To / By");
    compare_pack->type(Fl_Pack::VERTICAL);

    compare_previous = new Fl_Radio_Round_Button(0, 0, 280, 30, "Previous Value");
    compare_previous->set();
    compare_value = new Fl_Radio_Round_Button(0, 0, 280, 30, "Specific Value");

    compare_pack->end();

    operator_pack = new Fl_Pack(0, 0, 280, 200, "Comparison Operator");
    operator_pack->type(Fl_Pack::VERTICAL);

    operator_equal = new Fl_Radio_Round_Button(0, 0, 280, 30, "Equal To");
    operator_equal->set();
    operator_not_equal = new Fl_Radio_Round_Button(0, 0, 280, 30, "Not Equal To");
    operator_less = new Fl_Radio_Round_Button(0, 0, 280, 30, "Less Than");
    operator_greater = new Fl_Radio_Round_Button(0, 0, 280, 30, "Greater Than");
    operator_less_equal = new Fl_Radio_Round_Button(0, 0, 280, 30, "Less Than Or Equal To");
    operator_greater_equal = new Fl_Radio_Round_Button(0, 0, 280, 30, "Greater Than Or Equal To");

    operator_pack->end();

    type_choice = new Fl_Choice(0, 0, 280, 30, "Value Type");
    type_choice->align(FL_ALIGN_TOP_LEFT);
    type_choice->menu(type_items);
    // type_choice->callback(vcodec_cb, this);

    config_pack->end();

    new_button = new Fl_Button(510, 650, 70, 30, "New");
    new_button->callback(new_cb, this);

    search_button = new Fl_Button(600, 650, 70, 30, "Search");
    search_button->callback(search_cb, this);

    window->end();
}

Fl_Menu_Item RamSearchWindow::type_items[] = {
    {"unsigned char"},
    {"char"},
    {"unsigned short"},
    {"short"},
    {"unsigned int"},
    {"int"},
    {"unsigned int64"},
    {"int64"},
    {"float"},
    {"double"},
    {nullptr}
};

void RamSearchWindow::update()
{
    if (address_browser->size() > 1000)
        return;

    int i = 1;
    for (auto &ramwatch : ram_search.ramwatches) {
        address_browser->text(i++, ramwatch->get_line().c_str());
    }
}

static void new_cb(Fl_Widget* w, void* v)
{
    RamSearchWindow* rsw = (RamSearchWindow*) v;

    /* Build the memory region flag variable */
    int memregions = 0;
    if (rsw->mem_text->value())
        memregions |= MemSection::MemText;
    if (rsw->mem_data_ro->value())
        memregions |= MemSection::MemDataRO;
    if (rsw->mem_data_rw->value())
        memregions |= MemSection::MemDataRW;
    if (rsw->mem_bss->value())
        memregions |= MemSection::MemBSS;
    if (rsw->mem_heap->value())
        memregions |= MemSection::MemHeap;
    if (rsw->mem_file_mapping->value())
        memregions |= MemSection::MemFileMapping;
    if (rsw->mem_anonymous_mapping_ro->value())
        memregions |= MemSection::MemAnonymousMappingRO;
    if (rsw->mem_anonymous_mapping_rw->value())
        memregions |= MemSection::MemAnonymousMappingRW;
    if (rsw->mem_stack->value())
        memregions |= MemSection::MemStack;
    if (rsw->mem_special->value())
        memregions |= MemSection::MemSpecial;

    /* Call the RamSearch new function using the right type as template */
    switch (rsw->type_choice->value()) {
        case 0:
            rsw->ram_search.new_watches<unsigned char>(rsw->context->game_pid, memregions);
            break;
        case 1:
            rsw->ram_search.new_watches<char>(rsw->context->game_pid, memregions);
            break;
        case 2:
            rsw->ram_search.new_watches<unsigned short>(rsw->context->game_pid, memregions);
            break;
        case 3:
            rsw->ram_search.new_watches<short>(rsw->context->game_pid, memregions);
            break;
        case 4:
            rsw->ram_search.new_watches<unsigned int>(rsw->context->game_pid, memregions);
            break;
        case 5:
            rsw->ram_search.new_watches<int>(rsw->context->game_pid, memregions);
            break;
        case 6:
            rsw->ram_search.new_watches<int64_t>(rsw->context->game_pid, memregions);
            break;
        case 7:
            rsw->ram_search.new_watches<uint64_t>(rsw->context->game_pid, memregions);
            break;
        case 8:
            rsw->ram_search.new_watches<float>(rsw->context->game_pid, memregions);
            break;
        case 9:
            rsw->ram_search.new_watches<double>(rsw->context->game_pid, memregions);
            break;
    }

    rsw->address_browser->clear();
    for (auto &ramwatch : rsw->ram_search.ramwatches) {
        rsw->address_browser->add(ramwatch->get_line_update().c_str());
    }

    std::ostringstream oss;
    oss << rsw->address_browser->size();
    oss << " results";
    if (rsw->address_browser->size() > 1000) {
        oss << " (not updating because too many entries)";
    }

    rsw->watch_count->copy_label(oss.str().c_str());
}

static void search_cb(Fl_Widget* w, void* v)
{
    RamSearchWindow* rsw = (RamSearchWindow*) v;

    CompareType compare_type = CompareType::Previous;
    if (rsw->compare_value->value())
        compare_type = CompareType::Value;

    CompareOperator compare_operator = CompareOperator::Equal;
    if (rsw->operator_not_equal->value())
        compare_operator = CompareOperator::NotEqual;
    if (rsw->operator_less->value())
        compare_operator = CompareOperator::Less;
    if (rsw->operator_greater->value())
        compare_operator = CompareOperator::Greater;
    if (rsw->operator_less_equal->value())
        compare_operator = CompareOperator::LessEqual;
    if (rsw->operator_greater_equal->value())
        compare_operator = CompareOperator::GreaterEqual;

    rsw->ram_search.search_watches(compare_type, compare_operator, 0);

    rsw->address_browser->clear();
    for (auto &ramwatch : rsw->ram_search.ramwatches) {
        rsw->address_browser->add(ramwatch->get_line().c_str());
    }

    std::ostringstream oss;
    oss << "There are ";
    oss << rsw->address_browser->size();
    oss << " adresses";
    if (rsw->address_browser->size() > 1000) {
        oss << " (not updating because too many entries)";
    }

    rsw->watch_count->copy_label(oss.str().c_str());
}