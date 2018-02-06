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
#include "MainWindow.h"
#include <iostream>
#include <sstream>
#include <algorithm> // std::remove_if
// #include <X11/XKBlib.h>
// #include <FL/names.h>
// #include <FL/x.H>
#include "../ramsearch/CompareEnums.h"

static Fl_Callback new_cb;
static Fl_Callback search_cb;
static Fl_Callback add_cb;

RamSearchWindow::RamSearchWindow(Context* c) : context(c)
{
    window = new Fl_Double_Window(800, 700, "Ram Search");

    /* Table */
    address_table = new RamSearchTable(&ram_search.ramwatches, 10, 10, 480, 630, "");

    /* Progress bar */
    search_progress = new Fl_Hor_Fill_Slider(10, 650, 480, 10);
    search_progress->hide();
    search_progress->selection_color(FL_BLUE);
    search_progress->box(FL_THIN_DOWN_FRAME);
    search_progress->slider(FL_FLAT_BOX);

    watch_count = new Fl_Box(10, 670, 480, 30);
    watch_count->box(FL_NO_BOX);
    watch_count->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    config_pack = new Fl_Pack(510, 50, 280, 680, "");
    config_pack->type(Fl_Pack::VERTICAL);
    config_pack->spacing(30);

    mem_pack = new Fl_Pack(0, 0, 280, 160, "Included Memory Regions");
    mem_pack->type(Fl_Pack::HORIZONTAL);
    mem_pack->box(FL_ENGRAVED_FRAME);

    mem_col1 = new Fl_Pack(0, 0, 120, 160, "");
    mem_col1->type(Fl_Pack::VERTICAL);

    mem_text = new Fl_Check_Button(0, 0, 120, 30, "Text");
    mem_data_ro = new Fl_Check_Button(0, 0, 120, 30, "RO Data");
    mem_data_rw = new Fl_Check_Button(0, 0, 120, 30, "RW Data");
    mem_bss = new Fl_Check_Button(0, 0, 120, 30, "BSS");
    mem_heap = new Fl_Check_Button(0, 0, 120, 30, "Heap");

    mem_col1->end();
    mem_col2 = new Fl_Pack(0, 0, 155, 160, "");
    mem_col2->type(Fl_Pack::VERTICAL);

    mem_file_mapping = new Fl_Check_Button(0, 0, 155, 30, "File Mapping");
    mem_anonymous_mapping_ro = new Fl_Check_Button(0, 0, 155, 30, "Anon RO Mapping");
    mem_anonymous_mapping_rw = new Fl_Check_Button(0, 0, 155, 30, "Anon RW Mapping");
    mem_stack = new Fl_Check_Button(0, 0, 155, 30, "Stack");
    mem_special = new Fl_Check_Button(0, 0, 155, 30, "Special");

    mem_col2->end();
    mem_pack->end();

    compare_pack = new Fl_Pack(0, 0, 280, 100, "Compare To");
    compare_pack->type(Fl_Pack::VERTICAL);
    compare_pack->box(FL_ENGRAVED_FRAME);

    compare_previous = new Fl_Radio_Round_Button(0, 0, 280, 30, "Previous Value");
    compare_previous->set();

    compare_value = new Fl_Radio_Round_Button(0, 0, 280, 30, "Specific Value:");
    comparing_value = new Fl_Float_Input(0, 0, 280, 30);

    compare_pack->end();

    operator_pack = new Fl_Pack(0, 0, 280, 200, "Comparison Operator");
    operator_pack->type(Fl_Pack::VERTICAL);
    operator_pack->align(FL_ALIGN_TOP_LEFT);
    operator_pack->box(FL_ENGRAVED_FRAME);

    operator_equal = new Fl_Radio_Round_Button(0, 0, 280, 30, "Equal To");
    operator_equal->set();
    operator_not_equal = new Fl_Radio_Round_Button(0, 0, 280, 30, "Not Equal To");
    operator_less = new Fl_Radio_Round_Button(0, 0, 280, 30, "Less Than");
    operator_greater = new Fl_Radio_Round_Button(0, 0, 280, 30, "Greater Than");
    operator_less_equal = new Fl_Radio_Round_Button(0, 0, 280, 30, "Less Than Or Equal To");
    operator_greater_equal = new Fl_Radio_Round_Button(0, 0, 280, 30, "Greater Than Or Equal To");

    operator_pack->end();

    type_pack = new Fl_Pack(0, 0, 280, 30);
    type_pack->type(Fl_Pack::HORIZONTAL);

    type_choice = new Fl_Choice(0, 0, 160, 30, "Type");
    type_choice->align(FL_ALIGN_TOP_LEFT);
    type_choice->menu(type_items);

    display_choice = new Fl_Choice(0, 0, 120, 30, "Display");
    display_choice->align(FL_ALIGN_TOP_LEFT);
    display_choice->menu(display_items);

    type_pack->end();

    config_pack->end();

    new_button = new Fl_Button(510, 650, 70, 30, "New");
    new_button->callback(new_cb, this);

    search_button = new Fl_Button(600, 650, 70, 30, "Search");
    search_button->callback(search_cb, this);

    add_button = new Fl_Button(690, 650, 90, 30, "Add Watch");
    add_button->callback(add_cb, this);

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

Fl_Menu_Item RamSearchWindow::display_items[] = {
    {"Decimal"},
    {"Hexadecimal"},
    {nullptr}
};

void RamSearchWindow::update()
{
    /* This is just to trigger a table redraw */
    address_table->cols(3);
}

static void get_compare_parameters(RamSearchWindow* rsw, CompareType& compare_type, CompareOperator& compare_operator, double& compare_value)
{
    compare_type = CompareType::Previous;
    if (rsw->compare_value->value()) {
        compare_type = CompareType::Value;
        compare_value = strtod(rsw->comparing_value->value(), nullptr);
    }

    compare_operator = CompareOperator::Equal;
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
}

static void new_cb(Fl_Widget* w, void* v)
{
    RamSearchWindow* rsw = static_cast<RamSearchWindow*>(v);

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

    /* Get the comparison parameters */
    CompareType compare_type;
    CompareOperator compare_operator;
    double compare_value;
    get_compare_parameters(rsw, compare_type, compare_operator, compare_value);

    /* Call the RamSearch new function using the right type as template */
    switch (rsw->type_choice->value()) {
        case 0:
            rsw->ram_search.new_watches<unsigned char>(rsw->context->game_pid, memregions, compare_type, compare_operator, compare_value, rsw->search_progress);
            break;
        case 1:
            rsw->ram_search.new_watches<char>(rsw->context->game_pid, memregions, compare_type, compare_operator, compare_value, rsw->search_progress);
            break;
        case 2:
            rsw->ram_search.new_watches<unsigned short>(rsw->context->game_pid, memregions, compare_type, compare_operator, compare_value, rsw->search_progress);
            break;
        case 3:
            rsw->ram_search.new_watches<short>(rsw->context->game_pid, memregions, compare_type, compare_operator, compare_value, rsw->search_progress);
            break;
        case 4:
            rsw->ram_search.new_watches<unsigned int>(rsw->context->game_pid, memregions, compare_type, compare_operator, compare_value, rsw->search_progress);
            break;
        case 5:
            rsw->ram_search.new_watches<int>(rsw->context->game_pid, memregions, compare_type, compare_operator, compare_value, rsw->search_progress);
            break;
        case 6:
            rsw->ram_search.new_watches<int64_t>(rsw->context->game_pid, memregions, compare_type, compare_operator, compare_value, rsw->search_progress);
            break;
        case 7:
            rsw->ram_search.new_watches<uint64_t>(rsw->context->game_pid, memregions, compare_type, compare_operator, compare_value, rsw->search_progress);
            break;
        case 8:
            rsw->ram_search.new_watches<float>(rsw->context->game_pid, memregions, compare_type, compare_operator, compare_value, rsw->search_progress);
            break;
        case 9:
            rsw->ram_search.new_watches<double>(rsw->context->game_pid, memregions, compare_type, compare_operator, compare_value, rsw->search_progress);
            break;
    }

    rsw->address_table->hex = (rsw->display_choice->value() == 1);
    rsw->address_table->rows(rsw->ram_search.ramwatches.size());

    /* Update address count */
    std::ostringstream oss;
    oss << rsw->ram_search.ramwatches.size();
    oss << " addresses";
    rsw->watch_count->copy_label(oss.str().c_str());
}

static void search_cb(Fl_Widget* w, void* v)
{
    RamSearchWindow* rsw = static_cast<RamSearchWindow*>(v);

    CompareType compare_type;
    CompareOperator compare_operator;
    double compare_value;
    get_compare_parameters(rsw, compare_type, compare_operator, compare_value);

    rsw->search_progress->show();
    rsw->search_progress->bounds(0, rsw->ram_search.ramwatches.size());

    /* Update the previous_value attribute of each RamWatch object in the vector,
     * and remove objects from the vector where the search condition returns false.
     */
    int num = 0;
    rsw->ram_search.ramwatches.erase(
        std::remove_if(rsw->ram_search.ramwatches.begin(), rsw->ram_search.ramwatches.end(),
            [&compare_type, &compare_operator, &compare_value, &num, &rsw] (std::unique_ptr<IRamWatch> &watch) {
                if (!(num++ & 0xfff)) {
                    rsw->search_progress->value(num);
                    Fl::flush();
                }
                return watch->check_update(compare_type, compare_operator, compare_value);
            }),
        rsw->ram_search.ramwatches.end());

    rsw->search_progress->hide();

    /* Update table parameters */
    rsw->address_table->hex = (rsw->display_choice->value() == 1);
    rsw->address_table->compare_type = compare_type;
    rsw->address_table->compare_operator = compare_operator;
    rsw->address_table->compare_value_db = compare_value;
    rsw->address_table->rows(rsw->ram_search.ramwatches.size());

    /* Update address count */
    std::ostringstream oss;
    oss << rsw->ram_search.ramwatches.size();
    oss << " adresses";
    rsw->watch_count->copy_label(oss.str().c_str());
}

void add_cb(Fl_Widget* w, void* v)
{
    RamSearchWindow* rsw = static_cast<RamSearchWindow*>(v);

    int r;
    for (r=0; r<rsw->address_table->rows(); r++) {
        if (rsw->address_table->row_selected(r)) {
            break;
        }
    }

    /* If no watch was selected, return */
    if (r == rsw->address_table->rows())
        return;

    /* Fill the watch edit window with parameters from the selected watch */
    MainWindow& mw = MainWindow::getInstance();
    mw.ramwatch_window->edit_window->fill(rsw->ram_search.ramwatches.at(r));

    mw.ramwatch_window->edit_window->window->show();

    while (mw.ramwatch_window->edit_window->window->shown()) {
        Fl::wait();
    }

    if (mw.ramwatch_window->edit_window->ramwatch) {
        mw.ramwatch_window->edit_window->ramwatch->game_pid = rsw->context->game_pid;
        mw.ramwatch_window->watch_table->ramwatches.push_back(std::move(mw.ramwatch_window->edit_window->ramwatch));
        mw.ramwatch_window->watch_table->update();
    }
}
