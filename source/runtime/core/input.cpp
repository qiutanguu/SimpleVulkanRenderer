#include "input.h"

namespace flower { namespace input{

	glm::vec2 current_mouse_pos = glm::vec2(0.0f);
	bool left_mouse_button_down = false;
	bool right_mouse_button_down = false;
	bool key_grave_accent_down = false;
	bool key_tab = false;
	bool disable_cursor = true;
} }