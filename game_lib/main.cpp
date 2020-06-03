
#include <iostream>
#include <windows.h>
#include <algorithm>
#include <string>
#include <stdexcept>
#include <locale>
#include <codecvt>
#include <string>


using namespace std;


#include <lua.hpp>
#pragma comment(lib, "lua/lua53.lib")

#include <LuaBridge/LuaBridge.h>
using namespace luabridge;

#include "GameEngine.h"


class InterpreterLua;
static InterpreterLua* geh;
class InterpreterLua : public GameEngine {
public:
    int screen_width_ = 100;
    int screen_height_ = 100;
    int pixel_width_ = 1;
    int pixel_height_ = 1;
    bool full_screen_ = false;
    Pixel current_color = 0xFFFFFFFF;
    Pixel current_fill_color = 0x0;
    Texture::InterpolationMode interpolation_mode = Texture::InterpolationMode::Nearest;
    

    string file_prefix;

    int x = 200;
    int y = 200;

    lua_State* L = luaL_newstate();


    string get_filepath(const string& str)
    {
        size_t found = str.find_last_of("/\\");
        return str.substr(0, found);
    }



    InterpreterLua(string filename) {
        geh = this;
        window_name = L"My name";
        audio.create_audio(44100, 1, 8, 512);
        file_prefix = get_filepath(filename);
        

        bool res = lua_init(filename);
        if (!res) {
            cout << "Lua init error!" << endl;
            return;
        }

        construct_engine(screen_width_, screen_height_, pixel_width_, pixel_height_, full_screen_);
        start_engine();
    }



    static void size_lua(float w, float h) {
        geh->screen_width_ = w;
        geh->screen_height_ = h;
    }
    static void pixel_density_lua(float w, float h) {
        geh->pixel_width_ = w;
        geh->pixel_height_ = h;
    }
    static void fullscreen_lua() {
        geh->full_screen_ = true;
    }

    static void set_name_lua(const char* str) {
        // ascii to unicode
        int size = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
        wchar_t* buffer = new wchar_t[size];
        MultiByteToWideChar(CP_UTF8, 0, str, -1, buffer, size);
        geh->window_name = buffer;
    }

    static void clear_screen_lua(float r, float g, float b) {
        geh->clear_screen(Pixel::get_color(r,g,b));
    }

    static void draw_pixel_lua(float x, float y) {
        geh->draw_pixel(x, y, geh->current_color);
    }

    static void set_color_lua(float r, float g, float b) {
        geh->current_color = Pixel::get_color(r, g, b);
    }

    static void set_fill_color_lua(float r, float g, float b) {
        geh->current_fill_color = Pixel::get_color(r, g, b);
    }

    static void draw_line_lua(float x0, float y0, float x1, float y1) {
        geh->draw_line(x0, y0, x1, y1, geh->current_color);
    }
    static void draw_thick_line_lua(float x0, float y0, float x1, float y1, float thickness) {
        geh->draw_thick_line(x0, y0, x1, y1, thickness, geh->current_color);
    }

    static void draw_rect_lua(float x0, float y0, float x1, float y1) {
        geh->draw_rect(x0, y0, x1, y1, geh->current_color);
    }

    static void draw_filled_rect_lua(float x0, float y0, float x1, float y1) {
        geh->draw_filled_rect(x0, y0, x1, y1, geh->current_color, geh->current_fill_color);
    }
    
    static void draw_circle_lua(float x, float y, float r) {
        geh->draw_circle(x, y, r, geh->current_color);
    }

    static void draw_filled_circle_lua(float x, float y, float r) {
        geh->draw_filled_circle(x, y, r, geh->current_color, geh->current_fill_color);
    }

    static void draw_triangle_lua(float x0, float y0, float x1, float y1, float x2, float y2) {
        geh->draw_triangle(x0, y0, x1, y1, x2, y2, geh->current_color);
    }
    
    static void draw_triangle3d_lua(float x0, float y0, float z0, float x1, float y1, float z1, float x2, float y2, float z2) {
        geh->draw_triangle3d(x0, y0, z0, x1, y1, z1, x2, y2, z2, geh->current_color);
    }

