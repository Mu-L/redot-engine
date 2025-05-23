/**************************************************************************/
/*  label.h                                                               */
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

#include "scene/gui/control.h"
#include "scene/resources/label_settings.h"

class Label : public Control {
	GDCLASS(Label, Control);

private:
	HorizontalAlignment horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT;
	VerticalAlignment vertical_alignment = VERTICAL_ALIGNMENT_TOP;
	String text;
	String xl_text;
	TextServer::AutowrapMode autowrap_mode = TextServer::AUTOWRAP_OFF;
	BitField<TextServer::LineBreakFlag> autowrap_flags_trim = TextServer::BREAK_TRIM_START_EDGE_SPACES | TextServer::BREAK_TRIM_END_EDGE_SPACES;
	BitField<TextServer::JustificationFlag> jst_flags = TextServer::JUSTIFICATION_WORD_BOUND | TextServer::JUSTIFICATION_KASHIDA | TextServer::JUSTIFICATION_SKIP_LAST_LINE | TextServer::JUSTIFICATION_DO_NOT_SKIP_SINGLE_LINE;
	bool clip = false;
	String el_char = U"…";
	TextServer::OverrunBehavior overrun_behavior = TextServer::OVERRUN_NO_TRIMMING;
	mutable Size2 minsize;
	bool uppercase = false;

	struct Paragraph {
		bool lines_dirty = true;
		bool dirty = true;
		int start = 0;

		String text;
		RID text_rid;
		Vector<RID> lines_rid;
	};
	mutable bool dirty = true;
	mutable bool font_dirty = true;
	mutable bool text_dirty = true;
	mutable Vector<Paragraph> paragraphs;
	mutable int total_line_count = 0;
	String paragraph_separator = "\\n";

	String language;
	TextDirection text_direction = TEXT_DIRECTION_AUTO;
	TextServer::StructuredTextParser st_parser = TextServer::STRUCTURED_TEXT_DEFAULT;
	Array st_args;

	TextServer::VisibleCharactersBehavior visible_chars_behavior = TextServer::VC_CHARS_BEFORE_SHAPING;
	int visible_chars = -1;
	float visible_ratio = 1.0;
	int lines_skipped = 0;
	int max_lines_visible = -1;
	PackedFloat32Array tab_stops;

	Ref<LabelSettings> settings;

	struct ThemeCache {
		Ref<StyleBox> normal_style;
		Ref<StyleBox> focus_style;
		Ref<Font> font;

		int font_size = 0;
		int line_spacing = 0;
		int paragraph_spacing = 0;
		Color font_color;
		Color font_shadow_color;
		Point2 font_shadow_offset;
		Color font_outline_color;
		int font_outline_size;
		int font_shadow_outline_size;
	} theme_cache;

	Rect2 _get_line_rect(int p_para, int p_line) const;
	void _ensure_shaped() const;
	void _update_visible() const;
	void _shape() const;
	void _invalidate();

protected:
	RID get_line_rid(int p_line) const;
	Rect2 get_line_rect(int p_line) const;
	int get_layout_data(Vector2 &r_offset, int &r_last_line, int &r_line_spacing) const;

	void _notification(int p_what);
	static void _bind_methods();
#ifndef DISABLE_DEPRECATED
	bool _set(const StringName &p_name, const Variant &p_value);
#endif

public:
	virtual Size2 get_minimum_size() const override;
	virtual PackedStringArray get_configuration_warnings() const override;

	void set_horizontal_alignment(HorizontalAlignment p_alignment);
	HorizontalAlignment get_horizontal_alignment() const;

	void set_vertical_alignment(VerticalAlignment p_alignment);
	VerticalAlignment get_vertical_alignment() const;

	void set_text(const String &p_string);
	String get_text() const;

	void set_label_settings(const Ref<LabelSettings> &p_settings);
	Ref<LabelSettings> get_label_settings() const;

	void set_text_direction(TextDirection p_text_direction);
	TextDirection get_text_direction() const;

	void set_language(const String &p_language);
	String get_language() const;

	void set_paragraph_separator(const String &p_paragraph_separator);
	String get_paragraph_separator() const;

	void set_structured_text_bidi_override(TextServer::StructuredTextParser p_parser);
	TextServer::StructuredTextParser get_structured_text_bidi_override() const;

	void set_structured_text_bidi_override_options(Array p_args);
	Array get_structured_text_bidi_override_options() const;

	void set_autowrap_mode(TextServer::AutowrapMode p_mode);
	TextServer::AutowrapMode get_autowrap_mode() const;

	void set_autowrap_trim_flags(BitField<TextServer::LineBreakFlag> p_flags);
	BitField<TextServer::LineBreakFlag> get_autowrap_trim_flags() const;

	void set_justification_flags(BitField<TextServer::JustificationFlag> p_flags);
	BitField<TextServer::JustificationFlag> get_justification_flags() const;

	void set_uppercase(bool p_uppercase);
	bool is_uppercase() const;

