/*************************************************************************/
/*  display_server_android.cpp                                           */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "display_server_android.h"

#include "android_keys_utils.h"
#include "core/config/project_settings.h"
#include "java_godot_io_wrapper.h"
#include "java_godot_wrapper.h"
#include "os_android.h"

#if defined(VULKAN_ENABLED)
#include "drivers/vulkan/rendering_device_vulkan.h"
#include "platform/android/vulkan/vulkan_context_android.h"
#include "servers/rendering/renderer_rd/renderer_compositor_rd.h"
#endif

DisplayServerAndroid *DisplayServerAndroid::get_singleton() {
	return (DisplayServerAndroid *)DisplayServer::get_singleton();
}

bool DisplayServerAndroid::has_feature(Feature p_feature) const {
	switch (p_feature) {
		//case FEATURE_CONSOLE_WINDOW:
		case FEATURE_CURSOR_SHAPE:
		//case FEATURE_CUSTOM_CURSOR_SHAPE:
		//case FEATURE_GLOBAL_MENU:
		//case FEATURE_HIDPI:
		//case FEATURE_ICON:
		//case FEATURE_IME:
		case FEATURE_MOUSE:
		//case FEATURE_MOUSE_WARP:
		//case FEATURE_NATIVE_DIALOG:
		//case FEATURE_NATIVE_ICON:
		//case FEATURE_WINDOW_TRANSPARENCY:
		case FEATURE_CLIPBOARD:
		case FEATURE_KEEP_SCREEN_ON:
		case FEATURE_ORIENTATION:
		case FEATURE_TOUCHSCREEN:
		case FEATURE_VIRTUAL_KEYBOARD:
			return true;
		default:
			return false;
	}
}

String DisplayServerAndroid::get_name() const {
	return "Android";
}

void DisplayServerAndroid::clipboard_set(const String &p_text) {
	GodotJavaWrapper *godot_java = OS_Android::get_singleton()->get_godot_java();
	ERR_FAIL_COND(!godot_java);

	if (godot_java->has_set_clipboard()) {
		godot_java->set_clipboard(p_text);
	} else {
		DisplayServer::clipboard_set(p_text);
	}
}

String DisplayServerAndroid::clipboard_get() const {
	GodotJavaWrapper *godot_java = OS_Android::get_singleton()->get_godot_java();
	ERR_FAIL_COND_V(!godot_java, String());

	if (godot_java->has_get_clipboard()) {
		return godot_java->get_clipboard();
	} else {
		return DisplayServer::clipboard_get();
	}
}

void DisplayServerAndroid::screen_set_keep_on(bool p_enable) {
	GodotJavaWrapper *godot_java = OS_Android::get_singleton()->get_godot_java();
	ERR_FAIL_COND(!godot_java);

	godot_java->set_keep_screen_on(p_enable);
	keep_screen_on = p_enable;
}

bool DisplayServerAndroid::screen_is_kept_on() const {
	return keep_screen_on;
}

void DisplayServerAndroid::screen_set_orientation(DisplayServer::ScreenOrientation p_orientation, int p_screen) {
	GodotIOJavaWrapper *godot_io_java = OS_Android::get_singleton()->get_godot_io_java();
	ERR_FAIL_COND(!godot_io_java);

	godot_io_java->set_screen_orientation(p_orientation);
}

DisplayServer::ScreenOrientation DisplayServerAndroid::screen_get_orientation(int p_screen) const {
	GodotIOJavaWrapper *godot_io_java = OS_Android::get_singleton()->get_godot_io_java();
	ERR_FAIL_COND_V(!godot_io_java, SCREEN_LANDSCAPE);

	return (ScreenOrientation)godot_io_java->get_screen_orientation();
}

int DisplayServerAndroid::get_screen_count() const {
	return 1;
}

Point2i DisplayServerAndroid::screen_get_position(int p_screen) const {
	return Point2i(0, 0);
}

Size2i DisplayServerAndroid::screen_get_size(int p_screen) const {
	return OS_Android::get_singleton()->get_display_size();
}

Rect2i DisplayServerAndroid::screen_get_usable_rect(int p_screen) const {
	GodotIOJavaWrapper *godot_io_java = OS_Android::get_singleton()->get_godot_io_java();
	ERR_FAIL_COND_V(!godot_io_java, Rect2i());
	int xywh[4];
	godot_io_java->screen_get_usable_rect(xywh);
	return Rect2i(xywh[0], xywh[1], xywh[2], xywh[3]);
}

