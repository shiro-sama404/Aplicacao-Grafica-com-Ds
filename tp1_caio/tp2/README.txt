================================================================================
TRABALHO PRÁTICO 2 - RAY TRACER COM TRANSPARÊNCIA E SUPERAMOSTRAGEM ADAPTATIVA
================================================================================

AUTOR(ES):
[Seu nome aqui]

================================================================================
DESCRIÇÃO
================================================================================

Este trabalho estende o ray tracer básico do projeto Ds (cgdemo) com duas 
funcionalidades principais:

1. TRANSPARÊNCIA E REFRAÇÃO ESPECULAR
   - Implementação da Lei de Snell para cálculo de refração
   - Suporte a objetos transparentes com índice de refração (IOR)
   - Tratamento de reflexão interna total (RIT)
   - Tracking do índice de refração do meio atual durante o traçado recursivo
   - Adaptação do método shadow() para ignorar objetos transparentes

2. SUPERAMOSTRAGEM ADAPTATIVA
   - Amostragem inicial nos 4 cantos de cada pixel
   - Subdivisão recursiva baseada em variação de cor
   - Buffer de reutilização de raios para eficiência
   - Suporte a jitter (amostragem aleatória) opcional
   - Parâmetros configuráveis: threshold e níveis máximos de subdivisão

PRINCIPAIS DESAFIOS E SOLUÇÕES:

- Refração: A maior dificuldade foi determinar corretamente se o raio está 
  entrando ou saindo do objeto para calcular o IOR correto. A solução foi 
  usar o sinal do produto escalar entre a direção do raio e a normal.

- Tracking de IOR: Foi necessário adicionar um parâmetro currentIOR aos 
  métodos trace() e shade() para rastrear o índice de refração do meio atual.

- Eficiência do Antialiasing: Implementação de um buffer 2D para armazenar 
  cores de raios já traçados, permitindo reutilização entre pixels adjacentes 
  e subdivisões.

- Shadow com Transparência: Modificação do método shadow() para iterar 
  através de todas as interseções e ignorar objetos transparentes, retornando 
  true apenas se encontrar um objeto opaco.

================================================================================
COMPILAÇÃO
================================================================================

REQUISITOS:
- Windows 10/11
- CMake 3.16 ou superior
- Visual Studio 2022
- Projeto Ds compilado (biblioteca cg)

INSTRUÇÕES:

1. Certifique-se de que o projeto Ds está compilado. Se não estiver:
   - Abra o terminal na raiz do projeto Ds
   - Execute: cmake -S cg -B cg/build/vs2022
   - Abra cg/build/vs2022/cg.sln no Visual Studio 2022
   - Compile a solução (Build > Build Solution)

2. Para compilar o tp2:
   - Abra o terminal na raiz do projeto Ds
   - Execute: cmake -S apps/tp2 -B apps/tp2/build/vs2022
   - Abra apps/tp2/build/vs2022/tp2.sln no Visual Studio 2022
   - Compile a solução (Build > Build Solution)

   OU

   - Use o CMake GUI:
     * Source: C:/caminho/para/Ds/apps/tp2
     * Build: C:/caminho/para/Ds/apps/tp2/build/vs2022
     * Configure e Generate
     * Abra o .sln gerado no Visual Studio 2022

3. O executável será gerado em: apps/tp2/tp2.exe

================================================================================
EXECUÇÃO
================================================================================

1. Execute o programa tp2.exe a partir do diretório apps/tp2/

2. O programa abre uma janela com interface gráfica (ImGui)

3. Para carregar uma cena:
   - Menu: File > Open
   - Selecione uma das cenas em scenes/
   - Exemplos: scene1_basic_refraction.scn, scene2_nested_transparent.scn, etc.

4. Para renderizar:
   - Menu: View > Ray Tracer
   - A renderização será iniciada automaticamente

5. AJUSTAR PARÂMETROS (Menu: Ray Tracing):
   - Max Recursion Level: Profundidade máxima de recursão (0-20)
   - Min Weight: Peso mínimo dos raios secundários (0.001-1.0)
   - Adaptive Threshold: Limiar de variação para subdivisão (0.0-1.0)
     * Valores menores = mais subdivisões = melhor qualidade = mais lento
   - Max Subdiv Level: Níveis máximos de subdivisão (0-4)
     * 0 = sem antialiasing (1 raio por pixel)
     * 2-3 = recomendado para boa qualidade/performance
   - Use Jitter: Habilita amostragem aleatória (reduz padrões regulares)
   - Scene IOR: Índice de refração da cena (padrão: 1.0 para ar)

6. Para salvar a imagem renderizada:
   - A imagem é exibida na janela principal
   - Use a funcionalidade de screenshot do sistema ou capture da janela

