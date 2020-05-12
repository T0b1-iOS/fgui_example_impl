/*
 * This already defines all of the stuff to get it running, you just need to do two steps:
 * - edit helpers.hpp to look like this: https://i.imgur.com/AfQUAv3.png
 * - edit init_menu in menu.cpp to do what you want
 *
 * Key to open menu is insert by default and f7 to unload the dll
 */


#include "sdk.h"
#undef _WIN32  // stop the helper.h from """helping"""
#define FGUI_IMPLEMENTATION
namespace FGUI
{
	using FONT = unsigned long;
}

#include "fgui/FGUI/FGUI.hpp"
#define _WIN32

extern std::shared_ptr<FGUI::CForm> main_form;

// do stuff here
void init_menu()
{
	auto builder = FGUI::CBuilder{};
	main_form    = std::make_shared<FGUI::CForm>();
	builder.Widget(main_form)
	  .Title("Hello World!")
	  .Size(800, 600)
	  .Position(800, 200)
	  .Font("Tahoma", 12, true)
	  .Key(sdk::KEY_INSERT);

	auto tab = std::make_shared<FGUI::CTabs>();
	builder.Widget(tab)
	  .Title("MainTab")
	  .Font("Tahoma", 12, true)
	  .Spawn(main_form);
	tab = std::make_shared<FGUI::CTabs>();
	builder.Widget(tab)
	  .Title("MainTab2")
	  .Font("Tahoma", 12, true)
	  .Spawn(main_form);

	auto checkbox = std::make_shared<FGUI::CCheckBox>();
	builder.Widget(checkbox)
	  .Title("Test")
	  .Size(25, 25)
	  .Font("Tahoma", 12, true)
	  .Position(10, 10)
	  .Color(0, 0, 0)
	  .Spawn(tab);
}