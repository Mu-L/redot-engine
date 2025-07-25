/**************************************************************************/
/*  texture_3d_editor_plugin.cpp                                          */
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

#include "texture_3d_editor_plugin.h"

#include "editor/editor_string_names.h"
#include "editor/scene/texture/color_channel_selector.h"
#include "editor/themes/editor_scale.h"
#include "scene/gui/label.h"

// Shader sources.

constexpr const char *texture_3d_shader = R"(
	// Texture3DEditor preview shader.

	shader_type canvas_item;

	uniform sampler3D tex;
	uniform float layer;

	uniform vec4 u_channel_factors = vec4(1.0);

	vec4 filter_preview_colors(vec4 input_color, vec4 factors) {
		// Filter RGB.
		vec4 output_color = input_color * vec4(factors.rgb, input_color.a);

		// Remove transparency when alpha is not enabled.
		output_color.a = mix(1.0, output_color.a, factors.a);

		// Switch to opaque grayscale when visualizing only one channel.
		float csum = factors.r + factors.g + factors.b + factors.a;
		float single = clamp(2.0 - csum, 0.0, 1.0);
		for (int i = 0; i < 4; i++) {
			float c = input_color[i];
			output_color = mix(output_color, vec4(c, c, c, 1.0), factors[i] * single);
		}

		return output_color;
	}

	void fragment() {
		COLOR = textureLod(tex, vec3(UV, layer), 0.0);
		COLOR = filter_preview_colors(COLOR, u_channel_factors);
	}
)";

void Texture3DEditor::_texture_rect_draw() {
	texture_rect->draw_rect(Rect2(Point2(), texture_rect->get_size()), Color(1, 1, 1, 1));
}

void Texture3DEditor::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_RESIZED: {
			_texture_rect_update_area();
		} break;

		case NOTIFICATION_DRAW: {
			Ref<Texture2D> checkerboard = get_editor_theme_icon(SNAME("Checkerboard"));
			draw_texture_rect(checkerboard, texture_rect->get_rect(), true);
			_draw_outline();
		} break;

		case NOTIFICATION_THEME_CHANGED: {
			if (info) {
				Ref<Font> metadata_label_font = get_theme_font(SNAME("expression"), EditorStringName(EditorFonts));
				info->add_theme_font_override(SceneStringName(font), metadata_label_font);
			}
			theme_cache.outline_color = get_theme_color(SNAME("extra_border_color_1"), EditorStringName(Editor));
		} break;
	}
}

void Texture3DEditor::_texture_changed() {
	if (!is_visible()) {
		return;
	}

	setting = true;
	_update_gui();
	setting = false;

	_update_material(true);
	queue_redraw();
}

void Texture3DEditor::_update_material(bool p_texture_changed) {
	material->set_shader_parameter("layer", (layer->get_value() + 0.5) / texture->get_depth());

	if (p_texture_changed) {
		material->set_shader_parameter("tex", texture->get_rid());
	}

	material->set_shader_parameter("u_channel_factors", channel_selector->get_selected_channel_factors());
}

void Texture3DEditor::_draw_outline() {
	const float outline_width = Math::round(EDSCALE);
	const Rect2 outline_rect = texture_rect->get_rect().grow(outline_width * 0.5);
	draw_rect(outline_rect, theme_cache.outline_color, false, outline_width);
}

void Texture3DEditor::_make_shaders() {
	shader.instantiate();
	shader->set_code(texture_3d_shader);

	material.instantiate();
	material->set_shader(shader);
}

void Texture3DEditor::_texture_rect_update_area() {
	Size2 size = get_size();
	int tex_width = texture->get_width() * size.height / texture->get_height();
	int tex_height = size.height;

	if (tex_width > size.width) {
		tex_width = size.width;
		tex_height = texture->get_height() * tex_width / texture->get_width();
	}

	// Prevent the texture from being unpreviewable after the rescale, so that we can still see something
	if (tex_height <= 0) {
		tex_height = 1;
	}
	if (tex_width <= 0) {
		tex_width = 1;
	}

	int ofs_x = (size.width - tex_width) / 2;
	int ofs_y = (size.height - tex_height) / 2;

	texture_rect->set_position(Vector2(ofs_x, ofs_y - Math::round(EDSCALE)));
	texture_rect->set_size(Vector2(tex_width, tex_height));
}

