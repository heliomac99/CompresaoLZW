# Compresão LZW

Este repositório contém a implementação do algoritmo de compressão e descompressão de dados utilizando Lempel-Ziv-Welch (LZW), amplamente reconhecido por sua eficiência em compressão sem perdas. O objetivo é explorar como o LZW pode ser aplicado a diferentes tipos de arquivos, avaliando seu desempenho e características.


## 📚 Descrição do Projeto

O projeto implementa o algoritmo LZW em C++, utilizando uma Trie como estrutura de dados para gerenciar o dicionário de prefixos. A implementação é capaz de:

- **Comprimir arquivos**: Gerar versões compactas de arquivos de entrada em um formato binário.
- **Descomprimir arquivos**: Restaurar os arquivos comprimidos ao seu formato original.
- **Analisar desempenho**: Registrar métricas como tempo de execução, taxa de compressão e tamanho do dicionário.


## 📂 Estrutura do Repositório

- **include/**: Arquivos cabeçalhos 
- **test/**: Arquivos de teste utilizados nas análises experimentais (.txt e .bmp).
- **results/**: Dados resultantes das execuções, armazenados em formato JSON, gráficos e relatório em pdf.
- **testsCompressed/**: Arquivos comprimidos dos testes da pasta test/.
- **testsDescompressed**: Arquivos desomprimidos da pasta testsCompressed/.
- **triePrefixada.cpp**: Implementação do trabalho.
- **graficos.py**: Gera gráficos a partir dos dados de desempenho.
- **README.md**: Este documento.


## 🚀 Principais funcionalidades

- **Compressão de arquivos**: Gera versões compactadas dos arquivos de entrada em formato binário.
- **Descompressão de arquivos**: Restaura os arquivos compactados para o formato original.
- **Geração de Arquivo com dados de desempenho**: Coleta métricas como:
  1) **Taxa de compressão**: Redução no tamanho do arquivo original.
  2) **Tempo de execução**: Para compressão e descompressão.
  3) **Tamanho do dicionário**: Quantidade de entradas no dicionário da Trie.

## 🛠️ Como Usar

### 1. Clonar o Repositório
```bash
git clone https://github.com/heliomac99/CompresaoLZW.git
cd CompresaoLZW
```

### 2. Compilar o Código
Preferencialmente em uma máquina linux e
Certifique-se de ter o compilador **g++** instalado.
```bash
g++ triePrefixada.cpp -o trie
```

### 3. Executar o Programa
```bash
./trie -t [modo teste] -[int] [tamanho máximo dos códigos da Trie]
```

**Parâmetros disponíveis:**
- `-t`: Ativa o modo de teste para compressão e descompressão.
- `[int]`: Define o valor de `maxBits`, determinando o limite do tamanho dos códigos na Trie.

Exemplo de execução:
```bash
./trie -t -12
```
Ao executar o programa, um arquivo dados.json é gerado na pasta results/, e para gerar os gráficos a partir desses dados basta:

### 4. Gerar gráficos de desemepenho
Certifique-se de ter as bibliotecas de python listadas em requirements.txt baixadas.
```bash
python3 geraGraficos.py
```
Gráficos serão gerados no diretório results/graphic/
