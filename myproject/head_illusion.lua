function start()
    windowsize(600,600)
    

    load_model_data()
    
    print(#vertices)
    print(#facets)

end

function update()

    if key("SPACE").is_held then  end
    rotate_around_y_axe(0.05)
    clear(0,0,0)
    
       
    for k,v in ipairs(vertices) do
        
        local x = v[1]
        local y = v[2]
        
        local mapped_x = map(x,-1,1,0,system.width)
        local mapped_y = map(y,1,-1,0,system.height)
        
        
        pixel(mapped_x, mapped_y)
        
    end
       
       

    
end

--    void rotate_around_y_axe(double angle) {
--        double s = sin(angle);
--        double c = cos(angle);
--
--        for (auto& v : vertices) {
--            double znew = v.z * c - v.x * s;
--            double xnew = v.z * s + v.x * c;
--            double ynew = v.y;
--
--            v.x = xnew;
--            v.z = znew;
--        }
--    }

function rotate_around_y_axe(angle)

    local sn = math.sin(angle)
    local cs = math.cos(angle)
    
    
    for k,v in ipairs(vertices) do
        
        local x_new = v[3] * sn + v[1] * cs
        local z_new = v[3] * cs - v[1] * sn
        
        v[1] = x_new
        v[3] = z_new
                
    end

end



function load_model_data()
    head = io.open("assets/head.obj", "r")

    facets = {}
    vertices = {}
    for line in head:lines() do
        -- parse head vertices
        if line:sub(1,2) == "v " then
            coords = {}
            for word in line:gmatch("%S+") do 
                if word ~= "v" then
                    table.insert(coords, tonumber(word))
                end
            end
            table.insert(vertices, coords)
        end
        
        -- pase head triangle numbers
        if line:sub(1,2) == "f " then
            triangle_vertices = {}
            for triple in line:gmatch("%S+") do 
                if triple ~= "f" then
                    table.insert(triangle_vertices, tonumber(triple:match("(.-)/")))      
                end
            end
            
            table.insert(facets, triangle_vertices)
        end
        
    end

end