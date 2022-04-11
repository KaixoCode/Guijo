#pragma once
#include "Guijo/pch.hpp"
#include "Guijo/Objects/Object.hpp"
#include "Guijo/Event/BasicEvents.hpp"

namespace Guijo {
	class Window : public WindowBase {
		friend class Gui;
		Window(const Construct& c);
	public:

		double x() const override;
		double y() const override;
		double width() const override;
		double height() const override;
		void x(const double& v) override;
		void y(const double& v) override;
		void width(const double& v) override;
		void height(const double& v) override;
	
	private:
		friend class Gui;
		static inline std::size_t windowId = 0;

		WNDCLASS m_WindowClass{};
		HWND m_Handle{};
		std::size_t m_Id{};
		std::queue<std::unique_ptr<Event>> m_EventQueue;

		bool createWindow(const Construct& c);

		bool loop() override;
		void windowsLoop();

		void cursorEvent(double x, double y, KeyMod mod);
		void mouseButtonEvent(MouseButton button, bool press, KeyMod mod);
		void mouseWheelEvent(std::int16_t amount, KeyMod mod, double x, double y);
		void keyEvent(KeyCode key, bool repeat, int action, KeyMod mod);
		void resizeEvent(Dimensions dims);
		static KeyMod getCurrentKeyMod();
		static LRESULT windowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
		static LRESULT windowSubProc(HWND hwnd, UINT msg, WPARAM wparam,
			LPARAM lparam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
	};
}