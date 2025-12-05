=========================================================================
UFMS - Universidade Federal de Mato Grosso do Sul
Faculdade de Computação - FACOM
Computação Gráfica 2025
=========================================================================

TRABALHO PRÁTICO 1 (TP1) INTEGRADO À PROVA 2 (PARTE PRÁTICA)

-------------------------------------------------------------------------
AUTORES
-------------------------------------------------------------------------
1. Arthur Barbosa
2. Caio Mendes
3. Marcus Augusto

-------------------------------------------------------------------------
RESUMO DA ENTREGA
-------------------------------------------------------------------------
Este projeto consiste na aplicação de renderização baseada em física (PBR)
requisitada no Trabalho Prático 1.

Adicionalmente, conforme permitido nas instruções da Prova 2, esta aplicação
já integra as funcionalidades requisitadas na avaliação prática P2 (as funcionalidades da P2 não interferem com os objetivos do TP1:

1. Primitiva 'Box' (Caixa/AABB):
   - Implementada na classe 'Box' (derivada de Shape3).
   - Suporta dimensões customizáveis (width, height, depth).
   - Visualização em OpenGL e interseção via Ray Casting implementadas.
   - Geração automática de malha triangular com normais corretas.

2. Aceleração BVH (Bounding Volume Hierarchy):
   - A classe 'RayCaster' constrói e utiliza uma BVH para acelerar os 
     testes de interseção raio-cena.
   - Atores (PBRActor) e malhas (TriangleMesh) possuem suporte a cálculo 
     de AABB (bounds) no espaço do mundo.
   - Reconstrução dinâmica da BVH quando necessário.

3. Seleção de Atores:
   - Funcionalidade de seleção implementada via Ray Casting.
   - O clique do mouse dispara um raio que identifica o ator mais próximo.
   - Ator selecionado é destacado visualmente (wireframe) e na GUI.

-------------------------------------------------------------------------
FUNCIONALIDADES PRINCIPAIS
-------------------------------------------------------------------------

Renderização PBR (Physically Based Rendering):
  - Implementação do modelo de iluminação Cook-Torrance BRDF.
  - Suporte a materiais metálicos e dielétricos.
  - Presets de materiais: Cobre, Alumínio, Prata, Titânio, Ouro.
  - Controle de propriedades: Albedo (Od), Specular (Os), Roughness, 
    Metalness.
  - Sistema de iluminação com 3 luzes pontuais configuráveis.

Cena de Demonstração:
  - 48 objetos organizados em 4 fileiras:
    * Fileira 1: Esferas dielétricas com cores variadas (12 objetos)
    * Fileira 2: Caixas dielétricas com cores variadas (12 objetos)
    * Fileira 3: Esferas metálicas com presets (12 objetos)
    * Fileira 4: Caixas metálicas com presets (12 objetos)
  - Variação de roughness de 0.1 a 0.9 ao longo de cada fileira.
  - Plano de chão com material ciano.
  - Sistema de iluminação de 3 pontos (Key, Fill, Back lights).

Pipeline de Renderização:
  - Renderização em tempo real via OpenGL (PBRRenderer).
  - Pipeline alternativo de Ray Casting (RayCaster) para testes e 
    seleção de objetos.
  - Suporte a múltiplas primitivas: Esferas, Planos, Caixas.

Interface Gráfica (ImGui):
  - Painel de controle da câmera (FOV, Near/Far planes).
  - Controle de luzes (posição, cor, tipo de falloff).
  - Edição de materiais PBR em tempo real.
  - Lista de atores da cena com seleção interativa.
  - Visualização e edição de propriedades do ator selecionado.

-------------------------------------------------------------------------
ARQUITETURA DO PROJETO
-------------------------------------------------------------------------

Estrutura de Classes Principais:

  - MainWindow: Gerenciamento da janela, eventos e coordenação dos 
                subsistemas.
  - PBRRenderer: Pipeline de renderização OpenGL com shading PBR.
  - RayCaster: Pipeline de Ray Casting com BVH para interseções.
  - Scene: Container de atores e luzes da cena.
  - PBRActor: Entidade da cena (geometria + material + transformações).
  - PBRMaterial: Estrutura de dados para propriedades PBR.
  - Shape3: Interface base para primitivas geométricas.
    * Sphere: Primitiva esférica.
    * Plane: Primitiva plana.
    * Box: Primitiva de caixa (AABB).
  - SceneBuilder: Utilitário para construção da cena padrão.
  - GUIInitializer: Configuração e gerenciamento da interface ImGui.

-------------------------------------------------------------------------
COMO COMPILAR
-------------------------------------------------------------------------

Pré-requisitos:
  - Visual Studio 2022 (com suporte a C++20 e ferramentas de build).
  - CMake 3.16 ou superior.
  - Bibliotecas GLFW e ImGui (já inclusas/configuradas na estrutura do 
    projeto).
  - Biblioteca base 'cg' (fornecida pelo professor).

Opção A: Visual Studio 2022 (IDE)
  1. Certifique-se de compilar a biblioteca base 'cg' primeiro:
     - Abra: cg/build/vs2022/cg.sln
     - Configure para 'Debug' ou 'Release' (x64).
     - Build Solution (F7).

  2. Configure o projeto tp1 com CMake:
     - Navegue até o diretório raiz do projeto.
     - Execute CMake para gerar os arquivos de projeto:
       cmake -B build -S .
     - Ou use o CMake GUI apontando para o diretório raiz.

  3. Abra a solução gerada:
     - Arquivo: build/vs2022/tp1.sln (ou similar, dependendo da 
       configuração do CMake)
     - Configure para a mesma opção escolhida anteriormente 
       (Debug/Release, x64).
     - Build Solution (F7).

Opção B: Linha de Comando (CMake + MSBuild)
  1. Abra o "Developer Command Prompt for VS 2022".
  2. Navegue até o diretório raiz do projeto.
  3. Configure com CMake:
     cmake -B build -S .
  4. Compile:
     cmake --build build --config Release
  5. Ou use MSBuild diretamente:
     msbuild build/vs2022/tp1.sln /p:Configuration=Release /p:Platform=x64

Opção C: CMake (Geração de Ninja)
  1. Configure:
     cmake -B build -S . -G Ninja
  2. Compile:
     cmake --build build --config Release

-------------------------------------------------------------------------
COMO EXECUTAR
-------------------------------------------------------------------------

Após a compilação, o executável estará localizado em:
  - Debug:   apps/tp1/Debug/tp1.exe (ou build/vs2022/Debug/tp1.exe)
  - Release: apps/tp1/Release/tp1.exe (ou build/vs2022/Release/tp1.exe)

Nota: O caminho exato depende da configuração do CMakeLists.txt. O 
executável pode estar no diretório do projeto (apps/tp1/) ou no diretório 
de build, conforme configurado.

Execute o arquivo diretamente ou através do Visual Studio (F5).

-------------------------------------------------------------------------
CONTROLES E INTERAÇÃO
-------------------------------------------------------------------------

Movimentação da Câmera (Teclado):
  (W) Mover para frente
  (S) Mover para trás
  (A) Mover para esquerda
  (D) Mover para direita
  (Q) Mover para cima
  (Z) Mover para baixo

Controle de Visualização (Mouse):
  [Scroll Wheel]        Zoom in/out
  [Middle Click]        Pan (arrastar a câmera)
  [Alt + Right Click]   Rotacionar câmera (Orbit ao redor do ponto focal)

Funcionalidades P2 (Interação):
  [Left Click]          Selecionar Ator (Dispara raio da câmera através 
                        do ponto clicado)
                        *O ator selecionado será destacado com wireframe 
                         e aparecerá selecionado na GUI*

Interface Gráfica (GUI - ImGui):
  - Camera: 
    * Ajuste de FOV (Field of View)
    * Configuração de Near/Far planes
    * Reset da câmera para posição inicial
  
  - Lights: 
    * Lista de todas as luzes da cena
    * Controle de posição (X, Y, Z)
    * Controle de cor (RGB)
    * Seleção de tipo de falloff (Constant, Linear, Quadratic)
    * Ativação/desativação de luzes individuais
  
  - Materials: 
    * Edição de propriedades PBR do ator selecionado:
      - Albedo (Od): Cor difusa do material
      - Specular (Os): Cor especular (F0 para metais)
      - Roughness: Rugosidade da superfície [0.0, 1.0]
      - Metalness: Metalicidade [0.0, 1.0]
    * Aplicação de presets de materiais metálicos
    * Criação de materiais dielétricos customizados
  
  - Assets: 
    * Lista de todos os atores da cena
    * Seleção de ator por clique na lista
    * Visualização de propriedades do ator selecionado
    * Controle de visibilidade dos atores

-------------------------------------------------------------------------
DETALHES TÉCNICOS
-------------------------------------------------------------------------

Modelo de Iluminação PBR:
  - Implementação do modelo Cook-Torrance BRDF.
  - Distribuição normal: GGX (Trowbridge-Reitz).
  - Função de geometria: Smith com aproximação de Schlick-GGX.
  - Função de Fresnel: Schlick.
  - Suporte a múltiplas luzes pontuais com diferentes tipos de atenuação.

Sistema de Interseção:
  - Ray Casting com transformação de raios entre espaços (Mundo <-> Local).
  - Teste de interseção otimizado para cada primitiva:
    * Esfera: Teste analítico direto.
    * Plano: Teste de interseção raio-plano.
    * Caixa: Teste de interseção raio-AABB (Slab Method).
  - BVH para aceleração espacial em cenas complexas.

Transformações:
  - Sistema de transformações hierárquicas (TransformableObject).
  - Cálculo automático de matriz normal (normalMatrix) para shading correto.
  - Suporte a translação, rotação e escala.

Gerenciamento de Recursos:
  - Uso de Reference<T> para gerenciamento automático de memória.
  - SharedObject para materiais compartilhados.
  - Geração dinâmica de malhas triangulares para primitivas.

-------------------------------------------------------------------------
ESTRUTURA DE ARQUIVOS
-------------------------------------------------------------------------

apps/tp1/
  - Main.cpp              : Ponto de entrada da aplicação
  - MainWindow.h/cpp      : Gerenciamento da janela e eventos
  - PBRRenderer.h/cpp     : Pipeline de renderização OpenGL PBR
  - RayCaster.h/cpp       : Pipeline de Ray Casting com BVH
  - PBRActor.h            : Entidade da cena (geometria + material)
  - PBRMaterial.h         : Estrutura de dados PBR
  - Scene.h               : Container de atores e luzes
  - SceneBuilder.h        : Construção da cena padrão
  - Shape3.h              : Interface base para primitivas
  - Sphere.h              : Primitiva esférica
  - Plane.h               : Primitiva plana
  - Box.h                 : Primitiva de caixa (AABB)
  - Intersection.h        : Estrutura de dados para interseções
  - GUIInitializer.h/cpp  : Configuração da interface ImGui
  - CMakeLists.txt        : Configuração de build CMake

-------------------------------------------------------------------------
LINK DO VÍDEO DEMONSTRATIVO
-------------------------------------------------------------------------

https://drive.google.com/file/d/1P7oy7nmXkz1dse7lTL419Cu37AjeVhc7/view?usp=sharing

-------------------------------------------------------------------------