int DisplayServerAndroid::screen_get_dpi(int p_screen) const {
	GodotIOJavaWrapper *godot_io_java = OS_Android::get_singleton()->get_godot_io_java();
	ERR_FAIL_COND_V(!godot_io_java, 0);

	return godot_io_java->get_screen_dpi();
}

bool DisplayServerAndroid::screen_is_touchscreen(int p_screen) const {
	return true;
}

void DisplayServerAndroid::virtual_keyboard_show(const String &p_existing_text, const Rect2 &p_screen_rect, bool p_multiline, int p_max_length, int p_cursor_start, int p_cursor_end) {
	GodotIOJavaWrapper *godot_io_java = OS_Android::get_singleton()->get_godot_io_java();
	ERR_FAIL_COND(!godot_io_java);

	if (godot_io_java->has_vk()) {
		godot_io_java->show_vk(p_existing_text, p_multiline, p_max_length, p_cursor_start, p_cursor_end);
	} else {
		ERR_PRINT("Virtual keyboard not available");
	}
}

void DisplayServerAndroid::virtual_keyboard_hide() {
	GodotIOJavaWrapper *godot_io_java = OS_Android::get_singleton()->get_godot_io_java();
	ERR_FAIL_COND(!godot_io_java);

	if (godot_io_java->has_vk()) {
		godot_io_java->hide_vk();
	} else {
		ERR_PRINT("Virtual keyboard not available");
	}
}

int DisplayServerAndroid::virtual_keyboard_get_height() const {
	GodotIOJavaWrapper *godot_io_java = OS_Android::get_singleton()->get_godot_io_java();
	ERR_FAIL_COND_V(!godot_io_java, 0);

	return godot_io_java->get_vk_height();
}

void DisplayServerAndroid::window_set_window_event_callback(const Callable &p_callable, DisplayServer::WindowID p_window) {
	window_event_callback = p_callable;
}

void DisplayServerAndroid::window_set_input_event_callback(const Callable &p_callable, DisplayServer::WindowID p_window) {
	input_event_callback = p_callable;
}

void DisplayServerAndroid::window_set_input_text_callback(const Callable &p_callable, DisplayServer::WindowID p_window) {
	input_text_callback = p_callable;
}

void DisplayServerAndroid::window_set_rect_changed_callback(const Callable &p_callable, DisplayServer::WindowID p_window) {
	rect_changed_callback = p_callable;
}

void DisplayServerAndroid::window_set_drop_files_callback(const Callable &p_callable, DisplayServer::WindowID p_window) {
	// Not supported on Android.
}

void DisplayServerAndroid::_window_callback(const Callable &p_callable, const Variant &p_arg) const {
	if (!p_callable.is_null()) {
		const Variant *argp = &p_arg;
		Variant ret;
		Callable::CallError ce;
		p_callable.call((const Variant **)&argp, 1, ret, ce);
	}
}

void DisplayServerAndroid::send_window_event(DisplayServer::WindowEvent p_event) const {
	_window_callback(window_event_callback, int(p_event));
}

void DisplayServerAndroid::send_input_event(const Ref<InputEvent> &p_event) const {
	_window_callback(input_event_callback, p_event);
}

void DisplayServerAndroid::send_input_text(const String &p_text) const {
	_window_callback(input_text_callback, p_text);
}

void DisplayServerAndroid::_dispatch_input_events(const Ref<InputEvent> &p_event) {
	DisplayServerAndroid::get_singleton()->send_input_event(p_event);
}

Vector<DisplayServer::WindowID> DisplayServerAndroid::get_window_list() const {
	Vector<WindowID> ret;
	ret.push_back(MAIN_WINDOW_ID);
	return ret;
}

DisplayServer::WindowID DisplayServerAndroid::get_window_at_screen_position(const Point2i &p_position) const {
	return MAIN_WINDOW_ID;
}

void DisplayServerAndroid::window_attach_instance_id(ObjectID p_instance, DisplayServer::WindowID p_window) {
	window_attached_instance_id = p_instance;
}

ObjectID DisplayServerAndroid::window_get_attached_instance_id(DisplayServer::WindowID p_window) const {
	return window_attached_instance_id;
}

