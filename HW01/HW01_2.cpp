#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdint>
#include <string>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 強制 1-byte 對齊，確保 Header 剛好為 44 bytes
#pragma pack(push, 1)
struct WavHeader {
    // RIFF Chunk Descriptor
    char     riff[4];        // "RIFF"
    uint32_t chunkSize;      // 36 + subchunk2Size
    char     wave[4];        // "WAVE"

    // "fmt " Subchunk
    char     fmt[4];         // "fmt "
    uint32_t subchunk1Size;  // 16
    uint16_t audioFormat;    // 1 (PCM)
    uint16_t numChannels;    // 1 (Mono)
    uint32_t sampleRate;     // 固定為 44100
    uint32_t byteRate;       // sampleRate * numChannels * bitsPerSample / 8
    uint16_t blockAlign;     // numChannels * bitsPerSample / 8
    uint16_t bitsPerSample;  // 16

    // "data" Subchunk
    char     data[4];        // "data"
    uint32_t subchunk2Size;  // 音訊數據大小
};
#pragma pack(pop)

void generateMyWav2(double k, uint32_t s, double r, const std::string& filename = "Mywav2.wav") {
    // 題目規定：以 44100 的取樣頻率模擬指定取樣頻率 s
    const uint32_t fileSampleRate = 44100;

    if (s >= fileSampleRate) {
        std::cerr << "警告: 題目要求 s < 44100，請確認輸入數值！" << std::endl;
    }

    // 計算重採樣係數 c = floor(44100 / s)
    uint32_t c = fileSampleRate / s;
    if (c == 0) c = 1;

    // 總點數依 44100 Hz 計算
    uint32_t totalSamples = static_cast<uint32_t>(fileSampleRate * r);

    // 16-bit PCM 規格
    uint16_t numChannels = 1;
    uint16_t bitsPerSample = 16;
    uint32_t dataSize = totalSamples * numChannels * (bitsPerSample / 8);

    // 組裝 44-byte Header (取樣率填入 44100)
    WavHeader header;
    header.riff[0] = 'R'; header.riff[1] = 'I'; header.riff[2] = 'F'; header.riff[3] = 'F';
    header.chunkSize = 36 + dataSize;
    header.wave[0] = 'W'; header.wave[1] = 'A'; header.wave[2] = 'V'; header.wave[3] = 'E';

    header.fmt[0] = 'f';  header.fmt[1] = 'm';  header.fmt[2] = 't';  header.fmt[3] = ' ';
    header.subchunk1Size = 16;
    header.audioFormat = 1;
    header.numChannels = numChannels;
    header.sampleRate = fileSampleRate; // 固定為 44100
    header.byteRate = fileSampleRate * numChannels * (bitsPerSample / 8);
    header.blockAlign = numChannels * (bitsPerSample / 8);
    header.bitsPerSample = bitsPerSample;

    header.data[0] = 'd'; header.data[1] = 'a'; header.data[2] = 't'; header.data[3] = 'a';
    header.subchunk2Size = dataSize;

    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile.is_open()) {
        std::cerr << "無法建立檔案: " << filename << std::endl;
        return;
    }

    // 寫入 Header
    outFile.write(reinterpret_cast<const char*>(&header), sizeof(WavHeader));

    // 計算階梯狀振幅並寫入數據
    // 公式: wav(i) = cos(2 * PI * k * [i / c] / s)
    const double maxAmplitude = 32767.0;
    for (uint32_t i = 0; i < totalSamples; ++i) {
        // [i / c] 在 C++ 整數除法自動為 floor()
        uint32_t stepIndex = i / c;
        double t_sim = static_cast<double>(stepIndex) / static_cast<double>(s);
        double sampleVal = std::cos(2.0 * M_PI * k * t_sim);

        int32_t intVal = static_cast<int32_t>(sampleVal * maxAmplitude);
        if (intVal > 32767) intVal = 32767;
        if (intVal < -32768) intVal = -32768;

        int16_t sample16 = static_cast<int16_t>(intVal);
        outFile.write(reinterpret_cast<const char*>(&sample16), sizeof(int16_t));
    }

    outFile.close();

    std::cout << "\nsuccess: " << filename << std::endl;
    std::cout << "k: " << k << " Hz" << std::endl;
    std::cout << "s: " << s << " Hz" << std::endl;
    std::cout << "實際檔案取樣頻率: " << fileSampleRate << " Hz" << std::endl;
    std::cout << "c: " << c << " (每 " << c << " 個點保持相同振幅)" << std::endl;
    std::cout << "r: " << r << " 秒" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc == 4) {
        double k = std::stod(argv[1]);
        uint32_t s = static_cast<uint32_t>(std::stoul(argv[2]));
        double r = std::stod(argv[3]);
        generateMyWav2(k, s, r);
    } else {
        double k, r;
        uint32_t s;
        std::cout << "k (Hz, ex 440): ";
        if (!(std::cin >> k)) return 1;
        std::cout << "s (ex 44100): ";
        if (!(std::cin >> s)) return 1;
        std::cout << "r (ex 2): ";
        if (!(std::cin >> r)) return 1;

        generateMyWav2(k, s, r);
    }
    return 0;
}