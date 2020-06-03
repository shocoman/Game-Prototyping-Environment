#pragma once

#define _USE_MATH_DEFINES // for PI number
#define SUCCESS 0
#define FAILURE -1

// include libraries
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Winmm.lib")

#include <windows.h>
#include <gdiplus.h>
#include <Shlwapi.h>
#include <GL/gl.h>

#include <fstream>
#include <iomanip>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <cmath>

#include <vector>
#include <list>
#include <iostream>
#include <map>
#include <string>
#include <random>
#include <initializer_list>

using namespace std;


double map_interval(double num, double old_0, double old_1, double new_0, double new_1) {
    // normalize in old interval [old_0, old_1]
    double normalized = (num - old_0) / (old_1 - old_0);
    // map to new interval [new_0, new_1]
    double new_num = normalized * (new_1 - new_0) + new_0;
    return new_num;
}

template <typename T>
void constrain(T& num, T start_interval, T end_interval) {
    if (num < start_interval)
        num = start_interval;
    else if (num > end_interval) {
        num = end_interval;
    }
}


template <typename T>
int sign(T num) {
    return (num > T(0)) - (num < T(0));
}


template <typename T>
class Vector {
public:
    T x;
    T y;
    T z;

    Vector() : x(0), y(0), z(0) {}
    Vector(T x_, T y_, T z_ = 0) : x(x_), y(y_), z(z_) {}

    string to_printable() {
        return " { " + to_string(x) + ", " + to_string(y) + ", " + to_string(z) + " } ";
    }

    friend ostream& operator<<(ostream& stream, const Vector& vec) {
        stream << vec.to_printable();
        return stream;
    }

    void set(T x, T y, T z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    bool operator==(const Vector& rhs) {
        return x == rhs.x && y == rhs.y && z == rhs.z;
    }
    bool operator!=(const Vector& rhs) {
        return !(*this == rhs);
    }

    Vector operator+(const Vector& rhs) {
        return add(rhs);
    }
    Vector operator-(const Vector& rhs) {
        return sub(rhs);
    }

    Vector operator*(const T& scalar_) {
        return scalar(scalar_);
    }

    Vector scalar(const T& scalar_) {
        return Vector(x * scalar_, y * scalar_, z * scalar_);
    }

    Vector add(const Vector& rhs) {
        return Vector(x + rhs.x, y + rhs.y, z + rhs.z);
    }

    Vector sub(const Vector& rhs) {
        return Vector(x - rhs.x, y - rhs.y, z - rhs.z);
    }

    double magnitude() {
        return sqrt(magnitude_squared());
    }

    double magnitude_squared() {
        return x * x + y * y + z * z;
    }

    double dot(const Vector& rhs) {
        return x * rhs.x + y * rhs.y + z * rhs.z;
    }

    Vector cross(const Vector& rhs) {
        return Vector(y*rhs.z - z*rhs.y, z*rhs.x-x*rhs.z, x * rhs.y - y * rhs.x);
    }

    T cross_2d(const Vector& rhs) {
        return cross(rhs).z;
    }

    double angle_2d() {
        return atan2(y, x);
    }

    Vector negative() {
        return Vector(-x, -y, -z);
    }

    Vector normalize() {
        auto len = magnitude();
        return Vector(x / len, y / len, z / len);
    }

    double distance_between(const Vector& rhs) {
        return sub(rhs).magnitude();
    }

    Vector rotate_2d(double angle) {
        T new_x = x * cos(angle) - y * sin(angle);
        T new_y = x * sin(angle) + y * cos(angle);
        return Vector(new_x, new_y, z);
    }

    double angle_between(Vector& rhs) {
        double sign = cross_2d(rhs) < 0 ? -1 : 1;
        double product_of_lengths = magnitude()*rhs.magnitude();
        double angle = acos(dot(rhs) / product_of_lengths) * sign;
        return isnan(angle) ? 0 : angle;
    }

    Vector unit_vector(double angle = 0) {
        Vector<T> v = Vector(T(1), T(0));

        return angle == 0 ? v : v.rotate_2d(angle);
    }

    Vector random_2d() {
        double angle = 2 * M_PI * (double)rand() / RAND_MAX; // cheap but bad random
        //double angle = distribution(generator);
        
        return unit_vector().rotate_2d(angle);
    }

    Vector perpendicular_2d() {
        return Vector(-y, x);
    }



    operator Vector<double>() {
        return Vector<double>(x,y,z);
    }

    operator Vector<int>() {
        // correct float to int rounding
        return Vector<int>(x + 0.5, y + 0.5, z + 0.5);
    }

}; typedef Vector<double> vec;




template <typename T>
class Matrix {
public:
    int rows, cols;
    unique_ptr<T[]> matrix;

    enum class operation { summation, multiplication };

    Matrix() : rows(T(0)), cols(T(0)), matrix(nullptr) {}
    Matrix(int rows_, int cols_) {
        rows = rows_;
        cols = cols_;
        matrix = make_unique<T[]>(rows * cols);
        for (int i = 0; i < rows * cols; i++) matrix[i] = T(0);

    }

    T get(int row, int col) {
        bounds_check(row, col);
        int index = to_index(row, col);
        return matrix[index];
    }

    T& operator()(int row, int col) {
        bounds_check(row, col);
        int index = to_index(row, col);
        return matrix[index];
    }

    void set(int row, int col, T value) {
        bounds_check(row, col);
        int index = to_index(row, col);
        matrix[index] = value;
    }

    void add(Matrix* m2) {
        dimensions_check(this, m2, operation::summation);
        for (size_t row = 0; row < rows; row++) {
            for (size_t col = 0; col < cols; col++) {
                T new_val = get(row, col) + m2->get(row, col);
                set(row, col, new_val);
            }
        }
    }

    void sub(Matrix* m2) {
        dimensions_check(this, m2, operation::summation);
        for (size_t row = 0; row < rows; row++) {
            for (size_t col = 0; col < cols; col++) {
                T new_val = get(row, col) - m2->get(row, col);
                set(row, col, new_val);
            }
        }
    }

    Matrix* mult(Matrix* m2) {
        dimensions_check(this, m2, operation::multiplication);
        int new_rows = this->rows;
        int new_cols = m2->cols;
        auto new_matrix = new Matrix(new_rows, new_cols);

        for (int row = 0; row < new_rows; row++) {
            for (int col = 0; col < new_cols; col++) {

                T sum = T(0);
                // sum of this.rows with m2.cols
                for (int i = 0; i < this->cols; i++) {
                    sum += this->get(row, i) * m2->get(i, col);
                }

                new_matrix->set(row, col, sum);
            }
        }

        return new_matrix;
    }


    void bounds_check(int row, int col) {
        if (row < 0 || row >= rows) {
            cout << "Row out of bounds error!" << endl;
            throw exception();
        }
        if (col < 0 || col >= cols) {
            cout << "Col out of bounds error!" << endl;
            throw exception();
        }
    }

    void dimensions_check(Matrix* m1, Matrix* m2, operation op) {
        switch (op) {
        case operation::multiplication:
            if (m1->cols != m2->rows) {
                cout << "Multiplication Error: number of columns of first matrix should be equal to number of rows in the second one!" << endl;
                throw exception();
            }
            break;
        case operation::summation:
            if (m1->rows != m2->rows || m1->cols != m2->cols) {
                cout << "Summation Error: dimensions of two matrices should be equal!" << endl;
                throw exception();
            }
            break;
        }
    }

    int to_index(int row, int col) {
        return col + row * cols;
    }

    string str() {
        string output;
        output += "{ Matrix <" + string(typeid(T).name()) + ">(" + to_string(rows) + ", " + to_string(cols) + ")\n";
        for (size_t row = 0; row < rows; row++) {
            output += " ";
            for (size_t col = 0; col < cols; col++) {

                output += to_string(get(row, col)) + ", ";
            }
            output += '\n';
        }
        output += "}";

        return output;
    }

    friend ostream& operator<<(ostream& o, Matrix* m) {

        return o << m->str();
    }
};
template <typename T>
using matrix = unique_ptr<Matrix<T>>;




class AudioEngine {
public:

    struct WAV_HEADER {
        // RIFF chunk descriptor
        uint8_t RIFF[4];
        uint32_t ChunkSize;
        uint8_t Format[4];