void DisplayServerAndroid::window_set_title(const String &p_title, DisplayServer::WindowID p_window) {
	// Not supported on Android.
}

int DisplayServerAndroid::window_get_current_screen(DisplayServer::WindowID p_window) const {
	return SCREEN_OF_MAIN_WINDOW;
}

void DisplayServerAndroid::window_set_current_screen(int p_screen, DisplayServer::WindowID p_window) {
	// Not supported on Android.
}

Point2i DisplayServerAndroid::window_get_position(DisplayServer::WindowID p_window) const {
	return Point2i();
}

void DisplayServerAndroid::window_set_position(const Point2i &p_position, DisplayServer::WindowID p_window) {
	// Not supported on Android.
}

void DisplayServerAndroid::window_set_transient(DisplayServer::WindowID p_window, DisplayServer::WindowID p_parent) {
	// Not supported on Android.
}

void DisplayServerAndroid::window_set_max_size(const Size2i p_size, DisplayServer::WindowID p_window) {
	// Not supported on Android.
}

Size2i DisplayServerAndroid::window_get_max_size(DisplayServer::WindowID p_window) const {
	return Size2i();
}

void DisplayServerAndroid::window_set_min_size(const Size2i p_size, DisplayServer::WindowID p_window) {
	// Not supported on Android.
}

Size2i DisplayServerAndroid::window_get_min_size(DisplayServer::WindowID p_window) const {
	return Size2i();
}

void DisplayServerAndroid::window_set_size(const Size2i p_size, DisplayServer::WindowID p_window) {
	// Not supported on Android.
}

Size2i DisplayServerAndroid::window_get_size(DisplayServer::WindowID p_window) const {
	return OS_Android::get_singleton()->get_display_size();
}

Size2i DisplayServerAndroid::window_get_real_size(DisplayServer::WindowID p_window) const {
	return OS_Android::get_singleton()->get_display_size();
}

void DisplayServerAndroid::window_set_mode(DisplayServer::WindowMode p_mode, DisplayServer::WindowID p_window) {
	// Not supported on Android.
}

DisplayServer::WindowMode DisplayServerAndroid::window_get_mode(DisplayServer::WindowID p_window) const {
	return WINDOW_MODE_FULLSCREEN;
}

bool DisplayServerAndroid::window_is_maximize_allowed(DisplayServer::WindowID p_window) const {
	return false;
}

void DisplayServerAndroid::window_set_flag(DisplayServer::WindowFlags p_flag, bool p_enabled, DisplayServer::WindowID p_window) {
	// Not supported on Android.
}

bool DisplayServerAndroid::window_get_flag(DisplayServer::WindowFlags p_flag, DisplayServer::WindowID p_window) const {
	return false;
}

void DisplayServerAndroid::window_request_attention(DisplayServer::WindowID p_window) {
	// Not supported on Android.
}

void DisplayServerAndroid::window_move_to_foreground(DisplayServer::WindowID p_window) {
	// Not supported on Android.
}

bool DisplayServerAndroid::window_can_draw(DisplayServer::WindowID p_window) const {
	return true;
}

bool DisplayServerAndroid::can_any_window_draw() const {
	return true;
}

void DisplayServerAndroid::process_events() {
	Input::get_singleton()->flush_accumulated_events();
}

Vector<String> DisplayServerAndroid::get_rendering_drivers_func() {
	Vector<String> drivers;

#ifdef OPENGL_ENABLED
	drivers.push_back("opengl");
#endif
#ifdef VULKAN_ENABLED
	drivers.push_back("vulkan");
#endif

	return drivers;
}

DisplayServer *DisplayServerAndroid::create_func(const String &p_rendering_driver, DisplayServer::WindowMode p_mode, DisplayServer::VSyncMode p_vsync_mode, uint32_t p_flags, const Vector2i &p_resolution, Error &r_error) {
	DisplayServer *ds = memnew(DisplayServerAndroid(p_rendering_driver, p_mode, p_vsync_mode, p_flags, p_resolution, r_error));
	if (r_error != OK) {
		OS::get_singleton()->alert("Your video card driver does not support any of the supported Vulkan versions.", "Unable to initialize Video driver");
	}
	return ds;
}

void DisplayServerAndroid::register_android_driver() {
	register_create_function("android", create_func, get_rendering_drivers_func);
}

