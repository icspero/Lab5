#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdint>

using namespace std;

// базовый интерфейс для кодирования данных
class DataCoder {
public:
    virtual void encode(vector<uint8_t>& data) = 0;
    virtual void decode(vector<uint8_t>& data) = 0;
    virtual ~DataCoder() = default;
};

// базовый класс для обработки данных
class FileData {
protected:
    vector<uint8_t> data;

public:
    FileData(const string& filename) {
        ifstream file(filename, ios::binary);
        if (file) {
            file.seekg(0, ios::end);
            size_t size = file.tellg();
            file.seekg(0, ios::beg);
            data.resize(size);
            file.read(reinterpret_cast<char*>(data.data()), size);
            file.close();
        } else {
            cerr << "Error opening file!" << endl;
        }
    }

    void printData() {
        for (const auto& byte : data) {
            cout << hex << (int)byte << " ";
        }
        cout << dec << endl;
    }

    vector<uint8_t>& getData() {
        return data;
    }
};

// шифр цезаря
class CaesarCipher : public DataCoder {
private:
    int shift;
public:
    CaesarCipher(int shift) : shift(shift) {}

    void encode(vector<uint8_t>& data) override {
        for (auto& byte : data) {
            byte = (byte + shift) % 256; // простой сдвиг для шифрования
        }
    }

    void decode(vector<uint8_t>& data) override {
        for (auto& byte : data) {
            byte = (byte - shift + 256) % 256; // обратный сдвиг для дешифрования
        }
    }
};

// класс для XOR шифрования
class XORCipher : public DataCoder {
private:
    uint8_t key;
public:
    XORCipher(uint8_t key) : key(key) {}

    void encode(vector<uint8_t>& data) override {
        for (auto& byte : data) {
            byte ^= key; // побитное сравнение(если биты различны - 1, иначе 0)
        }
    }

    void decode(vector<uint8_t>& data) override {
        for (auto& byte : data) {
            byte ^= key; // побитное сравнение(если биты различны - 1, иначе 0)
        }
    }
};

// декоратор, который добавляет дополнительную функциональность к шифрованию
class DataCoderDecorator : public DataCoder {
protected:
    DataCoder* coder;

public:
    DataCoderDecorator(DataCoder* coder) : coder(coder) {}

    virtual void encode(vector<uint8_t>& data) override {
        coder->encode(data); // вызываем encode на реальном шифраторе
    }

    virtual void decode(vector<uint8_t>& data) override {
        coder->decode(data); // вызываем decode на реальном шифраторе
    }

    virtual ~DataCoderDecorator() {
        delete coder;
    }
};

// декоратор для сжатия данных
class CompressionDecorator : public DataCoderDecorator {
public:
    CompressionDecorator(DataCoder* coder) : DataCoderDecorator(coder) {}

    void encode(vector<uint8_t>& data) override {
        // сжимаем данные (просто для примера, сжимаем каждую пару одинаковых байтов)
        vector<uint8_t> compressedData;
        for (size_t i = 0; i < data.size(); ++i) {
            if (i > 0 && data[i] == data[i-1]) {
                compressedData.push_back(data[i] + 1); // пример сжатия
            } else {
                compressedData.push_back(data[i]);
            }
        }
        data = compressedData; // обновляем данные
        DataCoderDecorator::encode(data); // шифруем сжатые данные
    }

    void decode(vector<uint8_t>& data) override {
        DataCoderDecorator::decode(data); // дешифруем данные
        // распаковываем сжатые данные
        vector<uint8_t> decompressedData;
        for (size_t i = 0; i < data.size(); ++i) {
            if (i > 0 && data[i] == data[i-1] + 1) {
                decompressedData.push_back(data[i-1]);
            } else {
                decompressedData.push_back(data[i]);
            }
        }
        data = decompressedData;
    }
};

int main() {
    // загрузка данных из файла
    FileData fileData("data.txt");
    // создаем цепочку декораторов: шифрование и сжатие
    DataCoder* caesarCipher = new CaesarCipher(3); // цезарь со сдвигом 3
    DataCoder* xorCipher = new XORCipher(0xAA);    // XOR шифрование с ключом 0xAA
    DataCoder* compressor = new CompressionDecorator(caesarCipher);

    compressor->encode(fileData.getData());

    cout << "Encrypted data: ";
    fileData.printData();

    compressor->decode(fileData.getData());

    cout << "Decrypted data: ";
    fileData.printData();

    delete compressor;

    return 0;
}