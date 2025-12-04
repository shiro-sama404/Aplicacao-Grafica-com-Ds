=========================================================================
UFMS - Computação Gráfica 2025
PROVA 2 - PARTE PRÁTICA
=========================================================================

AUTORES:
- Arthur Barbosa
- Caio Mendes
- Marcus Augusto

DESCRIÇÃO DA SOLUÇÃO:

A solução para a Prova 2 foi desenvolvida e integrada diretamente na 
aplicação do Trabalho Prático 1 (TP1), localizada no diretório 'tp1_caio'.

Funcionalidades Implementadas:

1. Classe Box (Box.h / Box.cpp):
   - Criada uma classe derivada de 'Shape' representando uma Axis-Aligned 
     Bounding Box (AABB).
   - A interseção raio-caixa utiliza a lógica de 'Bounds3f' da biblioteca.
   - A visualização em OpenGL utiliza a malha auxiliar 'cg::Graphics3::box()'.

2. Aceleração BVH (RayCaster.h / RayCaster.cpp):
   - A classe 'Shape3' (e consequentemente 'PBRActor') fornece o método 
     'bounds()' retornando a AABB do objeto.
   - O 'RayCaster' constrói e utiliza uma BVH (Bounding Volume Hierarchy) 
     para acelerar os testes de interseção raio-cena.

3. Seleção de Atores (Ray Casting):
   - Implementado o método de seleção disparando um raio a partir da câmera 
     na direção do cursor do mouse.
   - O objeto mais próximo interceptado é destacado/selecionado na interface.

COMO TESTAR:
1. Compile e execute a aplicação (conforme instruções do TP1).
2. Na cena, verifique a presença dos objetos (incluindo a Caixa).
3. Clique com o botão esquerdo do mouse sobre qualquer ator na viewport 
   para testar a seleção via Ray Casting.

NOTA:
O código fonte completo encontra-se junto à submissão do TP1 (tp1.zip).