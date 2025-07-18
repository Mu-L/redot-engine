/**************************************************************************/
/*  string_name.h                                                         */
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

#include "core/string/ustring.h"
#include "core/templates/safe_refcount.h"

#define UNIQUE_NODE_PREFIX "%"

class Main;

class [[nodiscard]] StringName {
	struct Table;

	struct _Data {
		SafeRefCount refcount;
		SafeNumeric<uint32_t> static_count;
		String name;
#ifdef DEBUG_ENABLED
		uint32_t debug_references = 0;
#endif

		uint32_t hash = 0;
		_Data *prev = nullptr;
		_Data *next = nullptr;
		_Data() {}
	};

	_Data *_data = nullptr;

	void unref();
	friend void register_core_types();
	friend void unregister_core_types();
	friend class Main;
	static void setup();
	static void cleanup();
	static uint32_t get_empty_hash();
	static inline bool configured = false;
#ifdef DEBUG_ENABLED
	struct DebugSortReferences {
		bool operator()(const _Data *p_left, const _Data *p_right) const {
			return p_left->debug_references > p_right->debug_references;
		}
	};

	static inline bool debug_stringname = false;
#endif

	StringName(_Data *p_data) { _data = p_data; }

public:
	_FORCE_INLINE_ explicit operator bool() const { return _data; }

	bool operator==(const String &p_name) const;
	bool operator==(const char *p_name) const;
	bool operator!=(const String &p_name) const;
	bool operator!=(const char *p_name) const;

	const char32_t *get_data() const { return _data ? _data->name.ptr() : U""; }
	char32_t operator[](int p_index) const;
	int length() const;
	_FORCE_INLINE_ bool is_empty() const { return !_data; }

	_FORCE_INLINE_ bool is_node_unique_name() const {
		if (!_data) {
			return false;
		}
		return (char32_t)_data->name[0] == (char32_t)UNIQUE_NODE_PREFIX[0];
	}
	_FORCE_INLINE_ bool operator<(const StringName &p_name) const {
		return _data < p_name._data;
	}
	_FORCE_INLINE_ bool operator<=(const StringName &p_name) const {
		return _data <= p_name._data;
	}
	_FORCE_INLINE_ bool operator>(const StringName &p_name) const {
		return _data > p_name._data;
	}
	_FORCE_INLINE_ bool operator>=(const StringName &p_name) const {
		return _data >= p_name._data;
	}
	_FORCE_INLINE_ bool operator==(const StringName &p_name) const {
		// The real magic of all this mess happens here.
		// This is why path comparisons are very fast.
		return _data == p_name._data;
	}
	_FORCE_INLINE_ bool operator!=(const StringName &p_name) const {
		return _data != p_name._data;
	}
	_FORCE_INLINE_ uint32_t hash() const {
		if (_data) {
			return _data->hash;
		} else {
			return get_empty_hash();
		}
	}
	_FORCE_INLINE_ const void *data_unique_pointer() const {
		return (void *)_data;
	}

	_FORCE_INLINE_ operator String() const {
		if (_data) {
			return _data->name;
		}

		return String();
	}

	struct AlphCompare {
		template <typename LT, typename RT>
		_FORCE_INLINE_ bool operator()(const LT &l, const RT &r) const {
			return compare(l, r);
		}
		_FORCE_INLINE_ static bool compare(const StringName &l, const StringName &r) {
			return str_compare(l.get_data(), r.get_data()) < 0;
		}
		_FORCE_INLINE_ static bool compare(const String &l, const StringName &r) {
			return str_compare(l.get_data(), r.get_data()) < 0;
		}
		_FORCE_INLINE_ static bool compare(const StringName &l, const String &r) {
			return str_compare(l.get_data(), r.get_data()) < 0;
		}
		_FORCE_INLINE_ static bool compare(const String &l, const String &r) {
			return str_compare(l.get_data(), r.get_data()) < 0;
		}
	};

	StringName &operator=(const StringName &p_name);
	StringName &operator=(StringName &&p_name) {
		if (_data == p_name._data) {
			return *this;
		}

		unref();
		_data = p_name._data;
		p_name._data = nullptr;
		return *this;
	}
	StringName(const char *p_name, bool p_static = false);
	StringName(const StringName &p_name);
	StringName(StringName &&p_name) {
		_data = p_name._data;
		p_name._data = nullptr;
	}
	StringName(const String &p_name, bool p_static = false);
	StringName() {}

#ifdef SIZE_EXTRA
	_NO_INLINE_
#else
	_FORCE_INLINE_
#endif
	~StringName() {
		if (likely(configured) && _data) { //only free if configured
			unref();
		}
	}

#ifdef DEBUG_ENABLED
	static void set_debug_stringnames(bool p_enable) { debug_stringname = p_enable; }
#endif
};

// Zero-constructing StringName initializes _data to nullptr (and thus empty).
template <>
struct is_zero_constructible<StringName> : std::true_type {};

bool operator==(const String &p_name, const StringName &p_string_name);
bool operator!=(const String &p_name, const StringName &p_string_name);
bool operator==(const char *p_name, const StringName &p_string_name);
bool operator!=(const char *p_name, const StringName &p_string_name);

/*
 * The SNAME macro is used to speed up StringName creation, as it allows caching it after the first usage in a very efficient way.
 * It should NOT be used everywhere, but instead in places where high performance is required and the creation of a StringName
 * can be costly. Places where it should be used are:
 * - Control::get_theme_*(<name> and Window::get_theme_*(<name> functions.
 * - emit_signal(<name>,..) function
 * - call_deferred(<name>,..) function
 * - Comparisons to a StringName in overridden _set and _get methods.
 *
 * Use in places that can be called hundreds of times per frame (or more) is recommended, but this situation is very rare. If in doubt, do not use.
 */

#define SNAME(m_arg) ([]() -> const StringName & { static StringName sname = StringName(m_arg, true); return sname; })()
