=========================================================================
UFMS - Universidade Federal de Mato Grosso do Sul
Faculdade de Computação - FACOM
Computação Gráfica 2025
=========================================================================

TRABALHO PRÁTICO 2 (TP2)

-------------------------------------------------------------------------
AUTORES
-------------------------------------------------------------------------
1. Arthur Barbosa
2. Caio Kwiatkoski
3. Marcus Augusto

-------------------------------------------------------------------------
RESUMO DA ENTREGA
-------------------------------------------------------------------------
Este projeto estende o traçador de raios básico do cgdemo (projeto Ds) com 
duas funcionalidades principais:

1. Transparência Especular (Refração):
   - Implementação da Lei de Snell para cálculo de refração de raios.
   - Suporte a materiais transparentes com índice de refração (IOR).
   - Tratamento de reflexão interna total (RIT).
   - Gerenciamento do índice de refração do meio atual durante o traçado 
     recursivo de raios usando vetor (para casos de objetos parcialmente 
     intersectados).
   - Adaptação do método shadow() para considerar objetos transparentes.

2. Superamostragem Adaptativa:
   - Técnica de antialiasing que reduz o serrilhado nas bordas dos objetos.
   - Amostragem inicial nos 4 cantos de cada pixel.
   - Subdivisão recursiva baseada em variação de cor dentro do pixel.
   - Sistema de buffers para reutilização eficiente de raios entre pixels 
     adjacentes.
   - Suporte opcional a jitter (amostragem aleatória) para reduzir padrões 
     regulares.

-------------------------------------------------------------------------
FUNCIONALIDADES PRINCIPAIS
-------------------------------------------------------------------------

Transparência e Refração:
  - Cálculo da direção de refração usando a Lei de Snell.
  - Suporte a múltiplos índices de refração (ar, água, vidro, diamante, etc.).
  - Detecção automática de entrada/saída de objetos transparentes.
  - Gerenciamento de IOR usando vetor para casos complexos (objetos 
    parcialmente intersectados).
  - Reflexão interna total quando o discriminante é negativo.
  - Sombras que permitem passagem de luz através de objetos transparentes.

Superamostragem Adaptativa:
  - Amostragem inicial de 4 raios por pixel (um em cada canto).
  - Cálculo da média das cores e comparação com limiar de adaptatividade.
  - Subdivisão recursiva em até 4 níveis (máximo 25 raios por pixel).
  - Janela deslizante e buffer de linha para reutilização de raios.
  - Jitter opcional para amostragem aleatória (reduz padrões regulares).
  - Parâmetros configuráveis via GUI:
    * Adaptive Threshold: limiar de variação para subdivisão (0.0-1.0)
    * Max Subdivision Level: níveis máximos de subdivisão (0-4)
    * Use Jitter: habilita/desabilita amostragem aleatória

Interface Gráfica (ImGui):
  - Controle de parâmetros de ray tracing:
    * Max Recursion Level: profundidade máxima de recursão (0-20)
    * Min Weight: peso mínimo dos raios secundários (0.001-1.0)
    * Adaptive Threshold: limiar para superamostragem adaptativa
    * Max Subdivision Level: níveis máximos de subdivisão
    * Use Jitter: habilita jitter
    * Scene IOR: índice de refração da cena (padrão: 1.0)
  - Carregamento de cenas a partir de arquivos (.scn)
  - Visualização da imagem renderizada
  - Estatísticas de renderização (número de raios, hits, tempo)

Cenas de Teste:
  - Cenas com objetos opacos e transparentes.
  - Cenas com objetos transparentes dentro de objetos transparentes 
    (obrigatório).
  - Cenas com fontes de luz dentro de objetos transparentes (obrigatório).
  - Cenas para teste de antialiasing (objetos pequenos, bordas, padrões).

-------------------------------------------------------------------------
ARQUITETURA DO PROJETO
-------------------------------------------------------------------------

Estrutura de Classes Principais:

  - MainWindow: Gerenciamento da janela, eventos e coordenação dos 
                subsistemas.
  - RayTracer: Classe principal do traçador de raios estendida com:
    * Método shade(): cálculo de iluminação com suporte a refração
    * Método shadow(): verificação de sombras com objetos transparentes
    * Método scan(): renderização com superamostragem adaptativa
    * Método adapt(): subdivisão recursiva adaptativa
    * Estruturas de dados para buffers de superamostragem
  - Scene: Container de atores e luzes da cena (herdado do Ds).
  - Material: Estrutura de material com propriedades:
    * transparency: cor de transparência (Ot)
    * ior: índice de refração (η)