	void set_visible_characters_behavior(TextServer::VisibleCharactersBehavior p_behavior);
	TextServer::VisibleCharactersBehavior get_visible_characters_behavior() const;

	void set_visible_characters(int p_amount);
	int get_visible_characters() const;
	int get_total_character_count() const;

	void set_visible_ratio(float p_ratio);
	float get_visible_ratio() const;

	void set_clip_text(bool p_clip);
	bool is_clipping_text() const;

	void set_tab_stops(const PackedFloat32Array &p_tab_stops);
	PackedFloat32Array get_tab_stops() const;

	void set_text_overrun_behavior(TextServer::OverrunBehavior p_behavior);
	TextServer::OverrunBehavior get_text_overrun_behavior() const;

	void set_ellipsis_char(const String &p_char);
	String get_ellipsis_char() const;

	void set_lines_skipped(int p_lines);
	int get_lines_skipped() const;

	void set_max_lines_visible(int p_lines);
	int get_max_lines_visible() const;

	int get_line_height(int p_line = -1) const;
	int get_line_count() const;
	int get_visible_line_count() const;

	Rect2 get_character_bounds(int p_pos) const;

	Label(const String &p_text = String());
	~Label();

	template <typename... VarArgsFunc, typename... VarArgs>
	void draw_text(bool p_rtl, int p_ellipsis_pos, int p_ellipsis_gl_size, const Glyph *p_ellipsis_glyphs, bool p_trim_chars, int p_para_start, int p_visible_chars, bool p_trim_glyphs_ltr, int &p_processed_glyphs_step, int p_processed_glyphs, int p_visible_glyphs, bool p_trim_glyphs_rtl, int p_total_glyphs, const RID &p_ci, const Vector2 &p_ofs, int p_gl_size, int p_trim_pos, const Glyph *p_glyphs, const Color &p_color, void (*p_draw_func)(const Glyph &p_gl, const RID &p_canvas, const Color &p_font_outline_color, const Vector2 &p_ofs, VarArgsFunc... p_args), VarArgs &&...p_args) {
		p_processed_glyphs_step = p_processed_glyphs;
		Vector2 offset_step = p_ofs; /* Draw RTL ellipsis string when necessary. */
		if (p_rtl && p_ellipsis_pos >= 0) {
			for (int gl_idx = p_ellipsis_gl_size - 1; gl_idx >= 0; gl_idx--) {
				for (int j = 0; j < p_ellipsis_glyphs[gl_idx].repeat; j++) {
					bool skip = (p_trim_chars && p_ellipsis_glyphs[gl_idx].end + p_para_start > p_visible_chars) || (p_trim_glyphs_ltr && (p_processed_glyphs_step >= p_visible_glyphs)) || (p_trim_glyphs_rtl && (p_processed_glyphs_step < p_total_glyphs - p_visible_glyphs));
					if (!skip) {
						p_draw_func(p_ellipsis_glyphs[gl_idx], p_ci, p_color, offset_step, std::forward<VarArgs>(p_args)...);
					}
					p_processed_glyphs_step++;
					offset_step.x += p_ellipsis_glyphs[gl_idx].advance;
				}
			}
		} /* Draw main text. */
		for (int j = 0; j < p_gl_size; j++) { /* Trim when necessary. */
			if (p_trim_pos >= 0) {
				if (p_rtl) {
					if (j < p_trim_pos) {
						continue;
					}
				} else {
					if (j >= p_trim_pos) {
						break;
					}
				}
			}
			for (int k = 0; k < p_glyphs[j].repeat; k++) {
				bool skip = (p_trim_chars && p_glyphs[j].end + p_para_start > p_visible_chars) || (p_trim_glyphs_ltr && (p_processed_glyphs_step >= p_visible_glyphs)) || (p_trim_glyphs_rtl && (p_processed_glyphs_step < p_total_glyphs - p_visible_glyphs));
				if (!skip) {
					p_draw_func(p_glyphs[j], p_ci, p_color, offset_step, std::forward<VarArgs>(p_args)...);
				}
				p_processed_glyphs_step++;
				offset_step.x += p_glyphs[j].advance;
			}
		} /* Draw LTR ellipsis string when necessary. */
		if (!p_rtl && p_ellipsis_pos >= 0) {
			for (int gl_idx = 0; gl_idx < p_ellipsis_gl_size; gl_idx++) {
				for (int j = 0; j < p_ellipsis_glyphs[gl_idx].repeat; j++) {
					bool skip = (p_trim_chars && p_ellipsis_glyphs[gl_idx].end + p_para_start > p_visible_chars) || (p_trim_glyphs_ltr && (p_processed_glyphs_step >= p_visible_glyphs)) || (p_trim_glyphs_rtl && (p_processed_glyphs_step < p_total_glyphs - p_visible_glyphs));
					if (!skip) {
						p_draw_func(p_ellipsis_glyphs[gl_idx], p_ci, p_color, offset_step, std::forward<VarArgs>(p_args)...);
					}
					p_processed_glyphs_step++;
					offset_step.x += p_ellipsis_glyphs[gl_idx].advance;
				}
			}
		}
	}
};
