function start()
    windowsize(600,600)
    use_depth_buffer()

    load_model_data()
    
    print(#vertices)
    print(#facets)
    light_dir = Vec(1,0,0)
end

function update()

    if key("right_arrow").is_held then rotate_around_y_axe(0.05) end
    if key("left_arrow").is_held then rotate_around_y_axe(-0.05) end
    if key("up_arrow").is_held then rotate_around_x_axe(0.05) end
    if key("down_arrow").is_held then rotate_around_x_axe(-0.05) end

    clear(55,55,88)
    
    rotate_light_source()

       
    for i,f in ipairs(facets) do
        
        local fst_v = vertices[f[1]]
        local snd_v = vertices[f[2]]
        local third_v = vertices[f[3]]

        
        local fst_mapped_x = map(fst_v[1],-1,1,0,system.width)
        local fst_mapped_y = map(fst_v[2],1,-1,0,system.height)
        
        local snd_mapped_x = map(snd_v[1],-1,1,0,system.width)
        local snd_mapped_y = map(snd_v[2],1,-1,0,system.height)
        
        local third_mapped_x = map(third_v[1],-1,1,0,system.width)
        local third_mapped_y = map(third_v[2],1,-1,0,system.height)
        
        
        
        -- set up correct lighting
        local fst_vec = Vec(snd_v[1]-fst_v[1],snd_v[2]-fst_v[2],snd_v[3]-fst_v[3])
        local snd_vec = Vec(third_v[1]-fst_v[1],third_v[2]-fst_v[2],third_v[3]-fst_v[3])
        
        local normal = fst_vec:cross(snd_vec)
        local light_force = light_dir:dot(normal:normalize()) 
        
        local col = map(light_force, -1, 1, 25, 225)
        color(col,col,col)
        
        
        triangle3d(fst_mapped_x, fst_mapped_y, fst_v[3] * 1000, 
                    snd_mapped_x, snd_mapped_y, snd_v[3] * 1000,
                    third_mapped_x, third_mapped_y, third_v[3] * 1000)
    end  

end



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


function rotate_around_x_axe(angle)
    local sn = math.sin(angle)
    local cs = math.cos(angle)
    
    for k,v in ipairs(vertices) do
        local y_new = v[3] * sn + v[2] * cs
        local z_new = v[3] * cs - v[2] * sn
        
        v[2] = y_new
        v[3] = z_new 
    end
end



function rotate_light_source()
    if key("d").is_held then
            local sn = math.sin(0.05)
            local cs = math.cos(0.05)
            light_dir:set(light_dir.z * sn + light_dir.x * cs,
                             light_dir.y, light_dir.z * cs - light_dir.x * sn)
    elseif key("a").is_held then
            local sn = math.sin(-0.05)
            local cs = math.cos(-0.05)
            light_dir:set(light_dir.z * sn + light_dir.x * cs,
                             light_dir.y, light_dir.z * cs - light_dir.x * sn)
    elseif key("w").is_held then
            local sn = math.sin(0.05)
            local cs = math.cos(0.05)
            light_dir:set(light_dir.x, light_dir.z * sn + light_dir.y * cs,
                             light_dir.z * cs - light_dir.y * sn)
    elseif key("s").is_held then
            local sn = math.sin(-0.05)
            local cs = math.cos(-0.05)
            light_dir:set(light_dir.x, light_dir.z * sn + light_dir.y * cs,
                             light_dir.z * cs - light_dir.y * sn)
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