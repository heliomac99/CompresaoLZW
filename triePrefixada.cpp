#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <locale>
#include <codecvt>
#include <iomanip>
#include <cmath>


#if defined(_WIN32)
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

using namespace std;

// Funçao que calcula o número de bits necessários para representar um valor
int bitsNecessarios(int valor) {
    return valor > 0 ? int(ceil(log2(valor + 1))) : 1;
}

// Estrutura de nó da Trie
struct TrieNode {
    unordered_map<char, TrieNode*> children;
    int index = -1;  // indice da sequência, -1 se nao for um nó terminal
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
        map[index] = prefix;  // Mapeia o indice para a sequência
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

// Funçao de compressao LZW usando Trie
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
                throw runtime_error("Erro: Prefixo nao encontrado no dicionário durante a compressao.");
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
            throw runtime_error("Erro: Prefixo final nao encontrado no dicionário.");
        }
    }

    return compressed;
}


// Descompressao LZW usando Trie
string lzwDecompress(const vector<int>& compressed, Trie& trie) {
    string result;
    int nextIndex = 256;

    // O primeiro código sempre estará no dicionário
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
            // Caso em que o código é o próximo a ser gerado
            entry = currentString + currentString[0];
        }
        else {
            throw runtime_error("Código inválido durante a descompressao.");
        }

        result += entry;

        // Adiciona nova sequência ao dicionário
        trie.insert(currentString + entry[0], nextIndex++);
        trie.map[nextIndex - 1] = currentString + entry[0];

        currentString = entry;
    }

    return result;
}

// Funçao para salvar dados comprimidos em arquivo
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

// Funçao para salvar dados descomprimidos em arquivo
void writeDecompressedToFile(const string& filePath, const string& content) {
    ofstream outFile(filePath, ios::binary);
    if (!outFile) {
        cerr << "Erro ao abrir o arquivo para escrita: " << filePath << endl;
        return;
    }

    outFile.write(content.data(), content.size());
    outFile.close();

    cout << "Arquivo descomprimido salvo em: " << filePath << endl;
}

// Funçao para ler dados comprimidos do arquivo
void readCompressedFromFile(const string& filePath, vector<int>& compressed, int maxBits) {
    ifstream inFile(filePath, ios::binary);
    if (!inFile) {
        cerr << "Erro ao abrir o arquivo comprimido: " << filePath << endl;
        return;
    }

    int buffer = 0;
    int bufferBits = 0;
    int currentBits = 9;  // Inicia com 9 bits por código

    while (inFile.peek() != EOF) {
        char byte;
        inFile.get(byte);

        buffer = (buffer << 8) | (unsigned char)byte;
        bufferBits += 8;

        while (bufferBits >= currentBits) {
            bufferBits -= currentBits;
            int code = (buffer >> bufferBits) & ((1 << currentBits) - 1);
            compressed.push_back(code);

            // Ajusta o número de bits conforme necessário
            if (compressed.size() == (1 << currentBits) - 1 && currentBits < maxBits) {
                currentBits++;
            }
        }
    }

    inFile.close();
}

void processarArquivo(const string& caminhoEntrada, const string& nomeArquivoComprimido, const string& caminhoSaida, bool tamanhoFixo, int maxBits) {

    // Lê o arquivo de entrada
    ifstream inFile(caminhoEntrada, ios::binary);
    if (!inFile) {
        cerr << "Erro ao abrir o arquivo: " << caminhoEntrada << endl;
        return;
    }

    // Lê o conteúdo do arquivo
    string input((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
    inFile.close();

    // Compressao usando a Trie
    Trie trie;
    vector<int> compressed = lzwCompress(input, trie);
    writeCompressedToFile(compressed, nomeArquivoComprimido, maxBits);
    string decompressed = lzwDecompress(compressed, trie);
    writeDecompressedToFile(caminhoSaida, decompressed);


    // Calcula os tamanhos dos arquivos para relatório
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

        cout << "Compressao concluida. Arquivo salvo em: " << nomeArquivoComprimido << endl;
        cout << fixed << setprecision(2);
        cout << "Tamanho original: " << tamanhoOriginal << " bytes\n";
        cout << "Tamanho comprimido: " << tamanhoComprimido << " bytes\n";
        cout << "Tamanho descomprimido: " << tamanhoDescomprimido << " bytes\n";
        cout << "Taxa de compressao: " << taxaCompressao << "%\n";
    }
    else {
        cerr << "Erro ao calcular os tamanhos dos arquivos." << endl;
    }

    originalFile.close();
    compressedFile.close();
    decompressedFile.close();
    
    
   /* cout << "Descompressao concluida. Arquivo salvo em: " << caminhoSaida << endl;*/
}

// Funçao para obter a extensao do arquivo
string obterExtensao(const string& nomeArquivo) {
    size_t pos = nomeArquivo.find_last_of('.');
    if (pos != string::npos) {
        return nomeArquivo.substr(pos);
    }
    return "";
}

// Funçao para obter o nome base do arquivo (sem extensao)
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
// Funçao para listar arquivos no diretório
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
        cerr << "Erro ao abrir o diretório: " << caminho << endl;
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
        cerr << "Erro ao abrir o diretório: " << caminho << endl;
    }
    return arquivos;
}
#endif

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

    if (modoTeste) {
        // Diretórios para o modo de teste
        string diretorioTests = "tests";
        string diretorioCompressed = "testsCompressed";
        string diretorioDescompressed = "testsDescompressed";

        // Cria os diretórios de saida se nao existirem
#if defined(_WIN32)
        wstring wDiretorioCompressed = wstring(diretorioCompressed.begin(), diretorioCompressed.end());
        CreateDirectory(wDiretorioCompressed.c_str(), NULL);

        wstring wDiretorioDescompressed = wstring(diretorioDescompressed.begin(), diretorioDescompressed.end());
        CreateDirectory(wDiretorioDescompressed.c_str(), NULL);
#else
        mkdir(diretorioCompressed.c_str(), 0777);
        mkdir(diretorioDescompressed.c_str(), 0777);
#endif

        // Lista os arquivos no diretório 'tests'
        vector<string> arquivosTeste = listarArquivosNoDiretorio(diretorioTests);

        // Processa cada arquivo de teste
        for (const string& caminhoEntrada : arquivosTeste) {
            string nomeBase = obterNomeBase(caminhoEntrada);
            string extensao = obterExtensao(caminhoEntrada);

            string nomeArquivoComprimido = diretorioCompressed + "/" + nomeBase + ".lzw";
            string caminhoSaida = diretorioDescompressed + "/" + nomeBase + extensao;

            processarArquivo(caminhoEntrada, nomeArquivoComprimido, caminhoSaida, tamanhoFixo, maxBits);
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

            processarArquivo(caminhoEntrada, nomeArquivoComprimido, caminhoSaida, tamanhoFixo, maxBits);
        }
    }

    return 0;
}