    static void draw_text_lua(string text, float x, float y) {
        geh->draw_text(text.c_str(), x, y, geh->current_color, 4);
    }

    static void use_depth_buffer_lua() {
        geh->use_depth_buffer = true;
    }

    static int get_key_number_lua(string key) {
        if (key.empty()) return -1;
        for (auto& k : key) k = toupper(k);
        
        if (key.length() == 1) return key[0];
        
        if (key == "LEFT_ARROW"  || key == "LEFT") return key::LEFT_ARROW;
        if (key == "UP_ARROW" || key == "UP")  return key::UP_ARROW;
        if (key == "RIGHT_ARROW" || key == "RIGHT") return key::RIGHT_ARROW;
        if (key == "DOWN_ARROW" || key == "DOWN") return key::DOWN_ARROW;

        if (key == "SPACE") return key::SPACE;
        if (key == "ESC") return key::ESC;

        if (key == "L_MOUSE" || key == "LMOUSE") return key::L_MOUSE;
        if (key == "M_MOUSE" || key == "MMOUSE") return key::M_MOUSE;
        if (key == "R_MOUSE" || key == "RMOUSE") return key::R_MOUSE;
    }
    static key_status get_key_lua(string key) {
        int k = get_key_number_lua(key);
        return geh->get_key(k);
    }

    static Texture* load_texture_from_bmp_lua(const char* str) {
        // append file path
        string file_name = geh->file_prefix;
        file_name += string("/") + string(str);

        // ascii to unicode again
        int size = MultiByteToWideChar(CP_UTF8, 0, file_name.c_str(), -1, nullptr, 0);
        wchar_t* buffer = new wchar_t[size];
        MultiByteToWideChar(CP_UTF8, 0, file_name.c_str(), -1, buffer, size);
        return Texture::load_texture_from_bmp(buffer);
    }
    static void draw_texture_lua(Texture *t, float x, float y) {
        geh->draw_texture(t, x, y);
    }
    static void resize_texture_lua(Texture *t, float x, float y) {
        t->resize(x, y, geh->interpolation_mode);
    }

    static void set_interpolation_mode_lua(string str) {
        Texture::InterpolationMode mode = Texture::InterpolationMode::Nearest;
        for (auto& c : str) c = toupper(c);
        if (str == "NEAREST") mode = Texture::InterpolationMode::Nearest;
        if (str == "BILINEAR") mode = Texture::InterpolationMode::Bilinear;
        if (str == "BICUBIC") mode = Texture::InterpolationMode::Bicubic;
        geh->interpolation_mode = mode;
    }
    
    static void quit_lua() {
        geh->is_engine_done = true;
    }

    static int load_audio_sample_lua(string sample_name) {
        // append file path
        string file_name = geh->file_prefix + string("/") + sample_name;
        return geh->audio.load_audio_sample(file_name);
    }
    static void play_sample_lua(int audio_num, bool loop) {
        geh->audio.play_sample(audio_num, loop);
    }
    static void stop_sample_lua(int audio_num) {
        geh->audio.stop_sample(audio_num);
    }


    void on_user_start() override {

    }

