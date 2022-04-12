#include "Guijo/pch.hpp"
#include "Guijo/Objects/Object.hpp"
#include "Guijo/Event/BasicEvents.hpp"

namespace Guijo {
	class WindowBase : public Object {
	public:
		struct Construct {
			std::string_view name;
			Dimensions<float> dimensions{ -1, -1, -1, -1 };
		};

		WindowBase() = default;
		WindowBase(WindowBase&&) = delete;
		WindowBase(const WindowBase&) = delete;

		virtual bool loop() = 0;

		struct CursorState {
			MouseButtons buttons = 0;
			Vec2<float> position{ 0, 0 };
			Vec2<float> pressed{ 0, 0 };
		} cursor;

	protected:
		Graphics m_Graphics{};
	};
}

#ifdef WIN32
#include "WindowsWindow.hpp"
#else
using Window = WindowBase;
#endif