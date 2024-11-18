#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <locale>
#include <codecvt>
#include <iomanip>
#include <cmath>
#include <chrono>
#include <filesystem>
#include <vector>
#include "include/json.hpp"

#if defined(_WIN32)
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif



using namespace std;
using json = nlohmann::json;

enum Operacao {
    Compressao = 1,
    Descompressao = 2
};

// Estrutura de no da Trie
struct TrieNode {
    unordered_map<char, TrieNode*> children;
    int index = -1;  // indice da sequ�ncia, -1 se nao for um n� terminal
};

// Classe da Trie para compressao e descompressao
class Trie {
public:
    unordered_map<int, string> map;

    Trie() {
        root = new TrieNode();
        nextIndex = 256;  // ASCII de 0 a 255
    }

    ~Trie() {
        deleteTrie(root);
    }

    void insert(const string& prefix, int index) {
        TrieNode* node = root;
        for (char ch : prefix) {
            if (node->children.find(ch) == node->children.end()) {
                node->children[ch] = new TrieNode();
            }
            node = node->children[ch];
        }
        node->index = index;
        map[index] = prefix;  // Mapeia o indice para a sequ�ncia
    }

    int search(const string& prefix) {
        TrieNode* node = root;
        for (char ch : prefix) {
            if (node->children.find(ch) == node->children.end()) {
                return -1;  // Nao encontrado
            }
            node = node->children[ch];
        }
        return node->index;
    }

    int getNextIndex() {
        return nextIndex++;
    }

private:
    TrieNode* root;
    int nextIndex;

    void deleteTrie(TrieNode* node) {
        for (auto& child : node->children) {
            deleteTrie(child.second);
        }
        delete node;
    }
};

// Função que calcula o n�mero de bits necess�rios para representar um valor
int bitsNecessarios(int valor) {
    return valor > 0 ? int(ceil(log2(valor + 1))) : 1;
}

// Função de compressao LZW usando Trie
vector<int> lzwCompress(const string& data, Trie& trie) {
    // Inicializa a Trie com caracteres ASCII
    for (int i = 0; i < 256; ++i) {
        trie.insert(string(1, i), i);
        trie.map.insert({ i , string(1, i) });
    }

    vector<int> compressed;
    string currentPrefix;
    int index;

    for (char ch : data) {
        string newPrefix = currentPrefix + ch;
        index = trie.search(newPrefix);

        if (index != -1) {
            currentPrefix = newPrefix;
        }
        else {
            int currentIndex = trie.search(currentPrefix);
            if (currentIndex != -1) {
                compressed.push_back(currentIndex);
                //cout << currentIndex << endl;
            }
            else {
                throw runtime_error("Erro: Prefixo nao encontrado no dicion�rio durante a compressao.");
            }

            int nextIndex = trie.getNextIndex();
            trie.insert(newPrefix, nextIndex);
            currentPrefix = string(1, ch);
        }
    }

    if (!currentPrefix.empty()) {
        int lastIndex = trie.search(currentPrefix);
        if (lastIndex != -1) {
            compressed.push_back(lastIndex);
        }
        else {
            throw runtime_error("Erro: Prefixo final nao encontrado no dicion�rio.");
        }
    }

    return compressed;
}


// Descompressao LZW usando Trie
string lzwDecompress(const vector<int>& compressed, Trie& trie) {
    string result;
    int nextIndex = 256;

    // O primeiro c�digo sempre estar� no dicion�rio
    result += trie.map[compressed[0]];
    string currentString = trie.map[compressed[0]];

    for (size_t i = 1; i < compressed.size(); ++i) {
        int code = compressed[i];
        string entry;

        auto it = trie.map.find(code);
        if (it != trie.map.end()) {
            entry = it->second;
        }
        else if (code == nextIndex) {
            // Caso em que o c�digo � o pr�ximo a ser gerado
            entry = currentString + currentString[0];
        }
        else {
            throw runtime_error("C�digo inv�lido durante a descompressao.");
        }

        result += entry;

        // Adiciona nova sequ�ncia ao dicion�rio
        trie.insert(currentString + entry[0], nextIndex++);
        trie.map[nextIndex - 1] = currentString + entry[0];

        currentString = entry;
    }

    return result;
}

