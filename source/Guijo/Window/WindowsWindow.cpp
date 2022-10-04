#include "Guijo/Window/Window.hpp"
using namespace Guijo;

Window::Window(const Construct& c) {
    createWindow(c);
};

float Window::x() const { return EventReceiver::x(); }
float Window::y() const { return EventReceiver::y(); }
float Window::width() const { return EventReceiver::width(); }
float Window::height() const { return EventReceiver::height(); }
Point<float> Window::pos() const { return EventReceiver::pos(); }
Size<float> Window::size() const { return EventReceiver::size(); }
Dimensions<float> Window::dimensions() const { return EventReceiver::dimensions(); }

void Window::x(const float& v) { 
    RECT rect{ v, top(), right(), bottom() };
    AdjustWindowRect(&rect, WS_VISIBLE, false);
    SetWindowPos(m_Handle, nullptr, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, 0);
}

void Window::y(const float& v) { 
    RECT rect{ left(), v, right(), bottom() };
    AdjustWindowRect(&rect, WS_VISIBLE, false);
    SetWindowPos(m_Handle, nullptr, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, 0);
}

void Window::width(const float& v) {
    RECT rect{ left(), top(), left() + v, bottom() };
    AdjustWindowRect(&rect, WS_VISIBLE, false);
    SetWindowPos(m_Handle, nullptr, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, 0);
}

void Window::height(const float& v) {
    RECT rect{ left(), top(), right(), top() + v };
    AdjustWindowRect(&rect, WS_VISIBLE, false);
    SetWindowPos(m_Handle, nullptr, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, 0);
}

void Window::pos(const Point<float>& v) {
    RECT rect{ v.x(), v.y(), right(), bottom() };
    AdjustWindowRect(&rect, WS_VISIBLE, false);
    SetWindowPos(m_Handle, nullptr, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, 0);
}

void Window::size(const Size<float>& v) {
    RECT rect{ left(), top(), left() + v.width(), top() + v.height() };
    AdjustWindowRect(&rect, WS_VISIBLE, false);
    SetWindowPos(m_Handle, nullptr, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, 0);
}

void Window::dimensions(const Dimensions<float>& v) {
    RECT rect{ v.left(), v.top(), v.right(), v.bottom() };
    AdjustWindowRect(&rect, WS_VISIBLE, false);
    SetWindowPos(m_Handle, nullptr, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, 0);
}

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
        WS_POPUP | WS_THICKFRAME | WS_SYSMENU | WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_VISIBLE, // Window style
        _x, _y, _w, _h,
        NULL,       // Parent window    
        NULL,       // Menu
        _hInstance, // Instance handle
        NULL        // Additional application data
    );

    if (m_Handle == NULL) return false;
    else {
        SetWindowSubclass(m_Handle, windowSubProc, m_Id, (DWORD_PTR)this);

        // Set the margins of the Win32 window.
        MARGINS _margins = { 1, 1, 1, 1 };
        DwmExtendFrameIntoClientArea(m_Handle, &_margins);

        SendMessage(m_Handle, WM_CREATE, 0, 0);

        RECT _rect;
        if (GetClientRect(m_Handle, &_rect)) {
            float _x = static_cast<float>(_rect.left);
            float _y = static_cast<float>(_rect.top);
            float _width = static_cast<float>(_rect.right - _rect.left);
            float _height = static_cast<float>(_rect.bottom - _rect.top);
            resizeEvent({ _x, _y, _width, _height });
        }

        return true;
    }
}

