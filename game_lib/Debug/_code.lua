


function start()
	name("hello")
	windowsize(200,150)
	pixelsize(4,4)


	t = Texture.load("assets/eye.bmp")
end 


function update(dt)
	if key("ESC").is_pressed then quit() end
    if key("A").is_pressed then print "bobby" end
	--texture(t,0,0)
	clear(255,255,255)

	color(0,100,50)
	fcircle(mouse.x, mouse.y, 100)

end

