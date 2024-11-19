import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

dados = pd.read_json("results/dados.json")

"""---

- Como a média da taxa de compressão varia de acordo com o maxBits?
"""

df_compressao = dados[dados['tipo'] == 'Compressão']
media_compressao = df_compressao.groupby('maxBits')['taxa'].mean().reset_index()

plt.figure(figsize=(12, 8))
sns.lineplot(x='maxBits', y='taxa', data=media_compressao, marker='o')
plt.title('Média da Taxa de Compressão por maxBits')
plt.xlabel('maxBits')
plt.ylabel('Média da Taxa de Compressão (%)')
plt.grid(True)
output_file = "results/graphic/mediaTaxa_por_maxbits_grafico.png"
plt.savefig(output_file)

"""A taxa de compressão dos dados aumenta a medida que diminuímos o tamanho máximo do código dos prefixos

---

- Como o tempo de execução varia de acordo com o maxBits?
"""

media_tempo = dados.groupby('maxBits')['tempo'].mean().reset_index()

plt.figure(figsize=(12, 8))
sns.lineplot(x='maxBits', y='tempo', data=media_tempo, marker='o')
plt.title('Tempo Médio de Execução por maxBits')
plt.xlabel('maxBits')
plt.ylabel('Tempo Médio de Execução (ms)')
plt.grid(True)
output_file = "results/graphic/mediaTempo_por_maxbits_grafico.png"
plt.savefig(output_file)

"""O tempo médio de execução tanto para compressão quanto descompressão aumenta na medida que diminuímos nosso limitante de tamanho do código dos prefixos da árvore(maxBits)

---

- Como o tamanho medio do dicionario varia de acordo com o maxBits
"""

media_tamanho_dicionario = dados.groupby('maxBits')['tamanhoDicionario'].mean().reset_index()

plt.figure(figsize=(12, 8))
sns.lineplot(x='maxBits', y='tamanhoDicionario', data=media_tamanho_dicionario, marker='o')
plt.title('Tamanho Médio do Dicionário por maxBits')
plt.xlabel('maxBits')
plt.ylabel('Tamanho Médio do Dicionário')
plt.grid(True)
output_file = "results/graphic/mediaDicionario_por_maxbits_grafico.png"
plt.savefig(output_file)

"""O tamanho do dicionário não varia a medida que variamos nosso maxBits

---

- Tempo médio de Compressão VS Tempo médio de descompressão
"""

media_tempo_compressao = dados[dados['tipo'] == 'Compressão']['tempo'].mean()
media_tempo_descompressao = dados[dados['tipo'] == 'Descompressão']['tempo'].mean()

media_tempo = pd.DataFrame({
    'Tipo de Operação': ['Compressão', 'Descompressão'],
    'Tempo Médio (ms)': [media_tempo_compressao, media_tempo_descompressao]
})

plt.figure(figsize=(10, 6))
sns.barplot(x='Tipo de Operação', y='Tempo Médio (ms)', data=media_tempo)
plt.title('Tempo Médio de Compressão vs Descompressão')
plt.xlabel('Tipo de Operação')
plt.ylabel('Tempo Médio (ms)')
plt.grid(True)
output_file = "results/graphic/comp_vs_desc_grafico.png"
plt.savefig(output_file)

"""A compressão gasta muito mais tempo independente do maxbits, tipo e tamanho dos dados.


---

- Qual a relação do tamanho do dicionário com o tempo classificados por tipo de processamento
"""

plt.figure(figsize=(12, 6))
sns.scatterplot(data=dados, x='tamanhoDicionario', y='tempo', hue='tipo')
plt.title('Relação entre Tamanho do Dicionário e Tempo por Tipo')
plt.xlabel('Tamanho do Dicionário')
plt.ylabel('Tempo (ms)')
output_file = "results/graphic/tempo_espaco_grafico.png"
plt.savefig(output_file)

"""- A medida que aumentamos o tamanho do dicionário maior é o tempo de execução e maior é a diferença de tempo de execução entre compressão e descompressão para um mesmo arquivo"""