void Texture3DEditor::_update_gui() {
	if (texture.is_null()) {
		return;
	}

	_texture_rect_update_area();

	layer->set_max(texture->get_depth() - 1);

	const Image::Format format = texture->get_format();
	const String format_name = Image::get_format_name(format);

	if (texture->has_mipmaps()) {
		const int mip_count = Image::get_image_required_mipmaps(texture->get_width(), texture->get_height(), format);
		const int memory = Image::get_image_data_size(texture->get_width(), texture->get_height(), format, true) * texture->get_depth();

		info->set_text(vformat(String::utf8("%d×%d×%d %s\n") + TTR("%s Mipmaps") + "\n" + TTR("Memory: %s"),
				texture->get_width(),
				texture->get_height(),
				texture->get_depth(),
				format_name,
				mip_count,
				String::humanize_size(memory)));

	} else {
		const int memory = Image::get_image_data_size(texture->get_width(), texture->get_height(), format, false) * texture->get_depth();

		info->set_text(vformat(String::utf8("%d×%d×%d %s\n") + TTR("No Mipmaps") + "\n" + TTR("Memory: %s"),
				texture->get_width(),
				texture->get_height(),
				texture->get_depth(),
				format_name,
				String::humanize_size(memory)));
	}

	const uint32_t components_mask = Image::get_format_component_mask(format);
	if (is_power_of_2(components_mask)) {
		// Only one channel available, no point in showing a channel selector.
		channel_selector->hide();
	} else {
		channel_selector->show();
		channel_selector->set_available_channels_mask(components_mask);
	}
}

void Texture3DEditor::on_selected_channels_changed() {
	_update_material(false);
}

void Texture3DEditor::edit(Ref<Texture3D> p_texture) {
	if (texture.is_valid()) {
		texture->disconnect_changed(callable_mp(this, &Texture3DEditor::_texture_changed));
	}

	texture = p_texture;

	if (texture.is_valid()) {
		if (shader.is_null()) {
			_make_shaders();
		}

		texture->connect_changed(callable_mp(this, &Texture3DEditor::_texture_changed));
		texture_rect->set_material(material);

		setting = true;
		layer->set_value(0);
		layer->show();
		_update_gui();
		setting = false;

		_update_material(true);
		queue_redraw();

	} else {
		hide();
	}
}

Texture3DEditor::Texture3DEditor() {
	set_texture_repeat(TextureRepeat::TEXTURE_REPEAT_ENABLED);
	set_custom_minimum_size(Size2(1, 256.0) * EDSCALE);

	texture_rect = memnew(Control);
	texture_rect->set_mouse_filter(MOUSE_FILTER_IGNORE);
	texture_rect->connect(SceneStringName(draw), callable_mp(this, &Texture3DEditor::_texture_rect_draw));

	add_child(texture_rect);

	layer = memnew(SpinBox);
	layer->set_step(1);
	layer->set_max(100);

	layer->set_modulate(Color(1, 1, 1, 0.8));
	layer->set_h_grow_direction(GROW_DIRECTION_BEGIN);
	layer->set_anchor(SIDE_RIGHT, 1);
	layer->set_anchor(SIDE_LEFT, 1);
	layer->connect(SceneStringName(value_changed), callable_mp(this, &Texture3DEditor::_layer_changed));

	add_child(layer);

	channel_selector = memnew(ColorChannelSelector);
	channel_selector->connect("selected_channels_changed", callable_mp(this, &Texture3DEditor::on_selected_channels_changed));
	channel_selector->set_anchors_preset(Control::PRESET_TOP_LEFT);
	add_child(channel_selector);

	info = memnew(Label);
	info->set_focus_mode(FOCUS_ACCESSIBILITY);
	info->add_theme_color_override(SceneStringName(font_color), Color(1, 1, 1));
	info->add_theme_color_override("font_shadow_color", Color(0, 0, 0));
	info->add_theme_font_size_override(SceneStringName(font_size), 14 * EDSCALE);
	info->add_theme_color_override("font_outline_color", Color(0, 0, 0));
	info->add_theme_constant_override("outline_size", 8 * EDSCALE);

	info->set_h_grow_direction(GROW_DIRECTION_BEGIN);
	info->set_v_grow_direction(GROW_DIRECTION_BEGIN);
	info->set_h_size_flags(Control::SIZE_SHRINK_END);
	info->set_v_size_flags(Control::SIZE_SHRINK_END);
	info->set_anchor(SIDE_RIGHT, 1);
	info->set_anchor(SIDE_LEFT, 1);
	info->set_anchor(SIDE_BOTTOM, 1);
	info->set_anchor(SIDE_TOP, 1);

	add_child(info);
}

Texture3DEditor::~Texture3DEditor() {
	if (texture.is_valid()) {
		texture->disconnect_changed(callable_mp(this, &Texture3DEditor::_texture_changed));
	}
}

bool EditorInspectorPlugin3DTexture::can_handle(Object *p_object) {
	return Object::cast_to<Texture3D>(p_object) != nullptr;
}

void EditorInspectorPlugin3DTexture::parse_begin(Object *p_object) {
	Texture3D *texture = Object::cast_to<Texture3D>(p_object);
	if (!texture) {
		return;
	}
	Ref<Texture3D> m(texture);

	Texture3DEditor *editor = memnew(Texture3DEditor);
	editor->edit(m);
	add_custom_control(editor);
}

Texture3DEditorPlugin::Texture3DEditorPlugin() {
	Ref<EditorInspectorPlugin3DTexture> plugin;
	plugin.instantiate();
	add_inspector_plugin(plugin);
}