    void append_lua_bindings() {

        getGlobalNamespace(L)
            .addFunction("windowsize", size_lua)
            .addFunction("pixelsize", pixel_density_lua)
            .addFunction("fullscreen", fullscreen_lua)
            .addFunction("name", set_name_lua)
            .addFunction("pixel", draw_pixel_lua)
            .addFunction("line", draw_line_lua)
            .addFunction("thick_line", draw_thick_line_lua)
            .addFunction("rect", draw_rect_lua)
            .addFunction("frect", draw_filled_rect_lua)
            .addFunction("circle", draw_circle_lua)
            .addFunction("fcircle", draw_filled_circle_lua)
            .addFunction("triangle", draw_triangle_lua)
            .addFunction("triangle3d", draw_triangle3d_lua)
            .addFunction("text", draw_text_lua)
            .addFunction("color", set_color_lua)
            .addFunction("fill", set_fill_color_lua)
            .addFunction("texture", draw_texture_lua)
            .addFunction("interpolation_mode", set_interpolation_mode_lua)
            .addFunction("clear", clear_screen_lua)
            .addFunction("quit", quit_lua)
            .addFunction("load_audio", load_audio_sample_lua)
            .addFunction("play", play_sample_lua)
            .addFunction("stop", stop_sample_lua)
            .addFunction("map", map_interval)
            .addFunction("use_depth_buffer", use_depth_buffer_lua);

        getGlobalNamespace(L)
            .addFunction("key", get_key_lua)
            .beginClass<key_status>("Key_info")
                .addProperty("is_pressed", function <bool(const key_status*)>([](const key_status* key) {return key->is_pressed; }))
                .addProperty("is_released", function <bool(const key_status*)>([](const key_status* key) {return key->is_released; }))
                .addProperty("is_held", function <bool(const key_status*)>([](const key_status* key) {return key->is_held; }))
            .endClass()
            .beginNamespace("mouse")
                .addProperty("x", &mouseX)
                .addProperty("y", &mouseY)
            .endNamespace()
            .beginNamespace("screen")
                .addProperty("w", &screen_width_)
                .addProperty("h", &screen_height_)
            .endNamespace()
            .beginNamespace("system")
                .addProperty("frame", &frame_count)
                .addProperty("width", &screen_width_)
                .addProperty("height", &screen_height_)
                .addProperty("mouseX", &mouseX)
                .addProperty("mouseY", &mouseY)
            .endNamespace();


        // Texture and vector classes
        getGlobalNamespace(L)
            .beginClass<Texture>("Texture")
                .addStaticFunction("load", load_texture_from_bmp_lua)
                .addFunction("resize", resize_texture_lua)
                .addProperty("width", &Texture::width)
                .addProperty("height", &Texture::height)
            .endClass()
            .beginClass<vec>("Vec")
                .addConstructor<void (*) (double, double, double)>()
                .addProperty("x", &vec::x, true)
                .addProperty("y", &vec::y, true)
                .addProperty("z", &vec::z, true)
                .addFunction("__add", &vec::add)
                .addFunction("__sub", &vec::sub)
                .addFunction("__mul", &vec::operator*)
                .addFunction("__eq", &vec::operator==)
                .addFunction("__unm", &vec::negative)
                .addFunction("mag", &vec::magnitude)
                .addFunction("mag2", &vec::magnitude_squared)
                .addFunction("dot", &vec::dot)
                .addFunction("cross", &vec::cross)
                .addFunction("cross2d", &vec::cross_2d)
                .addFunction("rotate2d", &vec::rotate_2d)
                .addFunction("add", &vec::add)
                .addFunction("sub", &vec::sub)
                .addFunction("scalar", &vec::scalar)
                .addFunction("normalize", &vec::normalize)
                .addFunction("negative", &vec::negative)
                .addFunction("angle2d", &vec::angle_2d)
                .addFunction("set", &vec::set)
                .addFunction("angle_between", &vec::angle_between)
                .addFunction("unit2d", &vec::unit_vector)
                .addFunction("random2d", &vec::random_2d)
                .addFunction("__tostring", &vec::to_printable)
                .addFunction("perp", &vec::perpendicular_2d)
            .endClass();
    }

    bool lua_init(string lua_filename) {
        luaL_openlibs(L);
        
        append_lua_bindings();

        if (check_lua(luaL_dofile(L, lua_filename.c_str()))) {
            LuaRef lua_start_function = getGlobal(L, "start");

            try {
                lua_start_function();
                return true;
                setGlobal(L, &screen_width_, "width");
                setGlobal(L, &screen_height_, "height");
            }
            catch (LuaException const& e) {
                cout << e.what() << endl;
                return false;
            }
        }

        return false;
    }

    bool check_lua(int code) {
        if (code != LUA_OK) {
            cout << "Error: " << lua_tostring(L, -1) << endl;
            return false;
        } 
        return true;
    }