void DisplayServerAndroid::reset_window() {
#if defined(VULKAN_ENABLED)
	if (rendering_driver == "vulkan") {
		ANativeWindow *native_window = OS_Android::get_singleton()->get_native_window();
		ERR_FAIL_COND(!native_window);

		ERR_FAIL_COND(!context_vulkan);
		VSyncMode last_vsync_mode = context_vulkan->get_vsync_mode(MAIN_WINDOW_ID);
		context_vulkan->window_destroy(MAIN_WINDOW_ID);

		Size2i display_size = OS_Android::get_singleton()->get_display_size();
		if (context_vulkan->window_create(native_window, last_vsync_mode, display_size.width, display_size.height) == -1) {
			memdelete(context_vulkan);
			context_vulkan = nullptr;
			ERR_FAIL_MSG("Failed to reset Vulkan window.");
		}
	}
#endif
}

void DisplayServerAndroid::notify_surface_changed(int p_width, int p_height) {
	if (rect_changed_callback.is_null()) {
		return;
	}

	const Variant size = Rect2i(0, 0, p_width, p_height);
	const Variant *sizep = &size;
	Variant ret;
	Callable::CallError ce;

	rect_changed_callback.call(reinterpret_cast<const Variant **>(&sizep), 1, ret, ce);
}

DisplayServerAndroid::DisplayServerAndroid(const String &p_rendering_driver, DisplayServer::WindowMode p_mode, DisplayServer::VSyncMode p_vsync_mode, uint32_t p_flags, const Vector2i &p_resolution, Error &r_error) {
	rendering_driver = p_rendering_driver;

	// TODO: rendering_driver is broken, change when different drivers are supported again
	rendering_driver = "vulkan";

	keep_screen_on = GLOBAL_GET("display/window/energy_saving/keep_screen_on");

#if defined(OPENGL_ENABLED)
	if (rendering_driver == "opengl") {
		bool gl_initialization_error = false;

		if (RasterizerGLES2::is_viable() == OK) {
			RasterizerGLES2::register_config();
			RasterizerGLES2::make_current();
		} else {
			gl_initialization_error = true;
		}

		if (gl_initialization_error) {
			OS::get_singleton()->alert("Your device does not support any of the supported OpenGL versions.\n"
									   "Please try updating your Android version.",
					"Unable to initialize video driver");
			return;
		}
	}
#endif

#if defined(VULKAN_ENABLED)
	context_vulkan = nullptr;
	rendering_device_vulkan = nullptr;

	if (rendering_driver == "vulkan") {
		ANativeWindow *native_window = OS_Android::get_singleton()->get_native_window();
		ERR_FAIL_COND(!native_window);

		context_vulkan = memnew(VulkanContextAndroid);
		if (context_vulkan->initialize() != OK) {
			memdelete(context_vulkan);
			context_vulkan = nullptr;
			ERR_FAIL_MSG("Failed to initialize Vulkan context");
		}

		Size2i display_size = OS_Android::get_singleton()->get_display_size();
		if (context_vulkan->window_create(native_window, p_vsync_mode, display_size.width, display_size.height) == -1) {
			memdelete(context_vulkan);
			context_vulkan = nullptr;
			ERR_FAIL_MSG("Failed to create Vulkan window.");
		}

		rendering_device_vulkan = memnew(RenderingDeviceVulkan);
		rendering_device_vulkan->initialize(context_vulkan);

		RendererCompositorRD::make_current();
	}
#endif

	Input::get_singleton()->set_event_dispatch_function(_dispatch_input_events);

	r_error = OK;
}

DisplayServerAndroid::~DisplayServerAndroid() {
#if defined(VULKAN_ENABLED)
	if (rendering_driver == "vulkan") {
		if (rendering_device_vulkan) {
			rendering_device_vulkan->finalize();
			memdelete(rendering_device_vulkan);
		}

		if (context_vulkan) {
			memdelete(context_vulkan);
		}
	}
#endif
}

void DisplayServerAndroid::process_joy_event(DisplayServerAndroid::JoypadEvent p_event) {
	switch (p_event.type) {
		case JOY_EVENT_BUTTON:
			Input::get_singleton()->joy_button(p_event.device, (JoyButton)p_event.index, p_event.pressed);
			break;
		case JOY_EVENT_AXIS:
			Input::JoyAxisValue value;
			value.min = -1;
			value.value = p_event.value;
			Input::get_singleton()->joy_axis(p_event.device, (JoyAxis)p_event.index, value);
			break;
		case JOY_EVENT_HAT:
			Input::get_singleton()->joy_hat(p_event.device, (HatMask)p_event.hat);
			break;
		default:
			return;
	}
}