NOTA: A renderização pode demorar dependendo da resolução, nível de 
antialiasing e complexidade da cena. Para testes rápidos, use resoluções 
menores (ex: 400x300) e maxSubdivLevel = 1 ou 2.

================================================================================
DESCRIÇÃO DAS CENAS
================================================================================

scene1_basic_refraction.scn
---------------------------
Objetivo: Testar refração básica
Conteúdo:
- Esfera opaca vermelha à esquerda
- Esfera transparente (vidro, IOR 1.5) no centro
- Esfera opaca azul à direita
- Plano de chão
- 2 luzes pontuais
Demonstra: Refração básica através de vidro, distorção de objetos atrás

scene2_nested_transparent.scn
------------------------------
OBRIGATÓRIA: Objetos transparentes dentro de objetos transparentes
Conteúdo:
- Esfera grande transparente (vidro, IOR 1.5)
- Esfera média transparente dentro (água/líquido, IOR 1.33)
- Esfera pequena opaca no centro (ouro)
- Plano de referência
- 3 luzes pontuais
Demonstra: Múltiplas refrações, objetos transparentes aninhados

scene3_light_inside_transparent.scn
-----------------------------------
OBRIGATÓRIA: Fonte de luz dentro de objeto transparente
Conteúdo:
- Esfera grande transparente (vidro) contendo uma luz pontual no centro
- Várias esferas opacas coloridas ao redor
- Plano de chão
- Luz adicional externa
Demonstra: Iluminação através de material transparente, caustics básicos

scene4_aliasing_test.scn
-------------------------
Objetivo: Testar antialiasing (bordas, padrões complexos)
Conteúdo:
- Grade de pequenas esferas coloridas
- Objetos pequenos e distantes da câmera
- Plano de chão
- 2 luzes pontuais
Demonstra: Redução de aliasing (serrilhado) com superamostragem adaptativa

scene5_creative.scn
-------------------
Objetivo: Demonstrar criatividade
Conteúdo:
- Escultura de vidro: esfera grande de vidro contendo esfera de água, 
  contendo esfera de diamante (IOR 2.4)
- Base dourada
- 4 gemas vermelhas ao redor (IOR 1.7)
- Plano de chão
- 3 luzes pontuais
Demonstra: Múltiplos materiais transparentes com diferentes IORs, 
refrações complexas, criatividade artística

================================================================================
IMAGENS GERADAS
================================================================================

As imagens devem ser geradas para comparação entre renderizações com e sem 
antialiasing. Para cada cena:

- Renderizar com Max Subdiv Level = 0 (sem antialiasing)
- Renderizar com Max Subdiv Level = 2 ou 3 (com antialiasing)
- Comparar resultados para mostrar redução de aliasing

As imagens devem ser salvas no diretório images/ (criar se não existir).

IMAGENS SUGERIDAS:
- scene1_no_aa.png / scene1_with_aa.png
- scene2_no_aa.png / scene2_with_aa.png
- scene3_no_aa.png / scene3_with_aa.png
- scene4_no_aa.png / scene4_with_aa.png
- scene5_no_aa.png / scene5_with_aa.png

NOTA: O programa atual não possui funcionalidade de salvar imagens 
automaticamente. Use screenshots ou capture da janela para gerar as imagens.

================================================================================
ESTRUTURA DO PROJETO
================================================================================

apps/tp2/
├── RayTracer.h          - Cabeçalho do ray tracer (estendido)
├── RayTracer.cpp        - Implementação das extensões
├── MainWindow.h          - Interface gráfica (estendida)
├── MainWindow.cpp        - Implementação da GUI
├── Main.cpp              - Ponto de entrada
├── CMakeLists.txt        - Configuração CMake
├── README.txt            - Este arquivo
├── scenes/               - Arquivos de cena (.scn)
│   ├── scene1_basic_refraction.scn
│   ├── scene2_nested_transparent.scn
│   ├── scene3_light_inside_transparent.scn
│   ├── scene4_aliasing_test.scn
│   ├── scene5_creative.scn
│   └── colors.inc        - Definições de cores
└── reader/               - Parser de cenas (do cgdemo)
    └── ...

================================================================================
NOTAS TÉCNICAS
================================================================================

ÍNDICES DE REFRAÇÃO TÍPICOS:
- Ar: 1.0
- Água: 1.33
- Vidro: 1.5-1.9
- Diamante: 2.4

VALORES RECOMENDADOS:
- Adaptive Threshold: 0.05-0.2 (menor = mais subdivisões)
- Max Subdiv Level: 2-3 (4 pode ser muito lento)
- Max Recursion Level: 6-10
- Min Weight: 0.01-0.1

PERFORMANCE:
- Antialiasing pode ser MUITO lento com maxSubdivLevel = 4
- Certifique-se de que a reutilização de raios está funcionando
- Para testes, use resoluções menores (400x300 ou 800x600)

================================================================================
FIM DO README
================================================================================