        // fmt sub-chunk
        uint8_t Subchunk1ID[4];
        uint32_t Subchunk1Size;

        // 'wav' main info
        uint16_t AudioFormat;    // format type (1 = PCM (pulse code modulation), ...)
        uint16_t NumChannels;    // number of channels (1 = mono, 2 = stereo...)
        uint32_t SampleRate;     // sample rate 
        uint32_t ByteRate;       // for buffer estimation 
        uint16_t BlockAlign;     // block size of data 
        uint16_t BitsPerSample;  // number of bits per sample of mono data 

        // possibly 'wav' data sub-chunk
        uint8_t Subchunk2ID[4];
        uint32_t Subchunk2Size;
    };


    class AudioSample
    {
    public:
        string file_name;
        WAV_HEADER header;
        float* sample_buffer;
        bool is_valid;
        int samples;

        AudioSample(string name) {
            file_name = name;
            is_valid = load_sample_from_wav(file_name);

        }

        bool load_sample_from_wav(string file_name) {

            ifstream wav_file;
            wav_file.open(file_name.c_str(), ios_base::in | ios_base::binary);
            if (!wav_file.is_open()) {
                cout << "File open error!" << endl;
                return false;
            }
            wav_file.read((char*)&header, sizeof(WAV_HEADER));
            if (header.BitsPerSample != 16 /*|| header.SampleRate != 44100*/) {
                cout << "We can read only 16-bits rate 'wav' files" << endl;
                return false;
            }

            uint8_t* chunk_name = header.Subchunk2ID;
            uint32_t chunk_size = header.Subchunk2Size;
            // if the chunk is not the one with 'wav' data, skip it and take the next one
            while (strncmp((char*)chunk_name, "data", 4) != 0 && !wav_file.eof()) {
                wav_file.seekg(chunk_size, ios_base::cur);
                wav_file.read((char*)chunk_name, 4);
                wav_file.read((char*)&chunk_size, 1);
            }
            if (wav_file.eof()) {
                cout << "The file doesn't have 'data' sub-chunk" << endl;
                return false;
            }


            // read data chunk
            int bytes_per_sample = header.BitsPerSample >> 3;
            int sample_size = header.NumChannels * bytes_per_sample;
            samples = chunk_size / sample_size;


            sample_buffer = new float[samples * header.NumChannels];
            for (int i = 0; i < samples; i++)
            {
                for (int j = 0; j < header.NumChannels; j++)
                {
                    // read 2 bytes
                    short buffer = 0;
                    wav_file.read((char*)&buffer, bytes_per_sample);
                    int index = i * header.NumChannels + j;
                    sample_buffer[index] = (float)buffer / MAXSHORT;
                }
            }


            wav_file.close();
            return true;
        }

        void show_info() {
            cout << "File name: " << file_name << endl;
            cout << "nSamples: " << samples << endl;
            cout << "SampleSize: " << header.NumChannels * (header.BitsPerSample / 8) << endl;

            cout << "RIFF[4];       " << header.RIFF << endl;
            cout << "ChunkSize;    " << header.ChunkSize << endl;
            cout << "Format[4];     " << header.Format << endl;
            cout << " Subchunk1ID[4];" << header.Subchunk1ID << endl;
            cout << "Subchunk1Size;"  << header.Subchunk1Size << endl;
            cout << "AudioFormat;  "  << header.AudioFormat << endl;
            cout << "NumChannels;  "  << header.NumChannels << endl;
            cout << "SampleRate;   "  << header.SampleRate << endl;
            cout << "ByteRate;     "  << header.ByteRate << endl;
            cout << "BlockAlign;   "  << header.BlockAlign << endl;
            cout << "BitsPerSample;"  << header.BitsPerSample << endl;
            cout << " Subchunk2ID[4];" << header.Subchunk2ID << endl;
            cout << "Subchunk2Size;"  << header.Subchunk2Size << endl;
        }
    };


    thread audio_thread_;
    mutex block_not_zero_;
    condition_variable  cond_block_not_zero_;
    atomic<float> global_time_ = 0;

    atomic<bool> is_audio_thread_active_;
    uint32_t sample_rate_;
    uint32_t channels_;
    uint32_t blocks_;
    uint32_t samples_per_block_;
    uint32_t free_blocks_;
    uint32_t current_block_;

    unique_ptr<short[]> blocks_memory_;
    unique_ptr<WAVEHDR[]> wave_headers_;
    //short* blocks_memory_;
    //WAVEHDR* wave_headers_;
    HWAVEOUT hwo_;

    struct currently_playing_sample {
        int id = 0;
        int position = 0;
        bool finished = false;
        bool looped = false;
    };

    vector<AudioSample> audio_samples;
    list<currently_playing_sample> playing_samples;


    ~AudioEngine() {
        destroy_audio();
        waveOutClose(hwo_);
    }


    int load_audio_sample(string file_name) {
        AudioSample a(file_name);
        if (a.is_valid) {
            audio_samples.push_back(a);
            return audio_samples.size();
        }
        cout << "Audio sample loading error" << endl;
        return -1;
    }


    void play_sample(int id, bool looped = false) {
        currently_playing_sample s;
        s.id = id; s.position = 0; s.looped = looped; s.finished = false;
        if (id < 0 || id > audio_samples.size()) {
            cout << "Such sample doesn't exist!" << endl;
            return;
        }

        playing_samples.push_back(s);
    }

    void stop_sample(int id) {
        playing_samples.remove_if([id](currently_playing_sample &s) {return s.id == id; });
    }


    bool create_audio(uint32_t sample_rate = 44100, uint32_t channels = 1, uint32_t blocks = 8, uint32_t samples_per_block = 512)
    {
        is_audio_thread_active_ = false;
        sample_rate_ = sample_rate;
        channels_ = channels;
        blocks_ = blocks;
        samples_per_block_ = samples_per_block;
        free_blocks_ = blocks;
        current_block_ = 0;
        hwo_ = nullptr;

        WAVEFORMATEX wave_format;
        wave_format.wFormatTag = WAVE_FORMAT_PCM;
        wave_format.nSamplesPerSec = sample_rate_;
        wave_format.wBitsPerSample = sizeof(short) * 8;
        wave_format.nChannels = channels_;
        wave_format.nBlockAlign = (wave_format.wBitsPerSample / 8) * wave_format.nChannels;
        wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;
        wave_format.cbSize = 0;

        // find and prepare device for playing
        if (0 != waveOutOpen(&hwo_, WAVE_MAPPER, &wave_format, (DWORD_PTR)waveOutProc, (DWORD_PTR)this, CALLBACK_FUNCTION)) {
            return false;
        }

        blocks_memory_ = make_unique<short[]>(blocks_ * samples_per_block_);
        ZeroMemory(blocks_memory_.get(), sizeof(short) * blocks_ * samples_per_block_);

        wave_headers_ = make_unique<WAVEHDR[]>(blocks_);
        ZeroMemory(wave_headers_.get(), sizeof(WAVEHDR) * blocks_);


        for (size_t n = 0; n < blocks_; n++) {
            wave_headers_[n].dwBufferLength = samples_per_block_ * sizeof(short);
            wave_headers_[n].lpData = (char*)(blocks_memory_.get() + (n * samples_per_block_));
        }

        is_audio_thread_active_ = true;
        audio_thread_ = thread(&AudioEngine::audio_thread_func, this);


        unique_lock<mutex> lm(block_not_zero_);
        cond_block_not_zero_.notify_one();
        return true;
    }

    bool destroy_audio() {
        is_audio_thread_active_ = false;

        // wait for audio thread to finish
        if (audio_thread_.joinable()) { 
            audio_thread_.join();
        }

        return false;
    }


    static void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
    {
        if (uMsg != WOM_DONE) return;

        auto curr_inst = ((AudioEngine*)dwInstance);
        // if done playing, take the next block
        curr_inst->free_blocks_ += 1;
        unique_lock<mutex> lm(curr_inst->block_not_zero_);
        curr_inst->cond_block_not_zero_.notify_one();
    }