// Função para salvar dados comprimidos em arquivo
void writeCompressedToFile(const vector<int>& compressed, const string& filePath, int maxBits) {
    ofstream outFile(filePath, ios::binary);
    if (!outFile) {
        cerr << "Erro ao abrir o arquivo para escrita: " << filePath << endl;
        return;
    }

    int currentBits = 9;
    int buffer = 0;
    int bufferBits = 0;

    for (int code : compressed) {
        if (bitsNecessarios(code) > currentBits && currentBits < maxBits) {
            currentBits++;
        }

        buffer = (buffer << currentBits) | code;
        bufferBits += currentBits;

        while (bufferBits >= 8) {
            bufferBits -= 8;
            char byte = (buffer >> bufferBits) & 0xFF;
            outFile.put(byte);
        }
    }

    if (bufferBits > 0) {
        char byte = (buffer << (8 - bufferBits)) & 0xFF;
        outFile.put(byte);
    }

    outFile.close();
}

// Função para salvar dados descomprimidos em arquivo
void writeDecompressedToFile(const string& filePath, const string& content) {
    ofstream outFile(filePath, ios::binary);
    if (!outFile) {
        cerr << "Erro ao abrir o arquivo para escrita: " << filePath << endl;
        return;
    }

    outFile.write(content.data(), content.size());
    outFile.close();
}

// Função para ler dados comprimidos do arquivo
void readCompressedFromFile(const string& filePath, vector<int>& compressed, int maxBits) {
    ifstream inFile(filePath, ios::binary);
    if (!inFile) {
        cerr << "Erro ao abrir o arquivo comprimido: " << filePath << endl;
        return;
    }

    int buffer = 0;
    int bufferBits = 0;
    int currentBits = 9;  // Inicia com 9 bits por codigo

    while (inFile.peek() != EOF) {
        char byte;
        inFile.get(byte);

        buffer = (buffer << 8) | (unsigned char)byte;
        bufferBits += 8;

        while (bufferBits >= currentBits) {
            bufferBits -= currentBits;
            int code = (buffer >> bufferBits) & ((1 << currentBits) - 1);
            compressed.push_back(code);

            // Ajusta o numero de bits conforme necessario
            if (compressed.size() == (1 << currentBits) - 1 && currentBits < maxBits) {
                currentBits++;
            }
        }
    }

    inFile.close();
}

// Função para adicionar ao vetor de relatiorios, o relatorio do processo atual
void writeReport(vector<json>& relatorios, Operacao operacao, int maxBits, long int tamanhoDicionario, long int tempo, double taxa = -1) {
    json j;
    j["tipo"] = operacao == Compressao ? "Compressão" : "Descompressão";
    j["maxBits"] = maxBits;
    j["taxa"] = operacao == Compressao ? taxa : 0;
    j["tempo"] = tempo;
    j["tamanhoDicionario"] = tamanhoDicionario;

    relatorios.push_back(j);
}

//Função que imprime informações do processsamento do arquivo
void printInfo(Operacao operacao, const string& nomeArquivo, size_t tamanhoOriginal, size_t tamanhoComprimido, size_t tamanhoDescomprimido, float taxa, int tamanhoDicionario, float tempo, int maxBits) {
    if (operacao == Compressao)
        cout << "Compressao realizada com sucesso. Arquivo salvo em: " << nomeArquivo << endl;
    else
        cout << "Descompressao realizada com sucesso. Arquivo salvo em: " << nomeArquivo << endl;

    cout << "Tamanho original:\t" << tamanhoOriginal << "\tbytes\n";
    if (operacao == Compressao)
        cout << "Tamanho comprimido:\t" << tamanhoComprimido << "\tbytes\n";
    else
        cout << "Tamanho descomprimido:\t" << tamanhoDescomprimido << "\tbytes\n";

    if (operacao == Compressao) {
        cout << "Taxa:\t" << taxa << "\t%\n";
        cout << "Tamanho:\t" << tamanhoDicionario << "\telementos\n";
    }
        
    cout << "Tempo:\t" << tempo << "\tms\n\n";
    cout << "Max bits:\t" << maxBits << "\t\n\n";
}