void DisplayServerAndroid::_set_key_modifier_state(Ref<InputEventWithModifiers> ev) {
	ev->set_shift_pressed(shift_mem);
	ev->set_alt_pressed(alt_mem);
	ev->set_meta_pressed(meta_mem);
	ev->set_ctrl_pressed(control_mem);
}

void DisplayServerAndroid::process_key_event(int p_keycode, int p_scancode, int p_unicode_char, bool p_pressed) {
	static char32_t prev_wc = 0;
	char32_t unicode = p_unicode_char;
	if ((p_unicode_char & 0xfffffc00) == 0xd800) {
		if (prev_wc != 0) {
			ERR_PRINT("invalid utf16 surrogate input");
		}
		prev_wc = unicode;
		return; // Skip surrogate.
	} else if ((unicode & 0xfffffc00) == 0xdc00) {
		if (prev_wc == 0) {
			ERR_PRINT("invalid utf16 surrogate input");
			return; // Skip invalid surrogate.
		}
		unicode = (prev_wc << 10UL) + unicode - ((0xd800 << 10UL) + 0xdc00 - 0x10000);
		prev_wc = 0;
	} else {
		prev_wc = 0;
	}

	Ref<InputEventKey> ev;
	ev.instantiate();
	int val = unicode;
	int keycode = android_get_keysym(p_keycode);
	int phy_keycode = android_get_keysym(p_scancode);

	if (keycode == KEY_SHIFT) {
		shift_mem = p_pressed;
	}
	if (keycode == KEY_ALT) {
		alt_mem = p_pressed;
	}
	if (keycode == KEY_CTRL) {
		control_mem = p_pressed;
	}
	if (keycode == KEY_META) {
		meta_mem = p_pressed;
	}

	ev->set_keycode(keycode);
	ev->set_physical_keycode(phy_keycode);
	ev->set_unicode(val);
	ev->set_pressed(p_pressed);

	_set_key_modifier_state(ev);

	if (val == '\n') {
		ev->set_keycode(KEY_ENTER);
	} else if (val == 61448) {
		ev->set_keycode(KEY_BACKSPACE);
		ev->set_unicode(KEY_BACKSPACE);
	} else if (val == 61453) {
		ev->set_keycode(KEY_ENTER);
		ev->set_unicode(KEY_ENTER);
	} else if (p_keycode == 4) {
		OS_Android::get_singleton()->main_loop_request_go_back();
	}

	Input::get_singleton()->accumulate_input_event(ev);
}

