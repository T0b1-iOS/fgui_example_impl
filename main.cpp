/*
 * This already defines all of the stuff to get it running, you just need to do two steps:
 * - edit helpers.hpp to look like this: https://i.imgur.com/AfQUAv3.png
 * - edit init_menu in menu.cpp to do what you want
 *
 * Key to open menu is insert by default and f7 to unload the dll
 */

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#undef NOMINMAX
#undef WIN32_LEAN_AND_MEAN

#include "sdk.h"
#include "vmt.h"

#include <thread>

#undef _WIN32  // stop the helper.h from """helping"""
#define FGUI_IMPLEMENTATION
namespace FGUI
{
	using FONT = unsigned long;
}

#include "fgui/FGUI/FGUI.hpp"
#define _WIN32

extern void init_aliases();
extern void init_menu();

bool unloading     = false;
HINSTANCE instance = nullptr;

sdk::input_system *input_system = nullptr;
sdk::surface *surface           = nullptr;
sdk::panel *panel               = nullptr;

util::vmt_hook vmt_hook;
constexpr size_t PAINT_TRAVERSE_IDX = 41u;

std::shared_ptr<FGUI::CForm> main_form = nullptr;

static void init_interfaces()
{
	const auto interface_ptr = [](const char *module, const char *interface) {
		const auto mod = GetModuleHandleA(module);
		const auto create_interface =
		  reinterpret_cast<void *(*)(const char *, int *)>(
		    GetProcAddress(mod, "CreateInterface"));

		return create_interface(interface, nullptr);
	};

	input_system = reinterpret_cast<sdk::input_system *>(
	  interface_ptr("inputsystem.dll", "InputSystemVersion001"));
	surface = reinterpret_cast<sdk::surface *>(
	  interface_ptr("vguimatsurface.dll", "VGUI_Surface031"));
	panel =
	  reinterpret_cast<sdk::panel *>(interface_ptr("vgui2.dll", "VGUI_Panel009"));
}

void unload()
{
	vmt_hook.unhook();
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	OutputDebugStringA("Unloading...\n");
	FreeLibrary(instance);
	FreeLibraryAndExitThread(instance, 0);
}

struct vgui_panel
{
	char pad_0000[24];     //0x0000
	void *parent;          //0x0018
	char pad_001C[4];      //0x001C
	int32_t panel_handle;  //0x0020
	void *client_panel;    //0x0024
	int16_t pos[2];        //0x0028
	int16_t size[2];       //0x002C
	int16_t min_size[2];   //0x0030
	int16_t inset[4];      //0x0034
	int16_t clip_rect[4];  //0x003C
	int16_t abs_pos[2];    //0x0044
	int16_t z_pos;         //0x0048
	bool _visible : 1;
	bool _enabled : 1;
	bool _popup : 1;
	bool _mouseInput : 1;  // used for popups
	bool _kbInput : 1;
	bool _isTopmostPopup : 1;
	char pad_004C[64];  //0x004C
};                    //Size: 0x008C

static void __fastcall paint_traverse_hook(sdk::panel *thisptr, void *,
                                           const int panel,
                                           const bool force_repaint,
                                           const bool allow_force)
{
	static const auto orig_func =
	  reinterpret_cast<sdk::this_call<void, int, bool, bool>>(
	    vmt_hook.orig_func(PAINT_TRAVERSE_IDX));

	orig_func(thisptr, panel, force_repaint, allow_force);

	const auto panel_name = thisptr->name(panel);
	if (main_form && panel_name
	    && std::string_view{panel_name} == "FocusOverlayPanel")
	{
		const auto ptr = reinterpret_cast<vgui_panel *>(panel);
		if (main_form->GetState())
		{
			ptr->_mouseInput = true;
			ptr->_kbInput    = true;
			thisptr->request_focus(panel);
			ptr->_mouseInput = true;
			ptr->_kbInput    = true;
		} else
		{
			ptr->_mouseInput = false;
			ptr->_kbInput    = false;
		}
		main_form->Render();

		if (!unloading && GetAsyncKeyState(VK_F7))
		{
			unloading = true;
			auto t    = std::thread(unload);
			t.detach();
		}
	}
}

void main_thread(const HINSTANCE mod_instance)
{
	instance = mod_instance;
	init_interfaces();
	init_aliases();
	init_menu();

	vmt_hook.init(panel);
	vmt_hook.hook_func(PAINT_TRAVERSE_IDX, &paint_traverse_hook);
}

BOOL WINAPI DllMain(const HINSTANCE instance, const DWORD reason,
                    LPVOID reserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		CreateThread(nullptr, 0, LPTHREAD_START_ROUTINE(main_thread), instance, 0,
		             nullptr);
	}
	return TRUE;
}