-------------------------------------------------------------------------
COMO COMPILAR
-------------------------------------------------------------------------

Pré-requisitos:
  - Visual Studio 2022 (com suporte a C++20 e ferramentas de build).
  - CMake 3.16 ou superior.
  - Bibliotecas GLFW e ImGui (já inclusas/configuradas na estrutura do 
    projeto).
  - Biblioteca base 'cg' (fornecida pelo professor).

IMPORTANTE:
  **Recomenda-se fortemente compilar o projeto em modo *Release*, pois o 
  desempenho do ray tracing é significativamente mais rápido do que no 
  modo Debug.**

Opção A: Visual Studio 2022 (IDE)
  1. Certifique-se de compilar a biblioteca base 'cg' primeiro:
     - Abra: cg/build/vs2022/cg.sln
     - Configure para 'Debug' ou 'Release' (x64).
     - Build Solution (F7).

  2. Configure o projeto tp2 com CMake:
     - Navegue até o diretório raiz do projeto.
     - Execute CMake para gerar os arquivos de projeto:
       cmake -B build -S .
     - Ou use o CMake GUI apontando para o diretório raiz.

  3. Abra a solução gerada:
     - Arquivo: build/vs2022/tp2.sln (ou similar, dependendo da 
       configuração do CMake)
     - Configure para a mesma opção escolhida anteriormente 
       (Debug/Release, x64).
     - Build Solution (F7).
     **Observação:** Para melhor desempenho, utilize *Release* sempre que possível.

Opção B: Linha de Comando (CMake + MSBuild)
  1. Abra o "Developer Command Prompt for VS 2022".
  2. Navegue até o diretório raiz do projeto.
  3. Configure com CMake:
     cmake -B build -S .
  4. Compile:
     cmake --build build --config Release
  5. Ou use MSBuild diretamente:
     msbuild build/vs2022/tp2.sln /p:Configuration=Release /p:Platform=x64

Opção C: CMake (Geração de Ninja)
  1. Configure:
     cmake -B build -S . -G Ninja
  2. Compile:
     cmake --build build --config Release

-------------------------------------------------------------------------
COMO EXECUTAR
-------------------------------------------------------------------------

Após a compilação, o executável estará localizado em:
  - Debug:   apps/tp2/Debug/tp2.exe (ou build/vs2022/Debug/tp2.exe)
  - Release: apps/tp2/Release/tp2.exe (ou build/vs2022/Release/tp2.exe)

**Recomendação:** Execute sempre o binário de *Release*.  
A diferença de desempenho é muito grande: no modo Debug, o ray tracing 
fica sensivelmente mais lento devido às checagens extras do compilador 
e otimizações desativadas.

Execute o arquivo diretamente ou através do Visual Studio (F5).

IMPORTANTE: Execute o programa a partir do diretório apps/tp2/ para que 
os caminhos relativos das cenas e assets funcionem corretamente.

-------------------------------------------------------------------------
CONTROLES E INTERAÇÃO
-------------------------------------------------------------------------