    void audio_thread_func() {
        global_time_ = 0.0f;
        float time_step = 1.0f / (float)sample_rate_;

        short max_sample_s = (short)pow(2, (sizeof(short) * 8) - 1) - 1;
        float max_sample = float(max_sample_s);
        short previous_sample = 0;

        while (is_audio_thread_active_) {
            if (free_blocks_ == 0) {
                unique_lock<mutex> lm(block_not_zero_);
                while (free_blocks_ == 0)
                    cond_block_not_zero_.wait(lm);
            }
            free_blocks_ -= 1;

            if (wave_headers_[current_block_].dwFlags & WHDR_PREPARED)
                waveOutUnprepareHeader(hwo_, &wave_headers_[current_block_], sizeof(WAVEHDR));

            short new_sample = 0;
            int current_block = current_block_ * samples_per_block_;

            auto clip = [](float sample, float max) {
                if (sample > 0)
                    return fmin(sample, max);
                else
                    return fmax(sample, -max);
            };

            for (size_t n = 0; n < samples_per_block_; n++)
            {
                for (size_t c = 0; c < channels_; c++)
                {
                    new_sample = (short)(clip(get_mixer_output(c, global_time_, time_step), 1) * max_sample);
                    blocks_memory_[current_block + n + c] = new_sample;
                    previous_sample = new_sample;
                }

                global_time_ = global_time_ + time_step;
            }

            waveOutPrepareHeader(hwo_, &wave_headers_[current_block_], sizeof(WAVEHDR));
            waveOutWrite(hwo_, &wave_headers_[current_block_], sizeof(WAVEHDR));

            current_block_++;
            current_block_ %= blocks_;
        }
    }

    virtual float on_user_sound_sample(int channel, float global_time, float time_step) {
        return 0;
    }

    virtual float on_user_sound_filter(int channel, float global_time, float sample) {
        return sample;
    }

    float get_mixer_output(int channel, float global_time, float time_step) {
        float mixer_sample = 0;

        for (auto& s : playing_samples) {
            s.position += int(audio_samples[s.id - 1].header.SampleRate * time_step);

            // check if we done with the sample
            if (s.position < audio_samples[s.id - 1].samples)
                mixer_sample += audio_samples[s.id - 1].sample_buffer[(s.position * audio_samples[s.id - 1].header.NumChannels) + channel];
            else if (s.looped) { // if it's looped, start again
                s.position = 0;
            } else {              // or just finish and remove it later from list
                s.finished = true;
            }
        }
    
        // remove finished samples
        playing_samples.remove_if([](currently_playing_sample& s) {return s.finished; });

        // let user generate own sounds, and mix them too
        mixer_sample += on_user_sound_sample(channel, global_time, time_step);

        // let user change current sample
        return on_user_sound_filter(channel, global_time, mixer_sample);
    }
};





struct Pixel {
    union {
        uint32_t color = 0xFF000000;
        struct {
            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t a;
        };
    };

    enum class Mode {
        Normal, Blend, Transparent
    };

    Pixel() : color(0xFF000000) {};
    Pixel(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255) {
        r = red; g = green; b = blue; a = alpha;
    };
    Pixel(uint32_t col) {
        color = col;
    }

    static constexpr uint32_t get_color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255)  {
        uint32_t new_color = 0x0;
        new_color |= red;
        new_color |= (green << 8);
        new_color |= (blue << 16);
        new_color |= (alpha << 24);
        return new_color;
    }

    bool operator==(int rhs) {
        return this->color == rhs;
    }

    bool operator!=(int rhs) {
        return !(*this == rhs);
    }
};

enum color {
    red =   Pixel::get_color(255, 0, 0),
    green = Pixel::get_color(0, 255, 0),
    blue =  Pixel::get_color(0, 0, 255),
    yellow =  Pixel::get_color(255, 255, 0),
    magenta =  Pixel::get_color(255, 0, 255),
    cyan =  Pixel::get_color(0, 255, 255),
    white =  Pixel::get_color(255, 255, 255),
    black =  Pixel::get_color(0, 0, 0),
    gray =  Pixel::get_color(129, 129, 129),
    brown =  Pixel::get_color(119, 21, 21),
    purple =  Pixel::get_color(200, 0, 200)
};


Pixel lerp_color(Pixel c1, Pixel c2, float coeff /*[0; 1]*/) {
    constrain(coeff, 0.0f, 1.0f);
    // if coeff = 0, return c1
    // if coeff = 1, return c2
    // otherwise, return blend between them
    Pixel p;
    p.r = uint8_t((1.0f - coeff) * c1.r + coeff * c2.r);
    p.g = uint8_t((1.0f - coeff) * c1.g + coeff * c2.g);
    p.b = uint8_t((1.0f - coeff) * c1.b + coeff * c2.b);
    return p;
}



class Texture {
public:
    Pixel* pixels;
    int width;
    int height;
    Pixel transparent_pixel = 0x0;

    enum class InterpolationMode {
        Nearest, Bilinear, Bicubic
    };

    Texture() {
        pixels = nullptr;
        width = 0;
        height = 0;
    }

    Texture(int w, int h) {
        width = w; height = h;
        pixels = new Pixel[w*h];
    }

    void clear(Pixel p) {
        for (int i = 0; i < width * height; i++) {
            pixels[i] = Pixel(p);
        }
    }

    void set_transparent_pixel(uint8_t r, uint8_t g, uint8_t b) {
        transparent_pixel = Pixel(r, g, b);
    }

    void remove_transparent_pixel() {
        transparent_pixel = 0x0;
    }

    bool is_transparent_pixel(Pixel p) {
        if (transparent_pixel.color != 0x0) {
            p.a = transparent_pixel.a;
            if (p.color == transparent_pixel.color) {
                return true;
            }
        }

        return false;
    }

    bool set_pixel(int x, int y, Pixel p) {
        if (x >= width || x < 0 || y >= height || y < 0)
            return false;

        int index = x + y * width;
        pixels[index] = p;
        return true;
    }

    Pixel get_pixel(int x, int y) {
        if (x >= width || x < 0 || y >= height || y < 0)
            return color::black;

        int index = x + y * width;
        return pixels[index];
    }

    Pixel* get_pixels() {
        return pixels;
    }