bool Window::loop() {
    MSG msg;
    std::size_t _handled = 0;
    while (PeekMessage(&msg, m_Handle, 0, 0, PM_NOREMOVE)) {
        if (GetMessage(&msg, m_Handle, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (++_handled > 30) break;
    }
    windowsLoop();

    while (!m_EventQueue.empty()) { // Event cycle
        handle(*m_EventQueue.front());
        m_EventQueue.pop();
    }

    return !m_ShouldExit;
}

void Window::windowsLoop() {
    box.size = size();
    box.windowSize = size();
    box.format(*this);

    update(); // Update cycle

    m_Graphics.prepare(); // Graphics cycle
    pre(m_Graphics.context);
    draw(m_Graphics.context);
    post(m_Graphics.context);
    m_Graphics.render();
    m_Graphics.swapBuffers();
}

void Window::cursorEvent(float x, float y, KeyMod mod) {
    cursor.position = { x, y };
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

void Window::mouseWheelEvent(std::int16_t amount, KeyMod mod, float x, float y) {
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
    EventReceiver::dimensions(dims);
    m_Graphics.dimensions(dims);
    windowsLoop();
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

// Hit test the frame for resizing and moving.
LRESULT HitTestNCA(HWND hWnd, WPARAM wParam, LPARAM lParam, float scale)
{
    // Get the point coordinates for the hit test.
    POINT _ptMouse = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

    // Get the window rectangle.
    RECT _rcWindow;
    GetWindowRect(hWnd, &_rcWindow);

    // Get the frame rectangle, adjusted for the style without a caption.
    RECT _rcFrame = { 0 };
    AdjustWindowRectEx(&_rcFrame, WS_OVERLAPPEDWINDOW & ~WS_CAPTION, FALSE, NULL);

    // Determine if the hit test is for resizing. Default middle (1,1).
    USHORT _uRow = 1;
    USHORT _uCol = 1;
    bool _fOnResizeBorder = false;

    // Determine if the point is at the top or bottom of the window.
    int _padding = 8 / scale;
    if (_ptMouse.y >= _rcWindow.top && _ptMouse.y < _rcWindow.top + 32 / scale)
    {
        _fOnResizeBorder = (_ptMouse.y < (_rcWindow.top - _rcFrame.top));
        _uRow = 0;
    }
    else if (_ptMouse.y < _rcWindow.bottom && _ptMouse.y >= _rcWindow.bottom - _padding)
    {
        _uRow = 2;
    }

    // Determine if the point is at the left or right of the window.
    if (_ptMouse.x >= _rcWindow.left && _ptMouse.x < _rcWindow.left + _padding)
        _uCol = 0; // left side

    else if (_ptMouse.x < _rcWindow.right && _ptMouse.x >= _rcWindow.right - _padding)
        _uCol = 2; // right side

    // Hit test (HTTOPLEFT, ... HTBOTTOMRIGHT)
    LRESULT _hitTests[3][3] =
    {
        { HTTOPLEFT,    _fOnResizeBorder ? HTTOP : HTCAPTION,    HTTOPRIGHT },
        { HTLEFT,       HTNOWHERE,     HTRIGHT },
        { HTBOTTOMLEFT, HTBOTTOM, HTBOTTOMRIGHT },
    };

    return _hitTests[_uRow][_uCol];
}

LRESULT Window::windowSubProc(HWND hwnd, UINT msg, WPARAM wparam,
    LPARAM lparam, UINT_PTR, DWORD_PTR self) {
    Window* _self = reinterpret_cast<Window*>(self);
    if (_self == nullptr) return 0;
    bool _capture = true;
    switch (msg) {
    case WM_NCLBUTTONDOWN: case WM_NCRBUTTONDOWN: case WM_NCMBUTTONDOWN: {
        if (_self->cursor.position.x() < _self->width() - 3 * 45
            || _self->cursor.position.y() <= 8 || _self->cursor.position.x() >= _self->width() - 8) break;
        _capture = false;
        [[fallthrough]];
    }
    case WM_LBUTTONDOWN: case WM_RBUTTONDOWN: case WM_MBUTTONDOWN: {
        if (_capture) SetCapture(hwnd);
        MouseButton _button =
            msg == WM_LBUTTONDOWN ? MouseButton::Left :
            msg == WM_RBUTTONDOWN ? MouseButton::Right :
            msg == WM_MBUTTONDOWN ? MouseButton::Middle : MouseButton::None;
        _self->mouseButtonEvent(_button, true, getCurrentKeyMod());
        break;
    }
    case WM_NCLBUTTONUP: case WM_NCRBUTTONUP: case WM_NCMBUTTONUP:
    case WM_LBUTTONUP: case WM_RBUTTONUP: case WM_MBUTTONUP: {
        MouseButton _button =
            msg == WM_LBUTTONUP ? MouseButton::Left :
            msg == WM_RBUTTONUP ? MouseButton::Right :
            msg == WM_MBUTTONUP ? MouseButton::Middle : MouseButton::None;
        _self->mouseButtonEvent(_button, false, getCurrentKeyMod());
        ReleaseCapture();
        break;
    }
    case WM_NCMOUSEMOVE: {
        RECT _rect;
        long _x = 0, _y = 0;
        if (::GetWindowRect(hwnd, &_rect))
            _x = _rect.left, _y = _rect.top;

        float x = static_cast<float>(GET_X_LPARAM(lparam));
        float y = static_cast<float>(GET_Y_LPARAM(lparam));
        _self->cursorEvent(x -_x, y - _y, getCurrentKeyMod());
        break;
    }
    case WM_MOUSEMOVE: {
        float x = static_cast<float>(GET_X_LPARAM(lparam));
        float y = static_cast<float>(GET_Y_LPARAM(lparam));
        _self->cursorEvent(x, y, getCurrentKeyMod());

        SetCursor(_self->m_ArrorCursor);
        break;
    }
    case WM_SYSKEYDOWN: case WM_SYSKEYUP: case WM_KEYDOWN:
    case WM_KEYUP:      case WM_SYSCHAR:  case WM_CHAR: {
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
        float _x = 0, _y = 0;
        if (GetWindowRect(hwnd, &_rect)) {
            _x = static_cast<float>(_rect.left);
            _y = static_cast<float>(_rect.top);
        }

        std::int16_t _amount = GET_WHEEL_DELTA_WPARAM(wparam);
        float _xPos = static_cast<float>(GET_X_LPARAM(lparam));
        float _yPos = static_cast<float>(GET_Y_LPARAM(lparam));
        _self->mouseWheelEvent(_amount, 
            getCurrentKeyMod(), _xPos - _x, _yPos - _y);
        break;
    }
    case WM_EXITSIZEMOVE:
    case WM_SIZE:
    case WM_SIZING: {
        RECT _rect;
        if (GetClientRect(hwnd, &_rect)) {
            float _x = static_cast<float>(_rect.left);
            float _y = static_cast<float>(_rect.top);
            float _width = static_cast<float>(_rect.right - _rect.left);
            float _height = static_cast<float>(_rect.bottom - _rect.top);
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
        _self->m_ShouldExit = true;
        PostQuitMessage(0); 
        return 0;
    case WM_NCCALCSIZE: 
        if (wparam == TRUE) return 0;
        break;
    case WM_NCHITTEST:
        if (_self->cursor.position.x() >= _self->width() - 3 * 45
            && _self->cursor.position.y() > 8 && _self->cursor.position.x() < _self->width() - 8) break;
        if (auto _ret = HitTestNCA(hwnd, wparam, lparam, 1))
            return _ret;
        break;
    }

    return DefSubclassProc(hwnd, msg, wparam, lparam);
}
