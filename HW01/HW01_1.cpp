#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdint>
#include <vector>
#include <string>
using namespace std;

// 定義 π 的數值
const double PI = 3.14159265358979323846;

// 確保編譯器以 1-byte 對齊打包，避免 struct 內部 padding 破壞 WAV Header 大小
#pragma pack(push, 1)
struct WavHeader {
    // RIFF Chunk Descriptor
    char     riff[4]        = {'R', 'I', 'F', 'F'}; // "RIFF"
    uint32_t chunkSize      = 0;                     // 36 + subchunk2Size
    char     wave[4]        = {'W', 'A', 'V', 'E'}; // "WAVE"

    // "fmt " Sub-chunk
    char     fmt[4]         = {'f', 'm', 't', ' '}; // "fmt "
    uint32_t subchunk1Size  = 16;                    // 16 for PCM
    uint16_t audioFormat    = 1;                     // 1 for Linear PCM (無壓縮)
    uint16_t numChannels    = 1;                     // 1 for Mono (單聲道)
    uint32_t sampleRate     = 44100;                 // 取樣頻率 s
    uint32_t byteRate       = 0;                     // sampleRate * numChannels * bitsPerSample/8
    uint16_t blockAlign     = 0;                     // numChannels * bitsPerSample/8
    uint16_t bitsPerSample  = 16;                    // 16 bits per sample

    // "data" Sub-chunk
    char     data[4]        = {'d', 'a', 't', 'a'}; // "data"
    uint32_t subchunk2Size  = 0;                     // 實際音訊資料的位元組數 (bytes)
};
#pragma pack(pop)

void generateMyWav1(double k, uint32_t s, double r, const std::string& filename = "Mywav1.wav") {
    // 1. 計算總採樣點數 N = s * r
    uint32_t totalSamples = static_cast<uint32_t>(s * r);

    // 2. 準備 44-byte Header
    WavHeader header;
    header.sampleRate    = s;
    header.numChannels   = 1;  // 單聲道
    header.bitsPerSample = 16; // 16-bit PCM
    header.blockAlign    = header.numChannels * (header.bitsPerSample / 8);
    header.byteRate      = header.sampleRate * header.blockAlign;
    header.subchunk2Size = totalSamples * header.blockAlign;
    header.chunkSize     = 36 + header.subchunk2Size;

    // 3. 計算每個採樣點的餘弦波數值並轉為 16-bit 整數 (-32768 ~ 32767)
    vector<int16_t> audioData(totalSamples);
    const double maxAmplitude = 32767.0;

    for (uint32_t i = 0; i < totalSamples; ++i) {
        // 題目公式: wav(i) = cos(2 * pi * k * i / s)
        double sampleVal = std::cos(2.0 * PI * k * static_cast<double>(i) / static_cast<double>(s));
        
        // 映射到 16-bit 整數
        int32_t intVal = static_cast<int32_t>(sampleVal * maxAmplitude);

        // 避免數值溢位截斷
        if (intVal > 32767) intVal = 32767;
        if (intVal < -32768) intVal = -32768;

        audioData[i] = static_cast<int16_t>(intVal);
    }

    // 4. 以二進位 (binary) 寫入檔案
    ofstream outFile(filename, std::ios::binary);
    if (!outFile.is_open()) {
        cerr << "error: 無法建立或開啟檔案 " << filename << endl;
        return;
    }

    // 寫入 44 bytes Header
    outFile.write(reinterpret_cast<const char*>(&header), sizeof(WavHeader));

    // 寫入音訊 PCM 數據
    outFile.write(reinterpret_cast<const char*>(audioData.data()), audioData.size() * sizeof(int16_t));
    outFile.close();

    cout << "success: " << filename << "\n";
    cout << "k: " << k << " Hz\n";
    cout << "s: " << s << " Hz\n";
    cout << "r: " << r << " 秒\n";
}

int main(int argc, char* argv[]) {
    double k = 0.0;
    uint32_t s = 0;
    double r = 0.0;

    // 支援指令列參數模式 (方便 Demo 時直接敲指令): ./generate_wav1 <k> <s> <r>
    if (argc == 4) {
        k = std::stod(argv[1]);
        s = static_cast<uint32_t>(std::stoul(argv[2]));
        r = std::stod(argv[3]);
        generateMyWav1(k, s, r);
    } else {
        // 互動輸入模式
        //cout << "=== 練習題一: Mywav1.wav 產生器 (C++) ===\n";
        cout << "k (Hz, ex 440): ";
        if (!(std::cin >> k)) return 1;

        cout << "s (ex 44100): ";
        if (!(std::cin >> s)) return 1;

        cout << "r (ex 2): ";
        if (!(std::cin >> r)) return 1;

        generateMyWav1(k, s, r);
    }

    return 0;
}