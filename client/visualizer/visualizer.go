package visualizer

import "github.com/veandco/go-sdl2/sdl"

//import "../api"

func WindowTest() {

	sdl.Init(sdl.INIT_EVERYTHING)

	window, _:= sdl.CreateWindow("test", sdl.WINDOWPOS_UNDEFINED, sdl.WINDOWPOS_UNDEFINED, 800, 600, sdl.WINDOW_SHOWN)
	defer window.Destroy()

	surface, _:= window.GetSurface()
	surface.FillRect(&sdl.Rect{0, 0, 200, 200}, 0xffff0000)
	window.UpdateSurface()

	sdl.Delay(1000)
	sdl.Quit()
}