    static Texture* load_texture_from_bmp(LPCWSTR file_name) {

        HBITMAP hBmp = (HBITMAP)LoadImage(GetModuleHandle(NULL), file_name, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

        if (!hBmp) // failed to load bitmap
            return nullptr;

        //getting the size of the picture
        BITMAP bm;
        GetObject(hBmp, sizeof(bm), &bm);

        HDC dcBitmap = CreateCompatibleDC(NULL);
        SelectObject(dcBitmap, hBmp);

        // fill info from your bmp
        BITMAPINFO bmpInfo;
        bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmpInfo.bmiHeader.biWidth = bm.bmWidth;
        bmpInfo.bmiHeader.biHeight = -bm.bmHeight;  // flip it vertically
        bmpInfo.bmiHeader.biPlanes = bm.bmPlanes;
        bmpInfo.bmiHeader.biBitCount = bm.bmBitsPixel;
        bmpInfo.bmiHeader.biCompression = BI_RGB;
        bmpInfo.bmiHeader.biSizeImage = 0;
        
        // load pixel info into array of int32's
        COLORREF* colors = new COLORREF[bm.bmWidth * bm.bmHeight];
        GetDIBits(dcBitmap, hBmp, 0, bm.bmHeight, colors, &bmpInfo, DIB_RGB_COLORS);

        // convert and copy array into our texture
        Texture* t = new Texture(bm.bmWidth, bm.bmHeight);
        for (int i = 0; i < bm.bmWidth * bm.bmHeight; i++) {
            // ARGB -> ABGR
            auto color = colors[i];
            unsigned int a = (color >> 24) & 0xFF;
            unsigned int r = (color >> 16) & 0xFF;
            unsigned int g = (color >> 8) & 0xFF;
            unsigned int b = (color >> 0) & 0xFF;

            // if the pixel somehow has transparency value, use it; otherwise, the pixel will be opaque
            t->get_pixels()[i] = Pixel(r, g, b, a != 0 ? a : 0xFF);
        }

        DeleteObject(hBmp);
        delete[] colors;

        return t;
    }



    Pixel bi_lerp_color(Pixel top_left, Pixel top_right, Pixel bottom_left, Pixel bottom_right, float val_h, float val_v) {
        Pixel lerp_top = lerp_color(top_left, top_right, val_h);
        Pixel lerp_bottom = lerp_color(bottom_left, bottom_right, val_h);
        return lerp_color(lerp_top, lerp_bottom, val_v);
    }



    // used for bicubic interpolation only
    struct P {
        int r = 0, g = 0, b = 0, a = 0;
        P(Pixel p) : r(p.r), g(p.g), b(p.b), a(p.a) {}
        P(int red, int green, int blue, int alpha = 255) : r(red), g(green), b(blue), a(alpha) {};
        P operator+(const P& rhs) { return P(r + rhs.r, g + rhs.g, b + rhs.b); }
        P operator-(const P& rhs) { return P(r - rhs.r, g - rhs.g, b - rhs.b); }
        P operator*(double rhs) { return P(r * rhs, g * rhs, b * rhs); }
        friend P operator*(double lhs, const P& rhs) { return P(rhs.r * lhs, rhs.g * lhs, rhs.b * lhs); }
    };


    P cubic_interpolation(P p[4], double x) {
        return p[1] + 0.5 * x * (p[2] - p[0] + x * (2.0 * p[0] - 5.0 * p[1] + 4.0 * p[2] - p[3] + x * (3.0 * (p[1] - p[2]) + p[3] - p[0])));
    }

    P bi_cubic_interpolation(P p[4][4], double x, double y) {
        P arr[4] = {
            cubic_interpolation(p[0], x),
            cubic_interpolation(p[1], x),
            cubic_interpolation(p[2], x),
            cubic_interpolation(p[3], x)
        };

        return cubic_interpolation(arr, y);
    }

    bool resize(int new_width, int new_height, InterpolationMode mode = InterpolationMode::Nearest) {
        if (new_width < 0 || new_height < 0) return false;

        Pixel* old_pixels = pixels;
        int old_width = width;
        int old_height = height;

        Pixel* new_pixels = new Pixel[new_width * new_height];
        float width_ratio = (float)old_width / new_width;
        float height_ratio = (float)old_height / new_height;
        if (mode == InterpolationMode::Nearest) { // nearest-neighbor interpolation
            for (int x = 0; x < new_width; x++) {
                for (int y = 0; y < new_height; y++) {
                    int old_x = int(x * width_ratio);
                    int old_y = int(y * height_ratio);

                    int new_index = x + y * new_width;
                    new_pixels[new_index] = get_pixel(old_x, old_y);
                }
            }
        }
        else if (mode == InterpolationMode::Bilinear) { // bilinear interpolation
            for (int x = 0; x < new_width; x++) {
                for (int y = 0; y < new_height; y++) {

                    float val_x = x * width_ratio;
                    float val_y = y * height_ratio;

                    int floor_x = int(val_x);
                    int floor_y = int(val_y);
                    int ceil_x = floor_x + 1;
                    int ceil_y = floor_y + 1;

                    val_x = val_x - floor_x;
                    val_y = val_y - floor_y;

                    Pixel top_left = get_pixel(floor_x, floor_y);
                    Pixel top_right = get_pixel(ceil_x, floor_y);
                    Pixel bottom_left = get_pixel(floor_x, ceil_y);
                    Pixel bottom_right = get_pixel(ceil_x, ceil_y);
                    Pixel p = bi_lerp_color(top_left, top_right, bottom_left, bottom_right, val_x, val_y);

                    int new_index = x + y * new_width;
                    new_pixels[new_index] = p;
                }
            }
        }
        else if (mode == InterpolationMode::Bicubic) { // bicubic interpolation
            // thanks https://www.paulinternet.nl/?page=bicubic
            for (int x = 0; x < new_width; x++) {
                for (int y = 0; y < new_height; y++) {

                    double val_x = x * (double)width_ratio;
                    double val_y = y * (double)height_ratio;
                    int old_x = int(val_x);
                    int old_y = int(val_y);

                    // a1 a2 a3 a4
                    // b1 b2 b3 b4
                    // c1 c2 c3 c4
                    // d1 d2 d3 d4

                    P a1(get_pixel(old_x - 1, old_y - 1)), a2(get_pixel(old_x, old_y - 1)), a3(get_pixel(old_x + 1, old_y - 1)), a4(get_pixel(old_x + 2, old_y - 1));
                    P b1(get_pixel(old_x - 1, old_y + 0)), b2(get_pixel(old_x, old_y + 0)), b3(get_pixel(old_x + 1, old_y + 0)), b4(get_pixel(old_x + 2, old_y + 0));
                    P c1(get_pixel(old_x - 1, old_y + 1)), c2(get_pixel(old_x, old_y + 1)), c3(get_pixel(old_x + 1, old_y + 1)), c4(get_pixel(old_x + 2, old_y + 1));
                    P d1(get_pixel(old_x - 1, old_y + 2)), d2(get_pixel(old_x, old_y + 2)), d3(get_pixel(old_x + 1, old_y + 2)), d4(get_pixel(old_x + 2, old_y + 2));

                    P grid[4][4] = {
                        {a1, a2, a3, a4},
                        {b1, b2, b3, d4},
                        {c1, c2, c3, c4},
                        {d1, d2, d3, d4}
                    };

                    P result = bi_cubic_interpolation(grid, val_x - old_x, val_y - old_y);
                    constrain(result.r, 0, 255);
                    constrain(result.g, 0, 255);
                    constrain(result.b, 0, 255);
                    constrain(result.a, 0, 255);
                    
                    int new_index = x + y * new_width;
                    new_pixels[new_index] = Pixel(result.r, result.g, result.b);
                }
            }
        }



        width = new_width;
        height = new_height;
        pixels = new_pixels;
        delete[] old_pixels;

        return true;
    }


    ~Texture() {
        if (pixels) delete[] pixels;
    }
};


class GameEngine
{
public:
    int WindowX;
    int WindowY;
    int WindowWidth;
    int WindowHeight;
    int ViewX;
    int ViewY;
    int ViewW;
    int ViewH;
    int ScreenWidth; 
    int ScreenHeight;
    int PixelWidth;
    int PixelHeight;
    int mouseX;
    int mouseY;
    bool full_screen;
    bool use_depth_buffer = false;

    unsigned long long frame_count;

    LPCWSTR window_name;

    Texture* draw_target;
    Texture* default_draw_target;
    Texture* font_sheet; // for text drawing

    HDC hDC;				// device context
    HGLRC hRC;				// opengl context
    HWND  hWnd;				// window handle
    GLuint glBuffer;        // opengl buffer for main texture
    atomic<bool> is_engine_done = false;

    GLuint base;    // used for text drawing with opengl and winapi
    HFONT font;
    HFONT old_font;

    AudioEngine audio;
    Pixel::Mode pixel_mode;

    float *depth_buffer;


    enum key {
        A = 'A', B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        _0 = '0', _1, _2, _3, _4, _5, _6, _7, _8, _9,
        LEFT_ARROW = 37, UP_ARROW, RIGHT_ARROW, DOWN_ARROW,
        SPACE = 32, ESC = 27,
        L_MOUSE = 256, M_MOUSE, R_MOUSE
    };

    struct key_status {
        bool is_pressed = false;
        bool is_held = false;
        bool is_released = false;
    };

    map<int, key_status> keyboard_keys;
    bool old_keys_state[256 + 3]; // 256 keyboard keys + 3 mouse buttons
    bool new_keys_state[256 + 3];



    void set_pixel_mode(Pixel::Mode mode) {
        pixel_mode = mode;
    }

    void draw_part_of_texture(Texture* src_t, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int scale = 0) {
        
        // draw only visible part
        if (src_x + src_w >= ScreenWidth)
            src_w = ScreenWidth - src_x;
        if (src_y + src_h >= ScreenHeight)
            src_h = ScreenHeight - src_y;
        for (int x = src_x; x < src_w; x++) {
            for (int y = src_y; y < src_h; y++) {

                Pixel p = src_t->get_pixel(x, y);
                // if pixel is transparent, dont draw it later on
                if (pixel_mode == Pixel::Mode::Transparent 
                        && src_t->is_transparent_pixel(p)) { 
                    p.a = 0;
                }

                // scaling
                for (int sx = 0; sx <= scale; sx++)
                    for (int sy = 0; sy <= scale; sy++)
                        draw_pixel(dst_x + x + (scale * x) + sx, dst_y + y + (scale * y) + sy, p);
            }
        }
    }