void DisplayServerAndroid::process_touch(int p_event, int p_pointer, const Vector<DisplayServerAndroid::TouchPos> &p_points) {
	switch (p_event) {
		case AMOTION_EVENT_ACTION_DOWN: { //gesture begin
			if (touch.size()) {
				//end all if exist
				for (int i = 0; i < touch.size(); i++) {
					Ref<InputEventScreenTouch> ev;
					ev.instantiate();
					ev->set_index(touch[i].id);
					ev->set_pressed(false);
					ev->set_position(touch[i].pos);
					Input::get_singleton()->accumulate_input_event(ev);
				}
			}

			touch.resize(p_points.size());
			for (int i = 0; i < p_points.size(); i++) {
				touch.write[i].id = p_points[i].id;
				touch.write[i].pos = p_points[i].pos;
			}

			//send touch
			for (int i = 0; i < touch.size(); i++) {
				Ref<InputEventScreenTouch> ev;
				ev.instantiate();
				ev->set_index(touch[i].id);
				ev->set_pressed(true);
				ev->set_position(touch[i].pos);
				Input::get_singleton()->accumulate_input_event(ev);
			}

		} break;
		case AMOTION_EVENT_ACTION_MOVE: { //motion
			ERR_FAIL_COND(touch.size() != p_points.size());

			for (int i = 0; i < touch.size(); i++) {
				int idx = -1;
				for (int j = 0; j < p_points.size(); j++) {
					if (touch[i].id == p_points[j].id) {
						idx = j;
						break;
					}
				}

				ERR_CONTINUE(idx == -1);

				if (touch[i].pos == p_points[idx].pos)
					continue; //no move unncesearily

				Ref<InputEventScreenDrag> ev;
				ev.instantiate();
				ev->set_index(touch[i].id);
				ev->set_position(p_points[idx].pos);
				ev->set_relative(p_points[idx].pos - touch[i].pos);
				Input::get_singleton()->accumulate_input_event(ev);
				touch.write[i].pos = p_points[idx].pos;
			}

		} break;
		case AMOTION_EVENT_ACTION_CANCEL:
		case AMOTION_EVENT_ACTION_UP: { //release
			if (touch.size()) {
				//end all if exist
				for (int i = 0; i < touch.size(); i++) {
					Ref<InputEventScreenTouch> ev;
					ev.instantiate();
					ev->set_index(touch[i].id);
					ev->set_pressed(false);
					ev->set_position(touch[i].pos);
					Input::get_singleton()->accumulate_input_event(ev);
				}
				touch.clear();
			}
		} break;
		case AMOTION_EVENT_ACTION_POINTER_DOWN: { // add touch
			for (int i = 0; i < p_points.size(); i++) {
				if (p_points[i].id == p_pointer) {
					TouchPos tp = p_points[i];
					touch.push_back(tp);

					Ref<InputEventScreenTouch> ev;
					ev.instantiate();

					ev->set_index(tp.id);
					ev->set_pressed(true);
					ev->set_position(tp.pos);
					Input::get_singleton()->accumulate_input_event(ev);

					break;
				}
			}
		} break;
		case AMOTION_EVENT_ACTION_POINTER_UP: { // remove touch
			for (int i = 0; i < touch.size(); i++) {
				if (touch[i].id == p_pointer) {
					Ref<InputEventScreenTouch> ev;
					ev.instantiate();
					ev->set_index(touch[i].id);
					ev->set_pressed(false);
					ev->set_position(touch[i].pos);
					Input::get_singleton()->accumulate_input_event(ev);
					touch.remove(i);

					break;
				}
			}
		} break;
	}
}

void DisplayServerAndroid::process_hover(int p_type, Point2 p_pos) {
	// https://developer.android.com/reference/android/view/MotionEvent.html#ACTION_HOVER_ENTER
	switch (p_type) {
		case AMOTION_EVENT_ACTION_HOVER_MOVE: // hover move
		case AMOTION_EVENT_ACTION_HOVER_ENTER: // hover enter
		case AMOTION_EVENT_ACTION_HOVER_EXIT: { // hover exit
			Ref<InputEventMouseMotion> ev;
			ev.instantiate();
			_set_key_modifier_state(ev);
			ev->set_position(p_pos);
			ev->set_global_position(p_pos);
			ev->set_relative(p_pos - hover_prev_pos);
			Input::get_singleton()->accumulate_input_event(ev);
			hover_prev_pos = p_pos;
		} break;
	}
}

