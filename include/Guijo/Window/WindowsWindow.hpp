#pragma once
#include "Guijo/pch.hpp"
#include "Guijo/Objects/Object.hpp"
#include "Guijo/Event/BasicEvents.hpp"

namespace Guijo {
	class Window : public WindowBase {
		friend class Gui;
		Window(const Construct& c);
	public:

		float x() const override;
		float y() const override;
		float width() const override;
		float height() const override;
		Vec2<float> pos() const override;
		Vec2<float> size() const override; 
		Vec4<float> dimensions() const override; 
		void x(const float& v) override;
		void y(const float& v) override;
		void width(const float& v) override;
		void height(const float& v) override;
		void pos(const Vec2<float>& v) override;
		void size(const Vec2<float>& v) override;
		void dimensions(const Vec4<float>& v) override;
	
	private:
		friend class Gui;
		static inline std::size_t windowId = 0;

		WNDCLASS m_WindowClass{};
		HWND m_Handle{};
		std::size_t m_Id{};
		std::queue<std::unique_ptr<Event>> m_EventQueue;
		bool m_ShouldExit = false;

		bool createWindow(const Construct& c);

		bool loop() override;
		void windowsLoop();

		void cursorEvent(float x, float y, KeyMod mod);
		void mouseButtonEvent(MouseButton button, bool press, KeyMod mod);
		void mouseWheelEvent(std::int16_t amount, KeyMod mod, float x, float y);
		void keyEvent(KeyCode key, bool repeat, int action, KeyMod mod);
		void resizeEvent(Dimensions dims);
		static KeyMod getCurrentKeyMod();
		static LRESULT windowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
		static LRESULT windowSubProc(HWND hwnd, UINT msg, WPARAM wparam,
			LPARAM lparam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
	};
}