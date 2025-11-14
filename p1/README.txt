PARTE PRÁTICA P1 - COMPUTAÇÃO GRÁFICA

AUTORES:
Arthur Barbosa
Caio Mendes
Marcus Madureira

Este projeto implementa um Ray Caster 3D que renderiza cenas usando a técnica 
de ray casting com iluminação direta (modelo de Phong).
O sistema lança um raio por pixel da câmera e calcula a cor baseada na 
intersecção com os objetos da cena.

Implementação:
- Formas geométricas: Esfera e Plano definidos em espaço local
- Transformações TRS para posicionar/escalar/rotacionar objetos no espaço global
- Intersecção raio-objeto no espaço local com transformação de normais correta
- Iluminação direta com modelo de Phong
- Suporte a múltiplas luzes pontuais

Cena de teste:
- 1 plano como chão
- 3 esferas da cor vermelha com materiais diferentes
- 3 luzes pontuais com cores diferentes (vermelho, azul e verde)
OBS:Para visualizar a cena corretamente com o ray casting, pode ser necessário 
afastar a câmera usando os controles, pois ela inicia próxima dos objetos.	


COMPILAÇÃO:

O projeto usa CMake e a biblioteca CG do professor Paulo Pagliosa.

Passos para compilar:

- Coloque o projeto dentro da pasta templates do projeto Ds, devido às 
  configurações do CMake. Caso contrário, o projeto não será compilado corretamente.

1. Abra o terminal na pasta do projeto (apps/p1)

2. Crie a pasta build:
   mkdir build
   cd build

3. Configure o projeto com CMake:
   Windows (Visual Studio 2022):
     cmake .. -G "Visual Studio 17 2022" -A x64

4. Compile:
   Windows:
     cmake --build . --config Debug

5. Execute:
   Windows: O executável estará em Debug\p1.exe


CONTROLES

Camera:
- WASD: mover
- Q/Z: subir/descer
- Scroll: zoom
- Alt+Right-click: rotacionar

Ray Caster:
- Alt+P: ativar/desativar ray casting
- Botão "Re-render": renderizar novamente da perspectiva atual