    void draw_texture(Texture* src_t, int dst_x, int dst_y, int scale = 0) {
        draw_part_of_texture(src_t, 0, 0, src_t->width, src_t->height, dst_x, dst_y, scale);
    }

    void clear_screen(Pixel p = 0xFF000000) {
        draw_target->clear(p);
    }

    void set_draw_target(Texture* target) {
        if (target) draw_target = target;
        else draw_target = default_draw_target;
    }
    
    bool draw_pixel(int x, int y, Pixel p, float z = 0) {
        if (x < 0 || x >= ScreenWidth || y < 0 || y >= ScreenHeight) return false;
        if (use_depth_buffer && depth_buffer[x + y * ScreenWidth] >= z) return false;

        if (pixel_mode == Pixel::Mode::Normal) {
            if (draw_target->set_pixel(x, y, p) && use_depth_buffer)
                depth_buffer[x + y * ScreenWidth] = z;
            return true;
        }
        else if (pixel_mode == Pixel::Mode::Blend) {
            // considering opacity we blend color between old and new
            Pixel curr_pixel = draw_target->get_pixel(x, y);
            float alpha = float(p.a) / 255;
            p.r = uint8_t(alpha * p.r + (1 - alpha) * curr_pixel.r);
            p.g = uint8_t(alpha * p.g + (1 - alpha) * curr_pixel.g);
            p.b = uint8_t(alpha * p.b + (1 - alpha) * curr_pixel.b);
            p.a = 255;

            return draw_target->set_pixel(x, y, p);
        }
        else if (pixel_mode == Pixel::Mode::Transparent) {
            if (!draw_target->is_transparent_pixel(p)) { 
                return draw_target->set_pixel(x, y, p);
            }
            else if (draw_target->transparent_pixel == 0x0 && p.a == 255) // draw only opaque pixels
                return draw_target->set_pixel(x, y, p);
        }

        return false;
    }

    bool draw_rect(int x1, int y1, int x2, int y2, Pixel p) {
        // from left to right
        if (x1 > x2)
            swap(x1, x2);
        // from top to bottom
        if (y1 > y2)
            swap(y1, y2);

        for (int x = x1; x <= x2; x++)
            draw_pixel(x, y1, p);
        for (int y = y1; y <= y2; y++)
            draw_pixel(x1, y, p);

        for (int x = x1; x <= x2; x++)
            draw_pixel(x, y2, p);
        for (int y = y1; y <= y2; y++)
            draw_pixel(x2, y, p);

        return true;
    }


    bool draw_filled_rect(int x1, int y1, int x2, int y2, Pixel border_p, Pixel fill_p = NULL) {
        // from left to right
        if (x1 > x2)
            swap(x1, x2);
        // from top to bottom
        if (y1 > y2)
            swap(y1, y2);

        for (int x = x1; x <= x2; x++) {
            for (int y = y1; y <= y2; y++) {
                Pixel p = fill_p == NULL ? border_p : fill_p;
                // if fill pixel parameter is set
                if (fill_p != NULL && (x == x1 || x == x2 || y == y1 || y == y2)) {
                    p = border_p;
                }
                draw_pixel(x, y, p);
            }
        }

        return true;
    }



    bool draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, Pixel p, float z0 = 0, float z1 = 0, float z2 = 0) {
        // thanks https://habr.com/en/post/248159/
        // filled triangle scanline algorithm
        if (y0 == y1 && y0 == y2) return false;

        // sort; y0 - lowest point, y2 - highest
        if (y0 > y1) {
            swap(y0, y1);
            swap(x0, x1);
            swap(z0, z1);
        }
        if (y0 > y2) {
            swap(y0, y2);
            swap(x0, x2);
            swap(z0, z2);
        }
        if (y1 > y2) {
            swap(y1, y2);
            swap(x1, x2);
            swap(z1, z2);
        }
            
        int total_height = y2 - y0;
        for (int y = 0; y < total_height; y++) {
            bool second_half = y > y1 - y0 || y1 == y0;
            int segment_height = second_half ? y2 - y1 : y1 - y0;
            float alpha = (float)y / total_height;
            float beta = (float)(y - (second_half ? y1 - y0 : 0)) / segment_height;

            int a_x = x0 + (x2 - x0) * alpha;
            int a_y = y0 + (y2 - y0) * alpha;

            int b_x = second_half ? x1 + (x2 - x1) * beta : x0 + (x1 - x0) * beta;
            int b_y = second_half ? y1 + (y2 - y1) * beta : y0 + (y1 - y0) * beta;

            if (a_x > b_x) {
                swap(a_x, b_x);
                swap(a_y, b_y);
            }

            for (int x = a_x; x <= b_x; x++) {
                draw_pixel(x, y0 + y, p, (z0+z1+z2)/3);
            }
        }

        return true;
    }


    void draw_triangle3d(int x0, int y0, int z0, int x1, int y1, int z1, int x2, int y2, int z2, Pixel color) {
        draw_triangle3d(Vector<int>(x0,y0,z0), Vector<int>(x1, y1, z1), Vector<int>(x2, y2, z2), color);
    }

    void draw_triangle3d(Vector<int> t0, Vector<int> t1, Vector<int> t2, Pixel color) {
        if (t0.y == t1.y && t0.y == t2.y) return;

        if (t0.y > t1.y) std::swap(t0, t1);
        if (t0.y > t2.y) std::swap(t0, t2);
        if (t1.y > t2.y) std::swap(t1, t2);

        int total_height = t2.y - t0.y;
        for (int i = 0; i < total_height; i++) {
            bool second_half = i > t1.y - t0.y || t1.y == t0.y;
            int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;
            
            double alpha = (double)i / total_height;
            double beta = (double)(i - (second_half ? t1.y - t0.y : 0)) / segment_height;
            
            Vector<int> A = t0 + Vector<double>(t2 - t0) * alpha;
            Vector<int> B = second_half ? t1 + Vector<double>(t2 - t1) * beta : t0 + Vector<double>(t1 - t0) * beta;


            if (A.x > B.x) std::swap(A, B);
            for (int j = A.x; j <= B.x; j++) {
                double phi = B.x == A.x ? 1.0 : (double)(j - A.x) / (double)(B.x - A.x);
                Vector<int> P = Vector<double>(A) + Vector<double>(B - A) * phi;
                //P.x = j; P.y = t0.y + i;

                draw_pixel(P.x, P.y, color, P.z);
            }
        }
    }


    bool draw_circle(int center_x, int center_y, int radius, Pixel p) {
        // another Brasenham's algorithm
        // http://members.chello.at/~easyfilter/bresenham.html

        int x = -radius;
        int y = 0;
        int error = 2 - 2 * radius;
        do {
            draw_pixel(center_x - x, center_y + y, p); /*   I. Quadrant */
            draw_pixel(center_x - y, center_y - x, p); /*  II. Quadrant */
            draw_pixel(center_x + x, center_y - y, p); /* III. Quadrant */
            draw_pixel(center_x + y, center_y + x, p); /*  IV. Quadrant */
            radius = error;
            if (radius <= y) {
                y += 1;
                error += y * 2 + 1;
            }
            if (radius > x || error > y) {
                x += 1;
                error += x * 2 + 1;
            }
        } while (x < 0);
        

        return true;
    }


    bool draw_filled_circle(int center_x, int center_y, int radius, Pixel border_p, Pixel fill_p = NULL) {
        // Brasenham's algorithm
        // http://members.chello.at/~easyfilter/bresenham.html
        if (fill_p == NULL) {
            fill_p = border_p;
        }

        int x = -radius;
        int y = 0;
        int error = 2 - 2 * radius;
        do {
            // draw filling part
            draw_line(center_x + x, center_y - y, center_x - x, center_y - y, fill_p);
            draw_line(center_x - x, center_y + y, center_x + x, center_y + y, fill_p);

            // draw border
            draw_pixel(center_x + x, center_y - y, border_p);
            draw_pixel(center_x - x, center_y - y, border_p);
            draw_pixel(center_x - x, center_y + y, border_p);
            draw_pixel(center_x + x, center_y + y, border_p);

            radius = error;
            if (radius <= y) {
                y += 1;
                error += y * 2 + 1;
            }
            if (radius > x || error > y) {
                x += 1;
                error += x * 2 + 1;
            }
        } while (x <= 0);

        return true;
    }


