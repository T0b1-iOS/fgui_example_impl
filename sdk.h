#pragma once

#include <cstdint>
#include <utility>
#include <cwchar>

#include "btn_code.h"

namespace sdk
{
	template<typename return_type_t, typename... args_t>
	using this_call = return_type_t(__thiscall *)(void *, args_t...);

	template<typename function_t = void *>
	function_t vfunc(void *object, const size_t idx)
	{
		return reinterpret_cast<function_t>((*static_cast<void ***>(object))[idx]);
	}

	template<typename return_type_t, typename... args_t>
	return_type_t this_vfunc(void *object, const size_t idx, args_t... args)
	{
		return vfunc<this_call<return_type_t, args_t...>>(object, idx)(
		  object, std::forward<args_t>(args)...);
	}

	enum FONT_FLAGS
	{
		FONT_FLAG_NONE,
		FONT_FLAG_ITALIC        = 0x001,
		FONT_FLAG_UNDERLINE     = 0x002,
		FONT_FLAG_STRIKEOUT     = 0x004,
		FONT_FLAG_SYMBOL        = 0x008,
		FONT_FLAG_ANTI_ALIAS    = 0x010,
		FONT_FLAG_GAUSSIAN_BLUR = 0x020,
		FONT_FLAG_ROTARY        = 0x040,
		FONT_FLAG_DROP_SHADOW   = 0x080,
		FONT_FLAG_ADDITIVE      = 0x100,
		FONT_FLAG_OUTLINE       = 0x200,
		FONT_FLAG_CUSTOM =
		  0x400,  // custom generated font - never fall back to asian compatibility mode
		FONT_FLAG_BITMAP = 0x800,  // compiled bitmap font - no fallbacks
	};

	struct surface
	{
		using font_t = unsigned long;

		void draw_set_color(const int r, const int g, const int b, const int a)
		{
			this_vfunc<void>(this, 15u, r, g, b, a);
		}

		void draw_filled_rect(const int x, const int y, const int x1, const int y1)
		{
			this_vfunc<void>(this, 16u, x, y, x1, y1);
		}

		void draw_outlined_rect(const int x, const int y, const int x1,
		                        const int y1)
		{
			this_vfunc<void>(this, 18u, x, y, x1, y1);
		}

		void draw_line(const int x, const int y, const int x1, const int y1)
		{
			this_vfunc<void>(this, 19u, x, y, x1, y1);
		}

		void draw_set_text_font(const font_t font)
		{
			this_vfunc<void>(this, 23u, font);
		}

		void draw_set_text_color(const int r, const int g, const int b,
		                         const int a = 255)
		{
			this_vfunc<void>(this, 25u, r, g, b, a);
		}

		void draw_set_text_pos(const int x, const int y)
		{
			this_vfunc<void>(this, 26u, x, y);
		}

		void screen_size(int &width, int &height)
		{
			this_vfunc<void, int &, int &>(this, 44u, width, height);
		}

		void draw_text(const wchar_t *text)
		{
			this_vfunc<void>(this, 28u, text, wcslen(text), 0);
		}

		font_t create_font() { return this_vfunc<font_t>(this, 71u); }

		bool set_font_glyph_set(const font_t font, const char *name, const int tall,
		                        const int weight, const int blur,
		                        const int scan_lines, const int flags,
		                        const int range_min = 0, const int range_max = 0)
		{
			return this_vfunc<bool>(this, 72u, font, name, tall, weight, blur,
			                        scan_lines, flags, range_min, range_max);
		}

		void text_size(const font_t font, const wchar_t *text, int &width,
		               int &height)
		{
			this_vfunc<void>(this, 79u, font, text, width, height);
		}

		void draw_filled_rect_fade(const int x, const int y, const int x1,
		                           const int x2, const unsigned alpha0,
		                           const unsigned alpha1, const bool horizontal)
		{
			this_vfunc<void>(this, 123u, x, y, x1, x2, alpha0, alpha1, horizontal);
		}
	};

	struct input_system
	{
		bool button_down(const BUTTON_CODE code)
		{
			return this_vfunc<bool>(this, 15u, code);
		}

		void *window()
		{
			return *reinterpret_cast<void **>(reinterpret_cast<uintptr_t>(this)
			                                  + 0x1324);
		}
	};

	struct panel
	{
		const char *name(const int panel)
		{
			return this_vfunc<const char *>(this, 36u, panel);
		}

		void request_focus(const int panel)
		{
			this_vfunc<void>(this, 48u, panel, 0);
		}
	};

}  // namespace sdk

extern sdk::input_system *input_system;
extern sdk::surface *surface;
extern sdk::panel *panel;
