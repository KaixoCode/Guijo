#include "Guijo/Window/Window.hpp"
using namespace Guijo;

Window::Window(const Construct& c) {
	createWindow(c);
};

double Window::x() const { return Object::x(); }
double Window::y() const { return Object::y(); }
double Window::width() const { return Object::width(); }
double Window::height() const { return Object::height(); }
void Window::x(const double& v) { Object::x(v); }
void Window::y(const double& v) { Object::y(v); }
void Window::width(const double& v) { Object::width(v); }
void Window::height(const double& v) { Object::height(v); }

bool Window::createWindow(const Construct& c) {
	m_Id = windowId++;

	auto _hInstance = GetModuleHandle(0);

	m_WindowClass.style = CS_OWNDC;
	m_WindowClass.lpfnWndProc = windowProc;
	m_WindowClass.hInstance = _hInstance;
	m_WindowClass.lpszClassName = c.name.data();

	RegisterClass(&m_WindowClass);

	int _x = c.dimensions.x() < 0 ? CW_USEDEFAULT : (int)c.dimensions.x();
	int _y = c.dimensions.y() < 0 ? CW_USEDEFAULT : (int)c.dimensions.y();
	int _w = c.dimensions.width() < 0 ? CW_USEDEFAULT : (int)c.dimensions.width();
	int _h = c.dimensions.height() < 0 ? CW_USEDEFAULT : (int)c.dimensions.height();

	m_Handle = CreateWindowEx(
		CS_OWNDC,                         // Optional window styles.
		c.name.data(),                    // Window class
		c.name.data(),                    // Window text
		WS_OVERLAPPEDWINDOW | WS_VISIBLE, // Window style
		_x, _y, _w, _h,
		NULL,       // Parent window    
		NULL,       // Menu
		_hInstance, // Instance handle
		NULL        // Additional application data
	);

	if (m_Handle == NULL) return false;
	else {
		SetWindowSubclass(m_Handle, windowSubProc, m_Id, (DWORD_PTR)this);
		SendMessage(m_Handle, WM_CREATE, 0, 0);
		return true;
	}
}

bool Window::loop() {
	MSG msg;
	if (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		windowsLoop();
		return true;
	}
	else return false;
}

void Window::windowsLoop() {
	m_Graphics.prepare();
	DrawContext context;
	update();
	draw(context);
	m_Graphics.render(context);

	// Go through the even queue
	while (!m_EventQueue.empty())
	{
		handle(*m_EventQueue.front());

		// Check again if the eventqueue is empty, because some button somewhere
		// might trigger a resize of the window, which will trigger this Window::Update
		// which will finish this while loop, and then exit out of AddEvent about here ^
		// and then m_EventQueue will be empty, causing an exception when calling pop().
		if (!m_EventQueue.empty())
			m_EventQueue.pop();
	}
}

void Window::cursorEvent(double x, double y, KeyMod mod) {
	cursor.position[0] = x, cursor.position[1] = y;
	if (cursor.buttons == MouseButton::None) m_EventQueue.emplace(new MouseMove{ { x, y } });
	else m_EventQueue.emplace(new MouseDrag{ cursor.pressed, { x, y }, cursor.buttons, mod });
}

void Window::mouseButtonEvent(MouseButton button, bool press, KeyMod mod) {
	if (press) {
		cursor.buttons |= button; // Add button to pressed
		cursor.pressed = cursor.position; // Set press cursor position
		m_EventQueue.emplace(new MousePress{ cursor.position, button, mod });
	} else {
		cursor.buttons ^= button; // Remove button from pressed
		if (cursor.pressed == cursor.position) // If pos not changed, add click
			m_EventQueue.emplace(new MouseClick{ cursor.position, button, mod });

		m_EventQueue.emplace(new MouseRelease{ cursor.position, button, mod });
	}
}

void Window::mouseWheelEvent(std::int16_t amount, KeyMod mod, double x, double y) {
	m_EventQueue.emplace(new MouseWheel{ { x, y }, amount, mod });
}

void Window::keyEvent(KeyCode key, bool repeat, int action, KeyMod mod) {
	if (action == 0) // Release
		m_EventQueue.emplace(new KeyRelease{ key, mod });
	else if (action == 1) // Press
		m_EventQueue.emplace(new KeyPress{ key, mod, repeat });
	else {
		std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conversion;
		m_EventQueue.emplace(new KeyType{ conversion.to_bytes(key)[0], mod, ((char)key) == 0xffff });
	}
}

void Window::resizeEvent(Dimensions dims) {
	dimensions(dims);
	//graphics.dimensions(dims); TODO:
	//graphics->SetProjection(glm::ortho(0.0f, (float)std::max(width, 5), 0.0f, (float)std::max(height, 5)));
}

LRESULT Window::windowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

