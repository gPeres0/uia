# UIIA CAT
#### Projeto Final da disciplina de Computação Gráfica

### DESCRIÇÃO
O projeto se propõe a recriar, em OpenGL + GLUT, o meme UIIA, que mostra a imagem de um gato girando ao som de uma música eletrônica dizendo, repetidas vezes, as letras "U", "I" e "A".

### CAPTURAS DE TELA
![alt text](<images/Captura de tela de 2025-12-09 12-58-41.png>)
*Figura 1: Estado inicial do programa, mostrando um desenho 3D de um gato cinza-claro.*

![alt text](<images/Captura de tela de 2025-12-09 12-59-01.png>) 
*Figura 2: Desenho do gato realizando uma rotação enquanto muda-se a cor da iluminação, realizada após pressionar a tecla 'u'.*

![alt text](<images/Captura de tela de 2025-12-09 12-59-12.png>)
*Figura 3: Continuação da rotação do gato, dessa vez uma luz de cor amarela.*

![alt text](<images/Captura de tela de 2025-12-09 12-59-29.png>)
*Figura 4: Rotação realziada ao pressionar a tecla 'a', dessa vez com a cor da iluminação azul.*

![alt text](<images/Captura de tela de 2025-12-09 12-59-39.png>)
*Figura 5: Frame da rotação do gato durante a "Fase 2", onde a velocidade de rotação e levitação aumentam e o fundo da tela brilha em diferentes cores, juntamente com o gato.*

![alt text](<images/Captura de tela de 2025-12-09 13-00-03.png>)
*Figura 6: Tela de reinicialização do programa. Ocorre depois do "Modo Eterno", onde o gato gira cada vez mais rápido até explodir.*

### DEPENDÊNCIAS E COMPILAÇÃO
##### DEPENDÊNCIAS E BIBLIOTECAS:
| Biblioteca | Pacote (Ubuntu/Debian) | Função                 |  
| ---------- | ---------------------- | ---------------------- |
| G++        | build-essential        | Compilador C++ padrão   
| OpenGL     | mesa-common-dev        | Biblioteca gráfica base
| GLU        |libglu1-mesa-dev        | Utilitários OpenGL (câmera, esferas, cones)
| GLUT/FreeGLUT | freeglut3-dev       | Gerenciamento de janelas e inputs
| SDL2 Core  | libsdl2-dev            | Sistema base de áudio e multimidia.
| SDL2 Mixer | libsdl2-mixer-dev      | Extensão para tocar MP3 e mixar canais.

**Instalação SDL2:** 
``` bash
sudo apt-get update
sudo apt-get install build-essential freeglut3-dev libsdl2-dev libsdl2-mixer-dev
```
##### COMPILAÇÃO E EXECUÇÃO:
``` bash
g++ uia.cpp -o uia -lGL -lGLU -lglut -lSDL2 -lSDL2_mixer -lm
./uia
```

### FEATURES E COMANDOS
##### COMANDOS DE TECLADO:
- Pressionar teclas 'u', 'i' ou 'a': Realiza uma rotação do gato com resposta sonora dependendo da tecla pressionada.
- Segurar teclas 'u', 'i' e 'a' simultaneamente: Ativa modo UIIA. O gato gira e toca a música completa enquanto as teclas continuarem pressionadas.
- Pressionar tecla 'r': Reinicia completamente o programa.
#### FEATURES:
- Segurar 'u'+'i'+'a' por 4 segundos: Entra na FASE 2. Aqui, os movimentos do gato ficam mais rápidos.
- Segurar 'u'+'i'+'a' por 8 segundos: Entra no modo ETERNO. Os movimentos do gato ficam ainda mais rápidos e o fundo da tela começa a piscar junto com o gato. Uma música eletrônica frenética começa a tocar. Além disso, o usuário não precisa mais segurar as teclas nesse modo.
- Fim do modo ETERNO: Ao final do modo ETERNO, o gato explode com um som de explosão e um letreiro dizendo "PRESSIONE R PARA REINICIAR APARECE".