    bool draw_line(int x0, int y0, int x1, int y1, Pixel color) {

        bool steep = false;
        if (abs(y1 - y0) > abs(x1 - x0)) {
            swap(x0, y0);
            swap(x1, y1);
            steep = true;
        }
        if (x0 > x1) {
            swap(x0, x1);
            swap(y0, y1);
        }

        int dy = abs(y1 - y0);
        int dx = x1 - x0;

        int dir_y = (y1 - y0) > 0 ? 1 : -1;
        int error = 0;
        int y = y0;
        for (int x = x0; x < x1; x++) {
            if (!steep) {
                draw_pixel(x, y, color);
            }
            else {
                draw_pixel(y, x, color);
            }

            error += dy;
            if (error > dx) {
                y += dir_y;
                error -= dx;
            }
        }

        return true;
    }


    void draw_thick_line(int x0, int y0, int x1, int y1, int thickness, Pixel p) {
        vec center1(x0, y0); // start
        vec center2(x1, y1); // end

        // difference between center and edge
        vec diff = (center2 - center1).perpendicular_2d().normalize() * thickness * 0.5;
        // two start points
        vec p1 = center1 + diff;
        vec p2 = center1 - diff;

        // two end points
        vec p3 = center2 + diff;
        vec p4 = center2 - diff;

        // we draw it as two triangles
        draw_triangle(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p);
        draw_triangle(p4.x, p4.y, p2.x, p2.y, p3.x, p3.y, p);
    }


    void update_mouse_coords(int x, int y) {
        // map mouse coords from window coords to screen coords
        mouseX = (int)map_interval(x, ViewX, (double)WindowWidth - ViewX, 0, ScreenWidth);
        mouseY = (int)map_interval(y, ViewY, (double)WindowHeight - ViewY, 0, ScreenHeight);

        constrain(mouseX, 0, ScreenWidth-1);
        constrain(mouseY, 0, ScreenHeight-1);
    }


    void update_window_size(int w, int h) {
        WindowWidth = w;
        WindowHeight = h;

        update_viewport();
    }

    void update_viewport() {
        float width = float(ScreenWidth) * float(PixelWidth);
        float height = float(ScreenHeight) * float(PixelHeight);
        float aspect_ratio = width / height;

        ViewW = WindowWidth;
        ViewH = int(float(ViewW) / aspect_ratio);

        if (ViewH > WindowHeight) {
            ViewH = WindowHeight;
            ViewW = int(float(ViewH) * aspect_ratio);
        }

        // center viewport with "black bars"
        ViewX = int((WindowWidth - ViewW) / 2);
        ViewY = int((WindowHeight - ViewH) / 2);
    }


    void set_window_name(LPCWSTR new_name) {
        SetWindowText(hWnd, new_name);
    }