void processarArquivo(const string& caminhoEntrada, const string& nomeArquivoComprimido, const string& caminhoSaida, int maxBits, vector<json>& relatorios) {
    // Le o arquivo de entrada
    ifstream inFile(caminhoEntrada, ios::binary);
    if (!inFile) {
        cerr << "Erro ao abrir o arquivo: " << caminhoEntrada << endl;
        return;
    }

    // Le o conte�do do arquivo
    string input((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
    inFile.close();

    // Compressao usando a Trie
    auto start = std::chrono::high_resolution_clock::now();
    Trie trie;

    vector<int> compressed = lzwCompress(input, trie);
    writeCompressedToFile(compressed, nomeArquivoComprimido, maxBits);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    long int compTime = duration.count();

    // Descompressão
    auto startdesc = std::chrono::high_resolution_clock::now();

    string decompressed = lzwDecompress(compressed, trie);
    writeDecompressedToFile(caminhoSaida, decompressed);

    auto enddesc = std::chrono::high_resolution_clock::now();
    auto durationdesc = std::chrono::duration_cast<std::chrono::milliseconds>(enddesc - startdesc);
    long int decTime = durationdesc.count();

    // Calcula os tamanhos dos arquivos para relatorio
    ifstream originalFile(caminhoEntrada, ios::binary | ios::ate);
    ifstream compressedFile(nomeArquivoComprimido, ios::binary | ios::ate);
    ifstream decompressedFile(caminhoSaida, ios::binary | ios::ate);

    if (originalFile && compressedFile && decompressedFile) {
        size_t tamanhoOriginal;
        size_t tamanhoComprimido;
        size_t tamanhoDescomprimido;

        tamanhoOriginal = originalFile.tellg();
        tamanhoComprimido = compressedFile.tellg();
        tamanhoDescomprimido = decompressedFile.tellg();

        double taxaCompressao = 100.0 * (1.0 - (double(tamanhoComprimido) / tamanhoOriginal));
        long int tamanhoDicionario = trie.getNextIndex();

        //Imprime dados de desempenho do processamento do arquivo
        printInfo(Operacao::Compressao, nomeArquivoComprimido, tamanhoOriginal, tamanhoComprimido, tamanhoDescomprimido, taxaCompressao, tamanhoDicionario, compTime, maxBits);
        writeReport(relatorios, Operacao::Compressao, maxBits, tamanhoDicionario, compTime, taxaCompressao);

        printInfo(Operacao::Descompressao, caminhoSaida, tamanhoOriginal, tamanhoComprimido, tamanhoDescomprimido, 0, tamanhoDicionario, decTime, maxBits);
        writeReport(relatorios, Operacao::Descompressao, maxBits, tamanhoDicionario, decTime);
    }
    else {
        cerr << "Erro ao calcular os tamanhos dos arquivos." << endl;
    }

    originalFile.close();
    compressedFile.close();
    decompressedFile.close();
}

// Função para obter a extensao do arquivo
string obterExtensao(const string& nomeArquivo) {
    size_t pos = nomeArquivo.find_last_of('.');
    if (pos != string::npos) {
        return nomeArquivo.substr(pos);
    }
    return "";
}

// Função para obter o nome base do arquivo (sem extensao)
string obterNomeBase(const string& nomeArquivo) {
    size_t pos = nomeArquivo.find_last_of("/\\");
    string nome = (pos != string::npos) ? nomeArquivo.substr(pos + 1) : nomeArquivo;

    pos = nome.find_last_of('.');
    if (pos != string::npos) {
        nome = nome.substr(0, pos);
    }
    return nome;
}

string wstringParaString(const wstring& wstr) {
    wstring_convert<codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}
// Função para listar arquivos no diret�rio
#if defined(_WIN32)
vector<string> listarArquivosNoDiretorio(const string& caminho) {
    vector<string> arquivos;
    wstring busca = wstring(caminho.begin(), caminho.end()) + L"\\*";
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(busca.c_str(), &findFileData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            wstring nomeArquivoW = findFileData.cFileName;
            string nomeArquivo = wstringParaString(nomeArquivoW);

            if (nomeArquivo != "." && nomeArquivo != "..") {
                arquivos.push_back(caminho + "\\" + nomeArquivo);
            }
        } while (FindNextFile(hFind, &findFileData));
        FindClose(hFind);
    }
    else {
        cerr << "Erro ao abrir o diret�rio: " << caminho << endl;
    }

    return arquivos;
}
#else
vector<string> listarArquivosNoDiretorio(const string& caminho) {
    vector<string> arquivos;
    DIR* dir;
    struct dirent* ent;
    if ((dir = opendir(caminho.c_str())) != nullptr) {
        while ((ent = readdir(dir)) != nullptr) {
            string nomeArquivo = ent->d_name;
            if (nomeArquivo != "." && nomeArquivo != "..") {
                arquivos.push_back(caminho + "/" + nomeArquivo);
            }
        }
        closedir(dir);
    }
    else {
        cerr << "Erro ao abrir o diret�rio: " << caminho << endl;
    }
    return arquivos;
}
#endif

void executarProcessamento(const string& caminhoEntrada, const string& diretorioCompressed, const string& diretorioDescompressed, int maxBits, vector<json>& relatorios) {
    string nomeBase = obterNomeBase(caminhoEntrada);
    string extensao = obterExtensao(caminhoEntrada);
    string nomeArquivoComprimido = diretorioCompressed + "/" + nomeBase + ".lzw";
    string caminhoSaida = diretorioDescompressed + "/" + nomeBase + extensao;
    processarArquivo(caminhoEntrada, nomeArquivoComprimido, caminhoSaida, maxBits, relatorios);
}

void formatJson(ofstream& relatorio, const vector<json>& relatorios) {
    if (!relatorio.is_open()) {
        throw runtime_error("Erro: Arquivo não está aberto para escrita.");
    }

    relatorio << "[";

    for (size_t i = 0; i < relatorios.size(); ++i) {
        relatorio << relatorios[i];

        // Adiciona vírgula entre os elementos, mas não no último
        if (i < relatorios.size() - 1) {
            relatorio << "," << endl;
        }
    }

    relatorio << "]" << endl;

    // Não é necessário chamar close aqui, o chamador pode fazer isso
}

int main(int argc, char* argv[]) {
    bool modoTeste = false;
    bool tamanhoFixo = false;
    int maxBits = 16;  // Padrao para 16 bits
    vector<string> arquivosEntrada;

    // Processa os argumentos da linha de comando
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "-t") {
            modoTeste = true;
        }
        else if (arg == "-9") {
            maxBits = 9;
            tamanhoFixo = true;
        }
        else if (arg == "-12") {
            maxBits = 12;
            tamanhoFixo = true;
        }
        else if (arg == "-16") {
            maxBits = 16;
            tamanhoFixo = true;
        }
        else if (arg == "-20") {
            maxBits = 20;
            tamanhoFixo = true;
        }
        else {
            arquivosEntrada.push_back(arg);
        }
    }

