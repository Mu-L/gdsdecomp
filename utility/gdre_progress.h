/**************************************************************************/
/*  progress_dialog.h                                                     */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef GDRE_PROGRESS_DIALOG_H
#define GDRE_PROGRESS_DIALOG_H

#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/label.h"
#include "scene/gui/popup.h"
#include "scene/gui/progress_bar.h"

class GDREBackgroundProgress : public HBoxContainer {
	GDCLASS(GDREBackgroundProgress, HBoxContainer);

	_THREAD_SAFE_CLASS_

	struct Task {
		HBoxContainer *hb = nullptr;
		ProgressBar *progress = nullptr;
	};

	HashMap<String, Task> tasks;
	HashMap<String, int> updates;
	void _update();

protected:
	void _add_task(const String &p_task, const String &p_label, int p_steps);
	void _task_step(const String &p_task, int p_step = -1);
	void _end_task(const String &p_task);

public:
	void add_task(const String &p_task, const String &p_label, int p_steps);
	void task_step(const String &p_task, int p_step = -1);
	void end_task(const String &p_task);

	GDREBackgroundProgress() {}
};

class GDREProgressDialog : public PopupPanel {
	GDCLASS(GDREProgressDialog, PopupPanel);
	struct Task {
		String task;
		VBoxContainer *vb = nullptr;
		ProgressBar *progress = nullptr;
		Label *state = nullptr;
		uint64_t last_progress_tick = 0;
	};
	HBoxContainer *cancel_hb = nullptr;
	Button *cancel = nullptr;

	HashMap<String, Task> tasks;
	VBoxContainer *main = nullptr;

	LocalVector<Window *> host_windows;

	static GDREProgressDialog *singleton;
	void _popup();

	void _cancel_pressed();

	void _update_ui();
	bool canceled = false;

public:
	static GDREProgressDialog *get_singleton() { return singleton; }
	void add_task(const String &p_task, const String &p_label, int p_steps, bool p_can_cancel = false);
	bool task_step(const String &p_task, const String &p_state, int p_step = -1, bool p_force_redraw = true);
	void end_task(const String &p_task);

	void add_host_window(Window *p_window);
	void remove_host_window(Window *p_window);

	GDREProgressDialog();
	~GDREProgressDialog();
};

struct EditorProgressGDDC {
	String task;
	GDREProgressDialog *progress_dialog;
	bool step(const String &p_state, int p_step = -1, bool p_force_refresh = true);
	EditorProgressGDDC(Node *p_parent, const String &p_task, const String &p_label, int p_amount, bool p_can_cancel = false);
	~EditorProgressGDDC();
};

#endif // GDRE_PROGRESS_H