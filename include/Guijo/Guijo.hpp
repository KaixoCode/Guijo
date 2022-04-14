#pragma once
#include "Guijo/Objects/Object.hpp"
#include "Guijo/Window/Window.hpp"
#include "Guijo/Event/BasicEvents.hpp"

namespace Guijo {
	class Gui {
		constexpr static bool threading = false;
		struct WindowHandler {

			WindowHandler(auto ctor) : window(ctor()) {}

			WindowHandler(auto ctor, auto& syncData) {
				std::thread{ [this, ctor, &syncData]() { loop(ctor, syncData); } }.detach();
			}

			void loop(auto ctor, auto& syncData) {
				window = ctor();
				Pointer<Window> _window = window;
				{
					std::lock_guard<std::mutex> _lock(syncData.mutex);
					syncData.ready = true;
				}
				syncData.cv.notify_all();
				
				while (!done) {
					done = !_window->loop();
				}

				return;
			}

			Pointer<Window> window{ nullptr };
			bool done = false;

			~WindowHandler() {
				done = true;
			}
		};
	public:
		template<std::derived_from<Window> Ty>
		Ty& emplace(const typename Ty::Construct& arg) {
			if constexpr (threading) {
				// Data necessary to synchronize to the window thread
				// and return back here once the window has been constructed
				struct SyncData {
					std::condition_variable cv{};
					std::mutex mutex{};
					bool ready = false;
				} syncData;
				// Create the window handler, send a constructor and the syncData
				WindowHandler& v = m_Windows.emplace_back([&arg]() {
					return new Ty{ arg };
					}, syncData);
				// Wait for the window thread to notify us the 
				// window has been constructed
				std::unique_lock<std::mutex> _lock(syncData.mutex);
				syncData.cv.wait(_lock, [&]() { return syncData.ready; });
				// It is now safe to read the window from the handler
				return *dynamic_cast<Ty*>(v.window.get());
			} else {
				WindowHandler& v = m_Windows.emplace_back([&arg]() {
					return new Ty{ arg }; });
				return *dynamic_cast<Ty*>(v.window.get());
			}
		}

		bool loop() {
			auto i = m_Windows.begin();
			while (i != m_Windows.end()) {
				if constexpr (threading) {
					if (i->done) m_Windows.erase(i++);
					else ++i;
				} else {
					if (!i->window->loop()) m_Windows.erase(i++);
					else ++i;
				}
			}
			if constexpr (threading) {
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
			return m_Windows.size() != 0;
		}

	private:
		std::list<WindowHandler> m_Windows;
	};
}