    void on_user_update(float dt) override {
        if (use_depth_buffer) {
            reset_depth_buffer();
        }

        auto func = getGlobal(L, "update");
        try {
            func(dt);
        }
        catch (LuaException const& e) {
            cout << e.what() << endl;
        }
    }

};



int main(int argc, char** argv)
{

    //char* filename = nullptr;
    //if (argc > 1) {
    //    filename = argv[1];
    //}
    //
    string file_name = string(argv[1]);
    //string file_name = "_code.lua";
    InterpreterLua ge(file_name);
    //m = &ge;
    //ge.construct_engine(650, 650, 1, 1);
    //ge.start();

    return 0;
}








// HANDS
//class InterpreterLua : public GameEngine {
//public:
//    vector<Hand> hands;
//    InterpreterLua() {
//        window_name = TEXT("InterpreterLua");
//    }
//
//    void on_user_start() override {
//        vec center_point(ScreenWidth / 2, ScreenHeight / 2);
//        double radius = 100;
//        int segments = 50;
//        int segment_length = 3;
//        for (double angle = 0; angle < 2 * M_PI; angle += M_PI / 64) {
//            double dx = radius * cos(angle);
//            double dy = radius * sin(angle);
//            Pixel col;
//            if (angle < M_PI)
//                col = lerp_color(color::cyan, color::red, angle / M_PI);
//            else
//                col = lerp_color(color::red, color::cyan, (angle - M_PI) / M_PI);
//
//            Hand hand(center_point.x + dx, center_point.y + dy, segments, segment_length, angle, col);
//            hands.push_back(hand);
//        }
//    }
//
//
//    void on_user_update(float dt) override {
//        clear_screen(color::black);
//
//        for (auto&& hand : hands) {
//            hand.move_to(mouseX, mouseY);
//            hand.draw(this);
//        }
//    }
//};











//class InterpreterLua : public GameEngine {
//public:
//    vector<Hand> hands;
//    InterpreterLua() {
//        window_name = TEXT("InterpreterLua");
//    }
//
//    void on_user_start() override {
//        
//
//
//
//    }
//
//
//    void on_user_update(float dt) override {
//        clear_screen(color::black);
//
//        
//    }
//};
//
//
//
//
//
//
//
////void vector_tests() {
////    vec v1, v2(1.5, 4, -3);
////    v1.set(1.5, 4, -3);
////    assert(v1.x == 1.5 && v1.y == 4 && v1.z == -3);
////    assert(v1 == v2);
////    assert(v1 + v2 == vec(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z));
////    assert(v1 - v2 == vec(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z));
////    assert(v1.dot(v2) == v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
////    assert(v1.cross_2d(v2) == 0);
////    assert(vec(1, 0, 0).magnitude() == 1);
////    assert(abs(v1.normalize().magnitude_squared() - 1) < 1e9);
////    vec v5(0, 1);
////    assert(vec(1, 0).angle_between(v5) == M_PI / 2);
////    assert(v1.distance_between(v2) == pow((v1.x - v2.x), 2), pow((v1.y - v2.y), 2), pow((v1.z - v2.z), 2));
////    vec v3 = v1.negative();
////    assert(v3.x == -v1.x && v3.y == -v1.y && v3.z == -v1.z);
////    vec pv = vec(1, 0).perpendicular_2d();
////    assert(pv == vec(0, 1));
////    vec v4 = v1.scalar(4);
////    assert(v4.x == v1.x * 4 && v4.y == v1.y * 4 && v4.z == v1.z * 4);
////}
////
////void utilities_tests() {
////    assert(map_interval(0.5, 0, 1, 0, 100) == 50);
////    assert(map_interval(0, 10, -10, 2, 4) == 3);
////
////    double d = -13;
////    constrain<double>(d, -20, 20); assert(d == -13);
////    constrain<double>(d, 0, 20); assert(d == 0);
////    constrain<double>(d, -8, -5); assert(d == -5);
////
////    assert(-1 == sign(-20));
////    assert(0 == sign(0));
////    assert(1 == sign(0.1));
////}
//
//
////int main(int argc, char** argv)
////{
////    vector_tests();
////    utilities_tests();
////
////    return 0;
////}
//