string directory = "relatorio";
#if defined(_WIN32)
    // Convert string to wide string for Windows API
    wstring wDirectory(directory.begin(), directory.end());
    if (!CreateDirectory(wDirectory.c_str(), NULL)) {
        if (GetLastError() != ERROR_ALREADY_EXISTS) {
            throw std::runtime_error("Erro ao criar diretório: " + directory);
        }
    }
#else
    // Linux, macOS, and other Unix-like systems
    if (mkdir(directory.c_str(), 0777) != 0 && errno != EEXIST) {
        throw std::runtime_error("Erro ao criar diretório: " + directory);
    }
#endif


    // Cria Arquivo json para escrita do relatorio
    ofstream relatorio("relatorio/dados.json");
    if (!relatorio.is_open())
        throw runtime_error("Erro ao criar relatório");

    vector<json> relatorios;

    if (modoTeste) {
        // Diret�rios para o modo de teste
        string diretorioTests = "tests";
        string diretorioCompressed = "testsCompressed";
        string diretorioDescompressed = "testsDescompressed";

        // Cria os diret�rios de saida se nao existirem
#if defined(_WIN32)
        wstring wDiretorioCompressed = wstring(diretorioCompressed.begin(), diretorioCompressed.end());
        CreateDirectory(wDiretorioCompressed.c_str(), NULL);

        wstring wDiretorioDescompressed = wstring(diretorioDescompressed.begin(), diretorioDescompressed.end());
        CreateDirectory(wDiretorioDescompressed.c_str(), NULL);
#else
        mkdir(diretorioCompressed.c_str(), 0777);
        mkdir(diretorioDescompressed.c_str(), 0777);
#endif

        // Lista os arquivos no diret�rio 'tests'
        vector<string> arquivosTeste = listarArquivosNoDiretorio(diretorioTests);

        // Processa cada arquivo de teste
        if (tamanhoFixo) { // Compressão e descompressão dos dados usando um tamanho maxBits fixo
            for (const string& caminhoEntrada : arquivosTeste)
                executarProcessamento(caminhoEntrada, diretorioCompressed, diretorioDescompressed, maxBits, relatorios);
        }

        else { // Compressão e descompressão dos dados para 4 valores diferentes de maxbits
            int bits[4] = { 9, 12, 16, 20 };
            for (int i = 0; i < 4; i++)
                for (const string& caminhoEntrada : arquivosTeste)
                    executarProcessamento(caminhoEntrada, diretorioCompressed, diretorioDescompressed, bits[i], relatorios);
        }
    }
    else {
        if (arquivosEntrada.empty()) {
            cerr << "Uso: " << argv[0] << " arquivo1 [arquivo2 ... arquivoN] [-t] [-f] [-9] [-12] [-16]" << endl;
            return 1;
        }

        // Processa os arquivos passados como argumento
        for (const string& caminhoEntrada : arquivosEntrada) {
            string nomeBase = obterNomeBase(caminhoEntrada);
            string extensao = obterExtensao(caminhoEntrada);

            string nomeArquivoComprimido = nomeBase + ".lzw";
            string caminhoSaida = "arquivo_descomprimido" + extensao;

            processarArquivo(caminhoEntrada, nomeArquivoComprimido, caminhoSaida, maxBits, relatorios);
        }
    }

    // Escreve no json os dados de desempenhos
    formatJson(relatorio, relatorios);
    return 0;
}