    static LONG WINAPI windows_events(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        static GameEngine* ge;

        auto print = [wParam, lParam]() {cout << "wParam: " << wParam << "\nlParam: " << lParam << endl; return 0; };

        switch (uMsg) {
        case WM_CREATE:
            ge = (GameEngine*)(((LPCREATESTRUCT)lParam)->lpCreateParams);
            return 0;

        case WM_SIZE:
            ge->update_window_size(LOWORD(lParam), HIWORD(lParam));   return 0;
            
        case WM_MOUSEMOVE: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            ge->update_mouse_coords(x, y);
            return 0;
        }
        case WM_KEYDOWN: ge->new_keys_state[wParam] = true; return 0;
        case WM_KEYUP: ge->new_keys_state[wParam] = false; return 0;

        case WM_LBUTTONDOWN: ge->new_keys_state[key::L_MOUSE] = true; return 0;
        case WM_LBUTTONUP: ge->new_keys_state[key::L_MOUSE] = false; return 0;
        case WM_MBUTTONDOWN: ge->new_keys_state[key::M_MOUSE] = true; return 0;
        case WM_MBUTTONUP: ge->new_keys_state[key::M_MOUSE] = false; return 0;
        case WM_RBUTTONDOWN: ge->new_keys_state[key::R_MOUSE] = true; return 0;
        case WM_RBUTTONUP: ge->new_keys_state[key::R_MOUSE] = false; return 0;

        case WM_CLOSE: ge->is_engine_done = true;    return 0;
        case WM_DESTROY: PostQuitMessage(0);     return 0;
        }

        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }




    int init_opengl(BYTE type, DWORD flags) {
        int         pf;
        PIXELFORMATDESCRIPTOR pfd;
        hDC = GetDC(hWnd);


        //PIXELFORMATDESCRIPTOR pfd =
        //{
        //    sizeof(PIXELFORMATDESCRIPTOR), 1,
        //    PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        //    PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        //    PFD_MAIN_PLANE, 0, 0, 0, 0
        //};

        /* there is no guarantee that the contents of the stack that become
           the pfd are zeroed, therefore _make sure_ to clear these bits. */
        memset(&pfd, 0, sizeof(pfd));
        pfd.nSize = sizeof(pfd);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | flags;
        pfd.iPixelType = type;
        pfd.cColorBits = 32;
        pfd.iLayerType = PFD_MAIN_PLANE;

        pf = ChoosePixelFormat(hDC, &pfd);
        if (pf == 0) {
            MessageBox(NULL, L"ChoosePixelFormat() failed: Cannot find a suitable pixel format.", L"Error", MB_OK);
            return FAILURE;
        }
        if (SetPixelFormat(hDC, pf, &pfd) == FALSE) {
            MessageBox(NULL, L"SetPixelFormat() failed: Cannot set format specified.", L"Error", MB_OK);
            return FAILURE;
        }

        hRC = wglCreateContext(hDC);
        if (pf == 0) {
            MessageBox(NULL, L"wglCreateContext() failed", L"Error", MB_OK);
            return FAILURE;
        }
        wglMakeCurrent(hDC, hRC);

        glViewport(ViewX, ViewY, ViewW, ViewH);


        return SUCCESS;
    }

    int create_window()
    {
        WNDCLASS    wc;
        ZeroMemory(&wc, sizeof(WNDCLASS));
        wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpfnWndProc = windows_events;
        wc.lpszClassName = L"ge";

        if (!RegisterClass(&wc)) {
            MessageBox(NULL, L"RegisterClass() failed:  Cannot register window class.", L"Error", MB_OK);
            return FAILURE;
        }
        
        WindowWidth = ScreenWidth * PixelWidth;
        WindowHeight = ScreenHeight * PixelHeight;
        ViewW = WindowWidth;
        ViewH = WindowHeight;

        int pos_offset = 150;
        DWORD dwStyleEx = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
        DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;


        if (full_screen)
        {
            dwStyleEx = 0;
            dwStyle = WS_VISIBLE | WS_POPUP;
            pos_offset = 0;
            HMONITOR hmon = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
            MONITORINFO mi = { sizeof(mi) };
            if (!GetMonitorInfo(hmon, &mi)) return NULL;
            WindowWidth = mi.rcMonitor.right;
            WindowHeight = mi.rcMonitor.bottom;
           
        }


        update_viewport();
        RECT rWndRect = { 0, 0, WindowWidth, WindowHeight };
        AdjustWindowRectEx(&rWndRect, dwStyle, FALSE, dwStyleEx);
        int width = rWndRect.right - rWndRect.left;
        int height = rWndRect.bottom - rWndRect.top;

        hWnd = CreateWindowEx(dwStyleEx, wc.lpszClassName, window_name, dwStyle,
            pos_offset, pos_offset, width, height, NULL, NULL, GetModuleHandle(nullptr), this);

        if (hWnd == NULL) {
            MessageBox(NULL, L"CreateWindow() failed:  Cannot create a window.", L"Error", MB_OK);
            return FAILURE;
        }

        return SUCCESS;
    }



    void reset_depth_buffer() {
        for (int i = 0; i < ScreenWidth * ScreenHeight; i++) {
            depth_buffer[i] = -INFINITY;
        }
    }

    void construct_engine(int screen_w, int screen_h, int pixel_w = 1, int pixel_h = 1, bool full_screen_ = false) {
        ScreenWidth  = screen_w;
        ScreenHeight = screen_h;
        PixelWidth = pixel_w;
        PixelHeight = pixel_h;
        full_screen = full_screen_;
        frame_count = 0L;
        depth_buffer = new float[ScreenWidth * ScreenHeight];
        reset_depth_buffer();

        // default target is our screen
        default_draw_target = new Texture(ScreenWidth, ScreenHeight);
        set_draw_target(default_draw_target);

        construct_font_sheet();
        set_pixel_mode(Pixel::Mode::Normal);
    }


    void start_engine() {
        int res = create_window();
        if (res == FAILURE) {
            cout << "Window creation error!" << endl;
            return;
        }

        ShowWindow(hWnd, SW_NORMAL);


        // call main function in another thread with current instance of GameEngine as parameter
        thread t = thread(&GameEngine::engine_thread, this);


        // as usual read events from windows (keyboard, mouse buttons, closing, resizing, etc)
        MSG   msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        t.join(); // wait for engine to finish
    }



    key_status get_key(int key_code) {
       
        return keyboard_keys[toupper(key_code)];
    }

    void update_keys() {
        // compare keyboard key states on previous frame with currious frame
        int i;
        for (i = 0; i < 259; i++)
        {
            auto& old_state = old_keys_state[i];
            auto& curr_state = new_keys_state[i];
            
            if (old_state && curr_state) { // key is held
                keyboard_keys[i] = {false, true, false};
            }
            else if (!old_state && curr_state) { // key is pressed in the current frame
                keyboard_keys[i] = { true, false, false };
            }
            else if (old_state && !curr_state) { // key is released in the current frame
                keyboard_keys[i] = { false, false, true};
            }
            else {
                keyboard_keys[i] = { false, false, false };
            }

            old_keys_state[i] = new_keys_state[i];
        }

        //// same for mouse buttons
        //for (; i < 259; i++)
    }




    void draw_text(string text, int pos_x, int pos_y, Pixel p, int scale) {
        Pixel::Mode mode_backup = pixel_mode;

        if (p.a == 255)
            set_pixel_mode(Pixel::Mode::Transparent);
        else 
            set_pixel_mode(Pixel::Mode::Blend);

        int shift_x = 0;
        // fontsheet is a grid 6 by 16 chars, 8x8 pixels each
        int char_width = 8;
        int char_height = 8;
        int chars_per_row = 16;
        for (size_t i = 0; i < text.length(); i++) {
            char ch = text[i];
            if (ch == '\n') {
                pos_y += char_height * scale;
                shift_x = 0;
                continue;
            }

            int char_in_fontsheet = ch - ' '; // space is the first character in fontsheet and so we shift accordingly
            int x = char_in_fontsheet % chars_per_row;
            int y = char_in_fontsheet / chars_per_row;


            for (int col = 0; col < char_width; col++) {
                for (int row = 0; row < char_height; row++) {
                    Pixel pixel = font_sheet->get_pixel(x * char_width + col, y * char_height + row);
                    if (pixel.a != 255) continue;
                    
                    //scaling
                    for (int sx = 0; sx <= scale; sx++) 
                    {
                        for (int sy = 0; sy <= scale; sy++) 
                        {
                            draw_pixel(pos_x + shift_x + (col*scale) + sx, pos_y + (row*scale) + sy, p);
                        }
                    }
                }
            }

            shift_x += char_width * scale;
        }

        set_pixel_mode(mode_backup);
    }


    void construct_font_sheet() {

        // font_sheet: 128x48 image encoded into char array
        // 16x6 chars (8x8 pixels) starting from space character (32 in ascii) 
        string data;
        data += "?Q`0001oOch0o01o@F40o0<AGD4090LAGD<090@A7ch0?00O7Q`0600>00000000";
        data += "O000000nOT0063Qo4d8>?7a14Gno94AA4gno94AaOT0>o3`oO400o7QN00000400";
        data += "Of80001oOg<7O7moBGT7O7lABET024@aBEd714AiOdl717a_=TH013Q>00000000";
        data += "720D000V?V5oB3Q_HdUoE7a9@DdDE4A9@DmoE4A;Hg]oM4Aj8S4D84@`00000000";
        data += "OaPT1000Oa`^13P1@AI[?g`1@A=[OdAoHgljA4Ao?WlBA7l1710007l100000000";
        data += "ObM6000oOfMV?3QoBDD`O7a0BDDH@5A0BDD<@5A0BGeVO5ao@CQR?5Po00000000";
        data += "Oc``000?Ogij70PO2D]??0Ph2DUM@7i`2DTg@7lh2GUj?0TO0C1870T?00000000";
        data += "70<4001o?P<7?1QoHg43O;`h@GT0@:@LB@d0>:@hN@L0@?aoN@<0O7ao0000?000";
        data += "OcH0001SOglLA7mg24TnK7ln24US>0PL24U140PnOgl0>7QgOcH0K71S0000A000";
        data += "00H00000@Dm1S007@DUSg00?OdTnH7YhOfTL<7Yh@Cl0700?@Ah0300700000000";
        data += "<008001QL00ZA41a@6HnI<1i@FHLM81M@@0LG81?O`0nC?Y7?`0ZA7Y300080000";
        data += "O`082000Oh0827mo6>Hn?Wmo?6HnMb11MP08@C11H`08@FP0@@0004@000000000";
        data += "00P00001Oab00003OcKP0006@6=PMgl<@440MglH@000000`@000001P00000000";
        data += "Ob@8@@00Ob@8@Ga13R@8Mga172@8?PAo3R@827QoOb@820@0O`0007`0000007P0";
        data += "O`000P08Od400g`<3V=P0G`673IP0`@3>1`00P@6O`P00g`<O`000GP800000000";
        data += "?P9PL020O`<`N3R0@E4HC7b0@ET<ATB0@@l6C4B0O`H3N7b0?P01L3R000000020";

        font_sheet = new Texture(128, 48);
        int x = 0, y = 0;
        for (int b = 0; b < 1024; b += 4)
        {
            // read 4 bytes and 'or' them
            uint32_t sym1 = data[b + 0] - 48;
            uint32_t sym2 = data[b + 1] - 48;
            uint32_t sym3 = data[b + 2] - 48;
            uint32_t sym4 = data[b + 3] - 48;
            uint32_t r = sym1 << 18 | sym2 << 12 | sym3 << 6 | sym4;

            for (int i = 0; i < 24; i++)
            {
                // if i-th bit is on, draw pixel
                int k = r & (1 << i) ? 255 : 0;
                font_sheet->set_pixel(x, y, Pixel(k, k, k, k));
                y += 1;
                if (y == 48) { 
                    x += 1; 
                    y = 0; 
                }
            }
        }
    }


    GLvoid gl_draw_text(const char* fmt, ...) {
        char text[256];
        va_list ap;

        if (fmt == NULL) return;

        va_start(ap, fmt);
        vsprintf_s(text, fmt, ap);
        va_end(ap);

        glPushAttrib(GL_LIST_BIT);
        glListBase(base - 32);
        glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
        glPopAttrib();
    }

    void text_init() {
        base = glGenLists(96);
        font = CreateFont(-32, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
                                                    ANTIALIASED_QUALITY, FF_DONTCARE | DEFAULT_PITCH, TEXT("Comic Sans MS"));
        old_font = (HFONT)SelectObject(hDC, font);
        wglUseFontBitmaps(hDC, 32, 96, base);
        SelectObject(hDC, old_font);
        DeleteObject(font);
    }


    void engine_thread() {
        init_opengl(PFD_TYPE_RGBA, 0);

        glEnable(GL_TEXTURE_2D);
        glGenTextures(1, &glBuffer);
        glBindTexture(GL_TEXTURE_2D, glBuffer);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ScreenWidth, ScreenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, draw_target->get_pixels());


        on_user_start();


        text_init();


        auto t1 = std::chrono::system_clock::now();
        auto t2 = std::chrono::system_clock::now();

        long fps_counter = 0;
        float elapsed_time = 0;

        // main loop
        while (!is_engine_done) {
            t2 = std::chrono::system_clock::now();
            std::chrono::duration<float> frame_duration = t2 - t1;
            t1 = t2;
            float f_frame_duration = frame_duration.count();

            on_user_update(f_frame_duration);
           

            // Display Graphics
            glViewport(ViewX, ViewY, ViewW, ViewH);

            // Copy pixel array into texture
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ScreenWidth, ScreenHeight, GL_RGBA, GL_UNSIGNED_BYTE, draw_target->get_pixels());
 
            // Display texture on screen
            glBegin(GL_QUADS);
                glTexCoord2f(0.0, 1.0); glVertex2f(-1.0f, -1.0f);
                glTexCoord2f(0.0, 0.0); glVertex2f(-1.0f, 1.0f);
                glTexCoord2f(1.0, 0.0); glVertex2f(1.0f, 1.0f);
                glTexCoord2f(1.0, 1.0); glVertex2f(1.0f, -1.0f);
            glEnd();

            glRasterPos2f(0, 0);    // center opengl text (if any)

            // show texure to user
            SwapBuffers(hDC);   

            update_keys();


            // update fps counter
            elapsed_time += f_frame_duration;
            fps_counter += 1;
            frame_count += 1;
            if (elapsed_time >= 1) {
                elapsed_time -= 1;

                wstring new_name = window_name;
                new_name += L" ; FPS: " + to_wstring(fps_counter);
                set_window_name(new_name.c_str());
                fps_counter = 0;
            }
        }

        wglDeleteContext(hRC);
        PostMessage(hWnd, WM_DESTROY, 0, 0);
    }


    // should (or may) be implemented by user
    virtual void on_user_start() {};
    virtual void on_user_update(float dt) {};
};







