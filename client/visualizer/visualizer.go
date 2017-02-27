package visualizer

import (
	"github.com/Tondorf/gravioli/client/api"
	//"time"
	"github.com/veandco/go-sdl2/sdl"
	"github.com/veandco/go-sdl2/sdl_gfx"
	"fmt"
	"os"
)

var window *sdl.Window
var renderer *sdl.Renderer

func Init(worldchan <-chan *api.World) {
	sdl.Init(sdl.INIT_EVERYTHING)
	window, _ = sdl.CreateWindow("Gravioli", sdl.WINDOWPOS_UNDEFINED, sdl.WINDOWPOS_UNDEFINED, 800, 600, sdl.WINDOW_SHOWN)
	var err error
	renderer, err = sdl.CreateRenderer(window, -1, sdl.RENDERER_SOFTWARE)
	if err != nil {
		fmt.Fprint(os.Stderr, "Failed to create renderer: %s\n", err)
		os.Exit(2)
	}
	//defer window.Destroy()
	go run(worldchan)
}

func run(worldchan <-chan *api.World) {
	for {
		world := <-worldchan
		draw(world)
	}
}

func draw(world *api.World) {
	//println(world.Planets)

	surface, _ := window.GetSurface()

	surface.FillRect(&sdl.Rect{0, 0, 800, 600}, 0xff000000)

	for _, planet := range world.Planets {
		renderer.Clear()
		//defer renderer.Destroy()

		gfx.FilledCircleColor(renderer, int(*planet.Sphere.X), int(*planet.Sphere.Y), int(*planet.Sphere.Radius), sdl.Color{0, 0, 255, 0})

		renderer.Present()

		fmt.Println(planet)
	}


	//window.UpdateSurface()

}

//func WindowTest() {
//
//	surface, _:= window.GetSurface()
//	surface.FillRect(&sdl.Rect{0, 0, 200, 200}, 0xffff0000)
//	window.UpdateSurface()
//
//	sdl.Delay(1000)
//	sdl.Quit()
//}
