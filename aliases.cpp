#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#undef NOMINMAX
#undef WIN32_LEAN_AND_MEAN

#undef CreateFont

#include "sdk.h"
#include "vmt.h"

#include <array>
#include <cassert>

#undef _WIN32  // stop the helper.h from """helping"""
#define FGUI_IMPLEMENTATION
namespace FGUI
{
	using FONT = unsigned long;
}

#include "fgui/FGUI/FGUI.hpp"
#define _WIN32

namespace
{
	void create_font(FGUI::FONT &, const std::string &, int, int, bool);
	const FGUI::DIMENSION &screen_size();
	const FGUI::DIMENSION &text_size(FGUI::FONT &, const std::string &);
	void draw_rect(int, int, int, int, const FGUI::COLOR &);
	void draw_outlined_rect(int, int, int, int, const FGUI::COLOR &);
	void draw_gradient(int, int, int, int, size_t, size_t, const FGUI::COLOR &,
	                   bool);
	void draw_line(int, int, int, int, const FGUI::COLOR &);
	void draw_text(int, int, FGUI::FONT, const FGUI::COLOR &,
	               const std::string &);

	void poll_input();
	bool key_state(int);
	bool key_press(int);
	bool key_release(int);
	const FGUI::POINT &cursor_pos();
	const FGUI::POINT &cursor_delta();
	int cursor_wheel_delta();
	bool cursor_in_area(const FGUI::AREA &);
}  // namespace

void init_aliases()
{
	using namespace FGUI;

	RENDER.CreateFont    = &create_font;
	RENDER.GetScreenSize = &screen_size;
	RENDER.GetTextSize   = &text_size;
	RENDER.Rectangle     = &draw_rect;
	RENDER.Outline       = &draw_outlined_rect;
	RENDER.Gradient      = &draw_gradient;
	RENDER.Line          = &draw_line;
	RENDER.Text          = &draw_text;

	FGUI::INPUT.PullInput           = &poll_input;
	FGUI::INPUT.GetKeyState         = &key_state;
	FGUI::INPUT.GetKeyPress         = &key_press;
	FGUI::INPUT.GetKeyRelease       = &key_release;
	FGUI::INPUT.GetCursorPos        = &cursor_pos;
	FGUI::INPUT.GetCursorPosDelta   = &cursor_delta;
	FGUI::INPUT.GetCursorWheelDelta = &cursor_wheel_delta;
	FGUI::INPUT.IsCursorInArea      = &cursor_in_area;
}

// render
namespace
{
	void create_font(FGUI::FONT &font, const std::string &name, const int size,
	                 const int flags, const bool bold)
	{
		font = surface->create_font();
		surface->set_font_glyph_set(font, name.c_str(), size, bold ? 800 : 400, 0,
		                            0, flags);
	}

	const FGUI::DIMENSION &screen_size()
	{
		static auto size = FGUI::DIMENSION{};  // just... why
		surface->screen_size(size.m_iWidth, size.m_iHeight);
		return size;
	}

	const FGUI::DIMENSION &text_size(FGUI::FONT &font, const std::string &text)
	{
		static auto size = FGUI::DIMENSION{};
		wchar_t buf[256];
		const auto res = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, buf,
		                                     sizeof(buf) / sizeof(wchar_t));
		if (!res)
		{
			size.m_iWidth  = 0;
			size.m_iHeight = 0;
		} else
		{
			surface->text_size(font, buf, size.m_iWidth, size.m_iHeight);
		}

		return size;
	}

	void draw_rect(const int x, const int y, const int width, const int height,
	               const FGUI::COLOR &col)
	{
		surface->draw_set_color(col.m_ucRed, col.m_ucGreen, col.m_ucBlue,
		                        col.m_ucAlpha);
		surface->draw_filled_rect(x, y, x + width, y + height);
	}

	void draw_outlined_rect(const int x, const int y, const int width,
	                        const int height, const FGUI::COLOR &col)
	{
		surface->draw_set_color(col.m_ucRed, col.m_ucGreen, col.m_ucBlue,
		                        col.m_ucAlpha);
		surface->draw_outlined_rect(x, y, x + width, y + height);
	}

	void draw_gradient(int x, int y, int w, int h, size_t, size_t,
	                   const FGUI::COLOR &, bool)
	{
		assert(0);
	}

	void draw_line(const int x, const int y, const int width, const int height,
	               const FGUI::COLOR &col)
	{
		surface->draw_set_color(col.m_ucRed, col.m_ucGreen, col.m_ucBlue,
		                        col.m_ucAlpha);
		surface->draw_line(x, y, x + width, y + height);
	}

	void draw_text(const int x, const int y, const FGUI::FONT font,
	               const FGUI::COLOR &col, const std::string &text)
	{
		wchar_t buf[256];
		const auto res = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, buf,
		                                     sizeof(buf) / sizeof(wchar_t));
		if (!res)
		{
			return;
		}

		surface->draw_set_text_font(font);
		surface->draw_set_text_color(col.m_ucRed, col.m_ucGreen, col.m_ucBlue,
		                             col.m_ucAlpha);
		surface->draw_set_text_pos(x, y);
		surface->draw_text(buf);
	}
}  // namespace

// input
namespace
{
	std::array<bool, sdk::MOUSE_LAST> key_state_map{}, old_key_state_map{};
	FGUI::POINT mouse_pos, mouse_delta;

	void poll_input()
	{
		old_key_state_map = key_state_map;
		for (auto i = 0u; i < key_state_map.size(); ++i)
			key_state_map[i] =
			  input_system->button_down(static_cast<sdk::BUTTON_CODE>(i));

		POINT cursor_pos;

		GetCursorPos(&cursor_pos);
		ScreenToClient(reinterpret_cast<HWND>(input_system->window()), &cursor_pos);

		const auto old_mouse_pos = mouse_pos;
		mouse_pos.m_iX           = cursor_pos.x;
		mouse_pos.m_iY           = cursor_pos.y;

		mouse_delta.m_iX = mouse_pos.m_iX - old_mouse_pos.m_iX;
		mouse_delta.m_iY = mouse_pos.m_iY - old_mouse_pos.m_iY;
	}

	bool key_state(const int key) { return key_state_map[key]; }

	bool key_press(const int key)
	{
		return key_state_map[key] && !old_key_state_map[key];
	}

	bool key_release(const int key)
	{
		return !key_state_map[key] && old_key_state_map[key];
	}

	const FGUI::POINT &cursor_pos() { return mouse_pos; }

	const FGUI::POINT &cursor_delta() { return mouse_delta; }

	int cursor_wheel_delta()
	{
		assert(0);
		return 0;
	}

	bool cursor_in_area(const FGUI::AREA &area)  // seems like lazy programming
	{
		// this """"AREA"""" is actually not containing what the variables say it does
		// it contains top left, top right and width, height for some obscure reason
		return area.m_iLeft <= mouse_pos.m_iX && area.m_iTop <= mouse_pos.m_iY
		       && area.m_iRight + area.m_iLeft >= mouse_pos.m_iX
		       && area.m_iBottom + area.m_iTop >= mouse_pos.m_iY;
	}
}  // namespace