class MyModel {
public:
    vector<vec> vertices;
    vector<Vector<int>> edges;

    MyModel(string file_name) {
        load_3d_model(file_name);
    }


    void load_3d_model(string file_name) {
        ifstream file;
        file.open(file_name, ios_base::in);

        char buffer[99];

        string name, arg1, arg2, arg3;
        for (int i = 0; !file.eof(); i++)
        {
            file >> name;
            file >> arg1;
            file >> arg2;
            file >> arg3;
            file.getline(buffer, 99);

            if (name == "v") {
                vertices.push_back(vec(stod(arg1), stod(arg2), stod(arg3)));
            }
            else if (name == "f") {
                string first_v = arg1.substr(0, arg1.find_first_of("/", 0));
                string second_v = arg2.substr(0, arg2.find_first_of("/", 0));
                string third_v = arg3.substr(0, arg3.find_first_of("/", 0));

                edges.push_back(Vector<int>(stoi(first_v), stoi(second_v), stoi(third_v)));
            }
        }
    }


    void rotate_around_y_axe(double angle) {
        double s = sin(angle);
        double c = cos(angle);

        for (auto& v : vertices) {
            double znew = v.z * c - v.x * s;
            double xnew = v.z * s + v.x * c;
            double ynew = v.y;

            v.x = xnew;
            v.z = znew;
        }
    }

    //void draw() {
    //    for (auto& v : vertices) {
    //        auto mapped_x = map_interval(v.x, -1, 1, 0, ScreenWidth);
    //        auto mapped_y = map_interval(v.y, 1, -1, 0, ScreenHeight);

    //        draw_pixel(mapped_x, mapped_y, color::white);
    //    }

    //    for (auto& e : model->edges) {

    //        auto v1 = model->vertices[e.x - 1];
    //        auto v1_x = map_interval(v1.x, -1, 1, 0, ScreenWidth);
    //        auto v1_y = map_interval(v1.y, 1, -1, 0, ScreenHeight);

    //        auto v2 = model->vertices[e.y - 1];
    //        auto v2_x = map_interval(v2.x, -1, 1, 0, ScreenWidth);
    //        auto v2_y = map_interval(v2.y, 1, -1, 0, ScreenHeight);

    //        auto v3 = model->vertices[e.z - 1];
    //        auto v3_x = map_interval(v3.x, -1, 1, 0, ScreenWidth);
    //        auto v3_y = map_interval(v3.y, 1, -1, 0, ScreenHeight);

    //        draw_triangle(v1_x, v1_y, v2_x, v2_y, v3_x, v3_y, 
    //            lerp_color(color::white, color::black, map_interval(v3.z, -1, 1, 0, 1)), v1.z, v2.z, v3.z);
    //    }
    //}
};



class PerlinNoise {
    unique_ptr<double[]> gradient_grid;
    int size_x;
    int size_y;
    int dimensions = 2;

public:
    PerlinNoise(int max_x, int max_y) : size_x(max_x), size_y(max_y) {
        gradient_grid = make_unique<double[]>(max_x * max_y * dimensions);
    }

    // fill grid with random unit vectors
    void fill_grid() {
        for (int i = 0; i < size_x * size_y*dimensions; i += dimensions) {
            auto v = vec().random_2d();
            //cout << v << ", " << v.magnitude_squared() << endl;
            gradient_grid[i] = v.x;
            gradient_grid[i+1] = v.y;
        }
    }

    // classical linear interpolation
    double lerp(double x0, double x1, double v) {
        return (1 - v) * x0 + v * x1;
    }

    // dot product of vectors gradient_grid[ix][iy] and [x - ix, y - iy]
    double dot_product_vector(int ix, int iy, double x, double y) {
        double dx = x - double(ix);
        double dy = y - double(iy);

        return dx * gradient_grid[ix + iy * size_x] + dy * gradient_grid[ix + iy * size_x + 1];
    }

    double perlin(double x, double y) {
        int x0 = int(x) % (size_x - 1);
        int x1 = x0 + 1;
        int y0 = int(y) % (size_y - 1);
        int y1 = y0 + 1;

        // a0[x0,y0] a1[x1,y0]
        // b0[x0,y1] b1[x1,y1]

        double value_x = x - double(x0);
        double value_y = y - double(y0);


        double a0 = dot_product_vector(x0, y0, x, y);
        double a1 = dot_product_vector(x1, y0, x, y);
        double b0 = dot_product_vector(x0, y1, x, y);
        double b1 = dot_product_vector(x1, y1, x, y);
        
        // bicubic interpolation
        double top_x = lerp(a0, a1, value_x);
        double bottom_x = lerp(b0, b1, value_x);
        double lerped_y = lerp(top_x, bottom_x, value_y);

        return lerped_y;
    }
};



class Hand {
    struct Bone {
        vec p1, p2;
        double angle, length;

        Bone() {}
        Bone(vec p1_, double angle_, double length_) {
            p1 = p1_;
            angle = angle_;
            length = length_;
            calculate_end_point();
        }

        void draw(GameEngine* g, Pixel p = color::white) {
            g->draw_line(p1.x, p1.y, p2.x, p2.y, p);
        }

        void rotate(double a) {
            angle += a;
        }

        void calculate_end_point(double parent_angle_ = 0) {
            p2 = p1 + p1.unit_vector(angle + parent_angle_) * length;
        }
    };
    vector<Bone> bones;
    Pixel col_;

public:
    Hand(int x, int y, int segments, int len, double start_angle, Pixel col = color::white) : col_(col) {
        init(x, y, segments, len, start_angle);
    }

    void init(int x, int y, int segments, int len, double start_angle) {
        for (int i = 0; i < segments; i++) {
            bones.push_back(Bone(vec(x, y), i == 0 ? start_angle : 0, len));
        }
    }

    void move_to(int x, int y) {
        for (int i = bones.size() - 1; i >= 0; i--) {
            auto vec_to_endpoint = (bones[bones.size() - 1].p2 - bones[i].p1);
            auto current_vec = vec(x, y) - bones[i].p1;
            auto angle_between = current_vec.angle_between(vec_to_endpoint);

            bones[i].rotate(-angle_between);
            recalculate_bones();
        }
    }

    void recalculate_bones() {
        double a = 0;
        bones[0].calculate_end_point();
        for (int i = 0; i < bones.size() - 1; i++) {
            a += bones[i].angle;
            bones[i + 1].p1 = bones[i].p2;
            bones[i + 1].calculate_end_point(a);
        }
    }


    void draw(GameEngine* g) {
        for (int i = 0; i < bones.size(); i++) {
            bones[i].draw(g, col_);
        }
    }
};