void DisplayServerAndroid::process_mouse_event(int input_device, int event_action, int event_android_buttons_mask, Point2 event_pos, float event_vertical_factor, float event_horizontal_factor) {
	MouseButton event_buttons_mask = _android_button_mask_to_godot_button_mask(event_android_buttons_mask);
	switch (event_action) {
		case AMOTION_EVENT_ACTION_BUTTON_PRESS:
		case AMOTION_EVENT_ACTION_BUTTON_RELEASE: {
			Ref<InputEventMouseButton> ev;
			ev.instantiate();
			_set_key_modifier_state(ev);
			if ((input_device & AINPUT_SOURCE_MOUSE) == AINPUT_SOURCE_MOUSE) {
				ev->set_position(event_pos);
				ev->set_global_position(event_pos);
			} else {
				ev->set_position(hover_prev_pos);
				ev->set_global_position(hover_prev_pos);
			}
			ev->set_pressed(event_action == AMOTION_EVENT_ACTION_BUTTON_PRESS);
			MouseButton changed_button_mask = MouseButton(buttons_state ^ event_buttons_mask);

			buttons_state = event_buttons_mask;

			ev->set_button_index(_button_index_from_mask(changed_button_mask));
			ev->set_button_mask(event_buttons_mask);
			Input::get_singleton()->accumulate_input_event(ev);
		} break;

		case AMOTION_EVENT_ACTION_MOVE: {
			Ref<InputEventMouseMotion> ev;
			ev.instantiate();
			_set_key_modifier_state(ev);
			if ((input_device & AINPUT_SOURCE_MOUSE) == AINPUT_SOURCE_MOUSE) {
				ev->set_position(event_pos);
				ev->set_global_position(event_pos);
				ev->set_relative(event_pos - hover_prev_pos);
				hover_prev_pos = event_pos;
			} else {
				ev->set_position(hover_prev_pos);
				ev->set_global_position(hover_prev_pos);
				ev->set_relative(event_pos);
			}
			ev->set_button_mask(event_buttons_mask);
			Input::get_singleton()->accumulate_input_event(ev);
		} break;
		case AMOTION_EVENT_ACTION_SCROLL: {
			Ref<InputEventMouseButton> ev;
			ev.instantiate();
			if ((input_device & AINPUT_SOURCE_MOUSE) == AINPUT_SOURCE_MOUSE) {
				ev->set_position(event_pos);
				ev->set_global_position(event_pos);
			} else {
				ev->set_position(hover_prev_pos);
				ev->set_global_position(hover_prev_pos);
			}
			ev->set_pressed(true);
			buttons_state = event_buttons_mask;
			if (event_vertical_factor > 0) {
				_wheel_button_click(event_buttons_mask, ev, MOUSE_BUTTON_WHEEL_UP, event_vertical_factor);
			} else if (event_vertical_factor < 0) {
				_wheel_button_click(event_buttons_mask, ev, MOUSE_BUTTON_WHEEL_DOWN, -event_vertical_factor);
			}

			if (event_horizontal_factor > 0) {
				_wheel_button_click(event_buttons_mask, ev, MOUSE_BUTTON_WHEEL_RIGHT, event_horizontal_factor);
			} else if (event_horizontal_factor < 0) {
				_wheel_button_click(event_buttons_mask, ev, MOUSE_BUTTON_WHEEL_LEFT, -event_horizontal_factor);
			}
		} break;
	}
}

void DisplayServerAndroid::_wheel_button_click(MouseButton event_buttons_mask, const Ref<InputEventMouseButton> &ev, MouseButton wheel_button, float factor) {
	Ref<InputEventMouseButton> evd = ev->duplicate();
	_set_key_modifier_state(evd);
	evd->set_button_index(wheel_button);
	evd->set_button_mask(MouseButton(event_buttons_mask ^ (1 << (wheel_button - 1))));
	evd->set_factor(factor);
	Input::get_singleton()->accumulate_input_event(evd);
	Ref<InputEventMouseButton> evdd = evd->duplicate();
	evdd->set_pressed(false);
	evdd->set_button_mask(event_buttons_mask);
	Input::get_singleton()->accumulate_input_event(evdd);
}

void DisplayServerAndroid::process_double_tap(int event_android_button_mask, Point2 p_pos) {
	MouseButton event_button_mask = _android_button_mask_to_godot_button_mask(event_android_button_mask);
	Ref<InputEventMouseButton> ev;
	ev.instantiate();
	_set_key_modifier_state(ev);
	ev->set_position(p_pos);
	ev->set_global_position(p_pos);
	ev->set_pressed(event_button_mask != 0);
	ev->set_button_index(_button_index_from_mask(event_button_mask));
	ev->set_button_mask(event_button_mask);
	ev->set_double_click(true);
	Input::get_singleton()->accumulate_input_event(ev);
}

MouseButton DisplayServerAndroid::_button_index_from_mask(MouseButton button_mask) {
	switch (button_mask) {
		case MOUSE_BUTTON_MASK_LEFT:
			return MOUSE_BUTTON_LEFT;
		case MOUSE_BUTTON_MASK_RIGHT:
			return MOUSE_BUTTON_RIGHT;
		case MOUSE_BUTTON_MASK_MIDDLE:
			return MOUSE_BUTTON_MIDDLE;
		case MOUSE_BUTTON_MASK_XBUTTON1:
			return MOUSE_BUTTON_XBUTTON1;
		case MOUSE_BUTTON_MASK_XBUTTON2:
			return MOUSE_BUTTON_XBUTTON2;
		default:
			return MOUSE_BUTTON_NONE;
	}
}

