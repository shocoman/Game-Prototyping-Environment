function start()
    name("Mandelbrot")
    windowsize(600,600)
    
    real0 = -2.3
    imag0 = -1.3
    realStep = 0.05
    imagStep = 0.07

    real1 = real0 + realStep * 78
    imag1 = imag0 + imagStep * 40
end 

function control_input()
    if key("ESC").is_pressed then quit() end
    
    -- zoom
    if key("z").is_pressed then
        realStep = realStep - 0.01
        imagStep = imagStep - 0.01
    elseif key("x").is_pressed then
        realStep = realStep + 0.01
        imagStep = imagStep + 0.01
    end
end

function update(dt)
     control_input()
     clear(255,255,255)

     for y = imag0, imag1, imagStep do
         for x = real0, real1, realStep do
         
             local depth = mandelconverge(x, y)
             
             local color_value = map(depth, 0, 55, 255, 0)
             fill(color_value,color_value,color_value)
             color(color_value,color_value,color_value)
             
             local screen_x = map(x, real0, real1, 0, system.width)
             local screen_y = map(y, imag0, imag1, 0, system.height)
             fcircle(screen_x, screen_y, 3)
     
         end
     end
end

function mandelconverge(real, imag)
    return mandelconverger(real, imag, 0, real, imag)
end

function mandelconverger(real, imag, iters, creal, cimag)
    if iters > 255 or real*real + imag*imag > 4 then
        return iters
    else
        return mandelconverger(real*real - imag*imag + creal,
                      2*real*imag + cimag, iters+1, creal, cimag)
    end
end
