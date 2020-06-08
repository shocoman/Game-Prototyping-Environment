

function start()
    windowsize(1920,800)
    pixelsize(1,1)
    
    
    t1 = Texture.load("assets/eye.bmp")
    t2 = Texture.load("assets/eye.bmp")
    t3 = Texture.load("assets/eye.bmp")
    t4 = Texture.load("assets/eye.bmp")

    
    
    t1:resize(450,450)
    interpolation_mode("NEAREST")
    t2:resize(30,30)
    t2:resize(450,450)
    
    interpolation_mode("BILINEAR")
    t3:resize(30,30)
    t3:resize(450,450)
    
    interpolation_mode("BICUBIC")
    t4:resize(30,30)
    t4:resize(450,450)
    
end

    

function update()
    if key("ESC").is_pressed then quit() end
    
    local x0, y0 = system.width/2, system.height/2
    local x1, y1 = mouse.x, mouse.y
    
    clear(0,0,0)

--    draw_line(x0,y0,x1,y1)  

    texture(t1, 0, 0)
    texture(t2, 480, 0)
    texture(t3, 960, 0)
    texture(t4, 1440, 0)
    
    text("Original", 80, 500)
    text("Nearest\nNeighbours", 550, 500)
    text("Bilinear", 1050, 500)
    text("Bicubic", 1530, 500)
    
end