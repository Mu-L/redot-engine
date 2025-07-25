/**************************************************************************/
/*  editor_settings_dialog.h                                              */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             REDOT ENGINE                               */
/*                        https://redotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2024-present Redot Engine contributors                   */
/*                                          (see REDOT_AUTHORS.md)        */
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

#pragma once

#include "editor/inspector/editor_inspector.h"
#include "scene/gui/dialogs.h"

class CheckButton;
class EditorEventSearchBar;
class EventListenerLineEdit;
class InputEventConfigurationDialog;
class PanelContainer;
class SectionedInspector;
class TabContainer;
class TextureRect;
class Tree;
class TreeItem;

class EditorSettingsDialog : public AcceptDialog {
	GDCLASS(EditorSettingsDialog, AcceptDialog);

	bool updating = false;

	TabContainer *tabs = nullptr;
	Control *tab_general = nullptr;
	Control *tab_shortcuts = nullptr;

	LineEdit *search_box = nullptr;
	CheckButton *advanced_switch = nullptr;
	SectionedInspector *inspector = nullptr;
	EditorEventSearchBar *shortcut_search_bar = nullptr;

	// Shortcuts
	enum ShortcutButton {
		SHORTCUT_ADD,
		SHORTCUT_EDIT,
		SHORTCUT_ERASE,
		SHORTCUT_REVERT
	};

	Tree *shortcuts = nullptr;

	InputEventConfigurationDialog *shortcut_editor = nullptr;

	bool is_editing_action = false;
	String current_edited_identifier;
	Array current_events;
	int current_event_index = -1;

	Timer *timer = nullptr;

	virtual void cancel_pressed() override;
	virtual void ok_pressed() override;

	void _settings_changed();
	void _settings_property_edited(const String &p_name);
	void _settings_save();

	virtual void shortcut_input(const Ref<InputEvent> &p_event) override;
	void _notification(int p_what);
	void _update_icons();

	void _event_config_confirmed();

	TreeItem *_create_shortcut_treeitem(TreeItem *p_parent, const String &p_shortcut_identifier, const String &p_display, Array &p_events, bool p_allow_revert, bool p_is_common, bool p_is_collapsed);
	Array _event_list_to_array_helper(const List<Ref<InputEvent>> &p_events);
	void _update_builtin_action(const String &p_name, const Array &p_events);
	void _update_shortcut_events(const String &p_path, const Array &p_events);

	Variant get_drag_data_fw(const Point2 &p_point, Control *p_from);
	bool can_drop_data_fw(const Point2 &p_point, const Variant &p_data, Control *p_from) const;
	void drop_data_fw(const Point2 &p_point, const Variant &p_data, Control *p_from);

	void _tabs_tab_changed(int p_tab);
	void _focus_current_search_box();

	void _advanced_toggled(bool p_button_pressed);

	void _update_dynamic_property_hints();
	PropertyInfo _create_mouse_shortcut_property_info(const String &p_property_name, const String &p_shortcut_1_name, const String &p_shortcut_2_name);
	String _get_shortcut_button_string(const String &p_shortcut_name);

	bool _should_display_shortcut(const String &p_name, const Array &p_events, bool p_match_localized_name) const;

	void _update_shortcuts();
	void _shortcut_button_pressed(Object *p_item, int p_column, int p_idx, MouseButton p_button = MouseButton::LEFT);
	void _shortcut_cell_double_clicked();
	static void _set_shortcut_input(const String &p_name, Ref<InputEventKey> &p_event);

	static void _undo_redo_callback(void *p_self, const String &p_name);

	Label *restart_label = nullptr;
	TextureRect *restart_icon = nullptr;
	PanelContainer *restart_container = nullptr;
	Button *restart_close_button = nullptr;

	void _editor_restart_request();
	void _editor_restart();
	void _editor_restart_close();

protected:
	static void _bind_methods();

public:
	void popup_edit_settings();
	static void update_navigation_preset();

	EditorSettingsDialog();
};

class EditorSettingsPropertyWrapper : public EditorProperty {
	GDCLASS(EditorSettingsPropertyWrapper, EditorProperty);

	String property;
	EditorProperty *editor_property = nullptr;

	BoxContainer *container = nullptr;

	HBoxContainer *override_info = nullptr;
	Label *override_label = nullptr;
	Button *goto_button = nullptr;
	Button *remove_button = nullptr;

	bool requires_restart = false;

	void _update_override();
	void _create_override();
	void _remove_override();

protected:
	void _notification(int p_what);

public:
	static inline Callable restart_request_callback;

	virtual void update_property() override;
	void setup(const String &p_property, EditorProperty *p_editor_property, bool p_requires_restart);
};

class EditorSettingsInspectorPlugin : public EditorInspectorPlugin {
	GDCLASS(EditorSettingsInspectorPlugin, EditorInspectorPlugin);

	Object *current_object = nullptr;

public:
	SectionedInspector *inspector = nullptr;

	virtual bool can_handle(Object *p_object) override;
	virtual bool parse_property(Object *p_object, const Variant::Type p_type, const String &p_path, const PropertyHint p_hint, const String &p_hint_text, const BitField<PropertyUsageFlags> p_usage, const bool p_wide = false) override;
};
