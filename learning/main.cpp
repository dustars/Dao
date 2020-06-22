#pragma comment(lib, "basic.lib")

#include "window.h"
#include "Renderer.h"

int main() {

	Window w("OpenGL", 1280, 720, false); // -16 for viewport size, 528 for 512 dimension

	if (!w.HasInitialised()) {
		return -1;
	}

	Renderer renderer(w);
	if (!renderer.HasInitialised()) {
		return -1;
	}

	w.LockMouseToWindow(true);
	w.ShowOSPointer(false);

	while (w.UpdateWindow() && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE)) {
		float dt = w.GetTimer()->GetTimedMS();
		renderer.Update(dt);

		// ScreenShot, saved in "demo/screenshot.jpg"
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_P)) {
			renderer.ScreenShot();
		}

	}

	return 0;
}