void DisplayServerAndroid::process_scroll(Point2 p_pos) {
	Ref<InputEventPanGesture> ev;
	ev.instantiate();
	_set_key_modifier_state(ev);
	ev->set_position(p_pos);
	ev->set_delta(p_pos - scroll_prev_pos);
	Input::get_singleton()->accumulate_input_event(ev);
	scroll_prev_pos = p_pos;
}

void DisplayServerAndroid::process_accelerometer(const Vector3 &p_accelerometer) {
	Input::get_singleton()->set_accelerometer(p_accelerometer);
}

void DisplayServerAndroid::process_gravity(const Vector3 &p_gravity) {
	Input::get_singleton()->set_gravity(p_gravity);
}

void DisplayServerAndroid::process_magnetometer(const Vector3 &p_magnetometer) {
	Input::get_singleton()->set_magnetometer(p_magnetometer);
}

void DisplayServerAndroid::process_gyroscope(const Vector3 &p_gyroscope) {
	Input::get_singleton()->set_gyroscope(p_gyroscope);
}

void DisplayServerAndroid::mouse_set_mode(MouseMode p_mode) {
	if (mouse_mode == p_mode) {
		return;
	}

	if (p_mode == MouseMode::MOUSE_MODE_HIDDEN) {
		OS_Android::get_singleton()->get_godot_java()->get_godot_view()->set_pointer_icon(CURSOR_TYPE_NULL);
	} else {
		cursor_set_shape(cursor_shape);
	}

	if (p_mode == MouseMode::MOUSE_MODE_CAPTURED) {
		OS_Android::get_singleton()->get_godot_java()->get_godot_view()->request_pointer_capture();
	} else {
		OS_Android::get_singleton()->get_godot_java()->get_godot_view()->release_pointer_capture();
	}

	mouse_mode = p_mode;
}

DisplayServer::MouseMode DisplayServerAndroid::mouse_get_mode() const {
	return mouse_mode;
}

Point2i DisplayServerAndroid::mouse_get_position() const {
	return hover_prev_pos;
}

MouseButton DisplayServerAndroid::mouse_get_button_state() const {
	return buttons_state;
}

MouseButton DisplayServerAndroid::_android_button_mask_to_godot_button_mask(int android_button_mask) {
	MouseButton godot_button_mask = MOUSE_BUTTON_NONE;
	if (android_button_mask & AMOTION_EVENT_BUTTON_PRIMARY) {
		godot_button_mask |= MOUSE_BUTTON_MASK_LEFT;
	}
	if (android_button_mask & AMOTION_EVENT_BUTTON_SECONDARY) {
		godot_button_mask |= MOUSE_BUTTON_MASK_RIGHT;
	}
	if (android_button_mask & AMOTION_EVENT_BUTTON_TERTIARY) {
		godot_button_mask |= MOUSE_BUTTON_MASK_MIDDLE;
	}
	if (android_button_mask & AMOTION_EVENT_BUTTON_BACK) {
		godot_button_mask |= MOUSE_BUTTON_MASK_XBUTTON1;
	}
	if (android_button_mask & AMOTION_EVENT_BUTTON_SECONDARY) {
		godot_button_mask |= MOUSE_BUTTON_MASK_XBUTTON2;
	}

	return godot_button_mask;
}

void DisplayServerAndroid::cursor_set_shape(DisplayServer::CursorShape p_shape) {
	if (cursor_shape == p_shape) {
		return;
	}

	cursor_shape = p_shape;

	if (mouse_mode == MouseMode::MOUSE_MODE_VISIBLE || mouse_mode == MouseMode::MOUSE_MODE_CONFINED) {
		OS_Android::get_singleton()->get_godot_java()->get_godot_view()->set_pointer_icon(android_cursors[cursor_shape]);
	}
}

DisplayServer::CursorShape DisplayServerAndroid::cursor_get_shape() const {
	return cursor_shape;
}

void DisplayServerAndroid::window_set_vsync_mode(DisplayServer::VSyncMode p_vsync_mode, WindowID p_window) {
#if defined(VULKAN_ENABLED)
	context_vulkan->set_vsync_mode(p_window, p_vsync_mode);
#endif
}

DisplayServer::VSyncMode DisplayServerAndroid::window_get_vsync_mode(WindowID p_window) const {
#if defined(VULKAN_ENABLED)
	return context_vulkan->get_vsync_mode(p_window);
#else
	return DisplayServer::VSYNC_ENABLED;
#endif
}