KeyMod Window::getCurrentKeyMod() {
	constexpr uint16_t _check = 0x8000;
	const bool _rctrl = static_cast<bool>(GetKeyState(VK_RCONTROL) & _check);
	const bool _lctrl = static_cast<bool>(GetKeyState(VK_LCONTROL) & _check);
	const bool _rmenu = static_cast<bool>(GetKeyState(VK_RMENU) & _check);
	const bool _lmenu = static_cast<bool>(GetKeyState(VK_LMENU) & _check);
	const bool _rshft = static_cast<bool>(GetKeyState(VK_RSHIFT) & _check);
	const bool _lshft = static_cast<bool>(GetKeyState(VK_LSHIFT) & _check);
	const bool _nmlck = static_cast<bool>(GetKeyState(VK_NUMLOCK) & _check);
	const bool _cpslk = static_cast<bool>(GetKeyState(VK_CAPITAL) & _check);

	KeyMod _keymod{};
	if (_rctrl || _lctrl) _keymod |= Mods::Control;
	if (_rmenu || _lmenu) _keymod |= Mods::Alt;
	if (_rshft || _lshft) _keymod |= Mods::Shift;
	if (_rctrl) _keymod |= Mods::RightControl;
	if (_lctrl) _keymod |= Mods::LeftControl;
	if (_rmenu) _keymod |= Mods::RightAlt;
	if (_lmenu) _keymod |= Mods::LeftAlt;
	if (_rshft) _keymod |= Mods::RightShift;
	if (_lshft) _keymod |= Mods::LeftShift;
	if (_nmlck) _keymod |= Mods::NumLock;
	if (_cpslk) _keymod |= Mods::CapsLock;
	return _keymod;
}

LRESULT Window::windowSubProc(HWND hwnd, UINT msg, WPARAM wparam,
	LPARAM lparam, UINT_PTR, DWORD_PTR self) {
	Window* _self = reinterpret_cast<Window*>(self);
	if (_self == nullptr) return 0;

	switch (msg) {
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN: {
		SetCapture(hwnd);
		MouseButton _button =
			msg == WM_LBUTTONDOWN ? MouseButton::Left :
			msg == WM_RBUTTONDOWN ? MouseButton::Right :
			msg == WM_MBUTTONDOWN ? MouseButton::Middle : MouseButton::None;
		_self->mouseButtonEvent(_button, true, getCurrentKeyMod());
		break;
	}
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP: {
		MouseButton _button =
			msg == WM_LBUTTONUP ? MouseButton::Left :
			msg == WM_RBUTTONUP ? MouseButton::Right :
			msg == WM_MBUTTONUP ? MouseButton::Middle : MouseButton::None;
		_self->mouseButtonEvent(_button, false, getCurrentKeyMod());
		ReleaseCapture();
		break;
	}
	case WM_MOUSEMOVE: {
		double x = static_cast<double>(GET_X_LPARAM(lparam));
		double y = static_cast<double>(GET_Y_LPARAM(lparam));
		_self->cursorEvent(x, y, getCurrentKeyMod());
		break;
	}
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSCHAR:
	case WM_CHAR: {
		bool _repeat = (lparam & 0x40000000);
		int _type = 0; // Release
		if (msg == WM_SYSKEYDOWN || msg == WM_KEYDOWN) _type = 1; // Press
		else if (msg == WM_CHAR || msg == WM_SYSCHAR) _type = 2; // Type
		_self->keyEvent(static_cast<KeyCode>(wparam),
			_repeat, _type, getCurrentKeyMod());
		break;
	}
	case WM_MOUSEWHEEL: {
		RECT _rect;
		double _x = 0, _y = 0;
		if (GetWindowRect(hwnd, &_rect)) {
			_x = static_cast<double>(_rect.left);
			_y = static_cast<double>(_rect.top);
		}

		std::int16_t _amount = GET_WHEEL_DELTA_WPARAM(wparam);
		double _xPos = static_cast<double>(GET_X_LPARAM(lparam));
		double _yPos = static_cast<double>(GET_Y_LPARAM(lparam));
		_self->mouseWheelEvent(_amount, 
			getCurrentKeyMod(), _xPos - _x, _yPos - _y);
		break;
	}
	case WM_EXITSIZEMOVE:
	case WM_SIZE:
	case WM_SIZING: {
		RECT _rect;
		if (GetWindowRect(hwnd, &_rect)) {
			double _x = static_cast<double>(_rect.left);
			double _y = static_cast<double>(_rect.top);
			double _width = static_cast<double>(_rect.right - _rect.left);
			double _height = static_cast<double>(_rect.bottom - _rect.top);
			_self->resizeEvent({ _x, _y, _width, _height });
		}
		break;
	}
	case WM_CREATE: {
		// Create the API specific graphics context
		HDC _hdc = GetDC(_self->m_Handle);
		_self->m_Graphics.initialize(_hdc);
		return 0;
	}
	case WM_DESTROY: 
		PostQuitMessage(0);
		return 0;
	}

	return DefSubclassProc(hwnd, msg, wparam, lparam);
}