Interface Gráfica (GUI - ImGui):

  - File: 
    * Open: Carregar uma cena a partir de arquivo (.scn)
    * As cenas devem estar no diretório assets/scenes/
  
  - View: 
    * Ray Tracer: Abre a janela de renderização e inicia o ray tracing
  
  - Ray Tracing: 
    * Max Recursion Level: Profundidade máxima de recursão para raios 
      secundários (reflexão e refração).
      - Valores recomendados: 6-10
      - Valores muito altos (>15) podem ser muito lentos
    
    * Min Weight: Peso mínimo para continuar traçando raios secundários.
      - Valores menores = mais raios traçados = melhor qualidade = mais lento
      - Valores recomendados: 0.01-0.1
    
    * Adaptive Threshold: Limiar de variação de cor para subdivisão 
      adaptativa.
      - Valores menores = mais subdivisões = melhor qualidade = mais lento
      - Valores recomendados: 0.05-0.2
      - 0.0 = sempre subdivide (máxima qualidade, muito lento)
      - 1.0 = nunca subdivide (sem antialiasing)
    
    * Max Subdivision Level: Níveis máximos de subdivisão para 
      superamostragem.
      - 0 = sem antialiasing (1 raio por pixel, mais rápido)
      - 1 = até 4 raios por pixel
      - 2 = até 9 raios por pixel (recomendado)
      - 3 = até 16 raios por pixel
      - 4 = até 25 raios por pixel (máxima qualidade, muito lento)
    
    * Use Jitter: Habilita amostragem aleatória (jitter).
      - Reduz padrões regulares de aliasing
      - Pode melhorar a qualidade visual
      - Ligeiramente mais lento
    
    * Scene IOR: Índice de refração do meio da cena (padrão: 1.0 para ar).
      - Valores típicos: 1.0 (ar), 1.33 (água), 1.5 (vidro)

Estatísticas de Renderização:
  - O console exibe:
    * Número total de raios traçados
    * Número total de hits (interseções)
    * Tempo de renderização em milissegundos

-------------------------------------------------------------------------
DETALHES TÉCNICOS
-------------------------------------------------------------------------

Refração:
  - Implementação da Lei de Snell para cálculo da direção refratada.
  - Fórmula: T = η12 * L + (η12 * C1 - C2) * N
    onde:
    * η12 = η1/η2 (razão dos índices de refração)
    * L = direção de incidência
    * N = normal da superfície
    * C1 = cos(θ1) = -L · N
    * C2 = cos(θ2) = sqrt(1 - η12²(1 - C1²))
  - Verificação de reflexão interna total (discriminante < 0).
  - Gerenciamento de IOR usando vetor para casos de objetos parcialmente 
    intersectados (em vez de pilha simples).

Superamostragem Adaptativa:
  - Algoritmo recursivo que subdivide pixels baseado em variação de cor.
  - Métrica de variação: δ = max(|C - M|), onde C é a cor de um canto e 
    M é a média das cores dos 4 cantos.
  - Se δ < threshold para todos os cantos, usa a média.
  - Caso contrário, subdivide em 4 subpixels e repete.
  - Sistema de buffers (_lineBuffer e _window) para reutilização de raios:
    * Evita retraçar raios compartilhados entre pixels adjacentes
    * Reduz significativamente o número de raios traçados
  - Jitter: deslocamento aleatório uniforme de ±1/8 do tamanho do subpixel.

Sombras com Transparência:
  - O método shadow() itera através de todas as interseções do raio de luz.
  - Objetos transparentes são ignorados (raio continua).
  - Apenas objetos opacos bloqueiam completamente a luz.

Transformações:
  - Sistema de transformações do Ds (herdado).
  - Cálculo automático de normais e interseções no espaço do mundo.

Gerenciamento de Recursos:
  - Uso de Reference<T> para gerenciamento automático de memória.
  - BVH (Bounding Volume Hierarchy) para aceleração de interseções.
  - Buffers alocados uma vez e reutilizados durante a renderização.

-------------------------------------------------------------------------
ESTRUTURA DE ARQUIVOS
-------------------------------------------------------------------------

apps/tp2/
  - Main.cpp              : Ponto de entrada da aplicação
  - MainWindow.h/cpp      : Gerenciamento da janela e eventos
  - RayTracer.h           : Cabeçalho do ray tracer (estendido)
  - RayTracer.cpp         : Implementação das extensões
  - CMakeLists.txt        : Configuração de build CMake
  - README.txt            : Este arquivo
  - assets/
    - scenes/             : Arquivos de cena (.scn)
    - fonts/              : Fontes para ImGui
    - meshes/             : Malhas 3D (se houver)
  - reader/               : Parser de cenas (do cgdemo)

Nota: Os arquivos principais modificados são RayTracer.h e RayTracer.cpp. 
Os demais arquivos são herdados do cgdemo original.

-------------------------------------------------------------------------
LINK DO VÍDEO DEMONSTRATIVO
-------------------------------------------------------------------------

https://drive.google.com/drive/folders/1B2rZR4-0L-ltVU3M-xWsPjYiWG-9Itgs?usp=sharing

-------------------------------------------------------------------------
