=========================================================================
UFMS - Universidade Federal de Mato Grosso do Sul
Faculdade de Computação - FACOM
Computação Gráfica 2025
=========================================================================

TRABALHO PRÁTICO 1 (TP1) INTEGRAda À PROVA 2 (PARTE PRÁTICA)

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
já integra as funcionalidades requisitadas na avaliação prática (P2):

1. Primitiva 'Box' (Caixa/AABB):
   - Implementada na classe 'Box' (derivada de Shape).
   - Visualização em OpenGL e interseção via Ray Casting implementadas.

2. Aceleração BVH (Bounding Volume Hierarchy):
   - A classe 'RayCaster' constrói e utiliza uma BVH para acelerar os 
     testes de interseção raio-cena.
   - Atores e malhas (TriangleMesh) possuem suporte a cálculo de AABB (bounds).

3. Seleção de Atores:
   - Funcionalidade de seleção implementada via Ray Casting.
   - O clique do mouse dispara um raio que identifica o ator mais próximo.

-------------------------------------------------------------------------
COMO COMPILAR
-------------------------------------------------------------------------
Pré-requisitos:
- Visual Studio 2022 (com suporte a C++ e ferramentas de build).
- Bibliotecas GLFW e ImGui (já inclusas/configuradas na estrutura do projeto).

Opção A: Visual Studio 2022 (IDE)
1. Certifique-se de compilar a biblioteca base 'cg' primeiro:
   - Abra: cg/build/vs2022/cg.sln
   - Configure para 'Debug' ou 'Release' (x64).
   - Build Solution.

2. Abra a solução da aplicação:
   - Arquivo: build/vs2022/myapp.sln
     (Nota: Ajuste o caminho relativo se necessário conforme a estrutura do zip).
   - Configure para a mesma opção escolhida anteriormente (Debug/Release, x64).
   - Build Solution (F7).

Opção B: Linha de Comando (MSBuild)
1. Abra o "Developer Command Prompt for VS 2022".
2. Navegue até o diretório de build do projeto.
3. Execute:
   msbuild myapp.sln /p:Configuration=Release /p:Platform=x64

-------------------------------------------------------------------------
COMO EXECUTAR
-------------------------------------------------------------------------
Após a compilação, o executável estará localizado em:
- Debug:   build/vs2022/Debug/myapp.exe
- Release: build/vs2022/Release/myapp.exe

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
  [Middle Click]        Pan (arrastar)
  [Alt + Right Click]   Rotacionar câmera (Orbit)

Funcionalidades P2 (Interação):
  [Left Click]          Selecionar Ator (Dispara raio da câmera)
                        *O ator selecionado será destacado na GUI*

Interface Gráfica (GUI):
  - Camera: Ajuste de FOV, Near/Far planes.
  - Lights: Controle de posição e intensidade das luzes.
  - Materials: Edição de propriedades PBR (Albedo, Roughness, Metallic).
  - Assets: Seleção de objetos da cena.

-------------------------------------------------------------------------
LINK DO VÍDEO DEMONSTRATIVO

-------------------------------------------------------------------------