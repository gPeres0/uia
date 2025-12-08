/**
 * UNIVERSIDADE ESTADUAL DE LONDRINA
 * Trabalho Final de Computação Gráfica
 * RUN: g++ uia.cpp -o uia -lGL -lGLU -lglut -lSDL2 -lSDL2_mixer && ./uia
 */

#include <GL/glut.h>
#include <math.h>
#include <stdio.h>

// [NOVO] Bibliotecas SDL para áudio
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

// --- Variáveis Globais de Controle ---
GLfloat anguloGato = 45.0f;       // Rotação atual do gato
bool animando = false;           // Estado da animação
int  indiceCor = 0;              // Índice para cores do arco-íris

// Estados das teclas (para detectar combos)
bool tecla_u = false;
bool tecla_i = false;
bool tecla_a = false;

// [NOVO] Ponteiros para os sons
Mix_Chunk* som_u = NULL;
Mix_Chunk* som_i = NULL;
Mix_Chunk* som_a = NULL;

// Cores do Arco-íris (RGB) para as luzes
const GLfloat arcoIris[6][4] = {
    {1.0f, 0.0f, 0.0f, 1.0f}, // Vermelho
    {1.0f, 0.5f, 0.0f, 1.0f}, // Laranja
    {1.0f, 1.0f, 0.0f, 1.0f}, // Amarelo
    {0.0f, 1.0f, 0.0f, 1.0f}, // Verde
    {0.0f, 0.0f, 1.0f, 1.0f}, // Azul
    {0.5f, 0.0f, 1.0f, 1.0f}  // Roxo
};
const GLfloat luzAmbiente[] = {0.6f, 0.6f, 0.6f, 1.0f};

// [NOVO] Inicialização do SDL Audio
void initAudio() {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        printf("Erro ao inicializar SDL: %s\n", SDL_GetError());
        exit(1);
    }
    // Inicializa Mixer: Frequência 44100Hz, Formato padrão, 2 canais (stereo), chunksize 2048
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Erro ao inicializar SDL_mixer: %s\n", Mix_GetError());
        exit(1);
    }

    // Carrega os arquivos WAV
    som_u = Mix_LoadWAV("u.wav");
    som_i = Mix_LoadWAV("i.wav");
    som_a = Mix_LoadWAV("a.wav");

    if (!som_u || !som_i || !som_a) {
        printf("Aviso: Um ou mais arquivos de som (u.wav, i.wav, a.wav) nao foram encontrados!\n");
    }
}

// [NOVO] Limpeza de recursos ao sair
void cleanup() {
    if (som_u) Mix_FreeChunk(som_u);
    if (som_i) Mix_FreeChunk(som_i);
    if (som_a) Mix_FreeChunk(som_a);
    Mix_CloseAudio();
    SDL_Quit();
    printf("Recursos de audio liberados.\n");
}

// --- Configuração de Iluminação ---
void configurarLuzes() {
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);

    GLfloat pos0[] = {-5.0f, 5.0f, 5.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, pos0);
    glEnable(GL_LIGHT0);

    GLfloat pos1[] = {0.0f, 5.0f, -5.0f, 1.0f};
    glLightfv(GL_LIGHT1, GL_POSITION, pos1);
    glEnable(GL_LIGHT1);

    GLfloat pos2[] = {5.0f, 5.0f, 5.0f, 1.0f};
    glLightfv(GL_LIGHT2, GL_POSITION, pos2);
    glEnable(GL_LIGHT2);

    GLfloat mat_ambient[] = {0.6f, 0.6f, 0.6f, 1.0f};
    GLfloat mat_diffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat mat_specular[] = {0.6f, 0.5f, 0.6f, 1.0f};
    GLfloat mat_shininess[] = {100.0f};

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
}

void atualizarCorLuzes() {
    const GLfloat* corAtual;
    if (animando) {
        corAtual = arcoIris[indiceCor];
    } else {
        corAtual = luzAmbiente;
    }
    for (int i = 0; i < 3; i++) {
        glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, corAtual);
        glLightfv(GL_LIGHT0 + i, GL_SPECULAR, corAtual);
    }
}

// --- Modelagem Hierárquica do Gato ---
void desenharGato() {
    glPushMatrix();
        glRotatef(anguloGato, 0.0f, 1.0f, 0.0f);

        // 1. Corpo
        glPushMatrix();
            glScalef(0.8f, 0.8f, 1.0f);
            glutSolidSphere(1.0, 40, 40);
        glPopMatrix();
    
        // 2. Cabeça
        glPushMatrix();
            glTranslatef(0.0f, 0.8f, 0.8f);
            glutSolidSphere(0.6, 40, 40);

            // 2.1 Orelha Esquerda
            glPushMatrix();
                glTranslatef(-0.3f, 0.5f, 0.0f);
                glRotatef(30, 0.0f, 0.0f, 1.0f);
                glRotatef(-90, 1.0f, 0.0f, 0.0f);
                glutSolidCone(0.2, 0.2, 20, 20);
            glPopMatrix();

            // 2.2 Orelha Direita
            glPushMatrix();
                glTranslatef(0.3f, 0.5f, 0.0f);
                glRotatef(-30, 0.0f, 0.0f, 1.0f);
                glRotatef(-90, 1.0f, 0.0f, 0.0f);
                glutSolidCone(0.2, 0.2, 20, 20);
            glPopMatrix();
            
            // 2.3 Olhos
            glPushMatrix();
                glTranslatef(-0.2f, 0.1f, 0.5f);
                glutSolidSphere(0.1, 10, 10);
            glPopMatrix();
            glPushMatrix();
                glTranslatef(0.2f, 0.1f, 0.5f);
                glutSolidSphere(0.1, 10, 10);
            glPopMatrix();
        glPopMatrix();

        // 3. Pernas
        glPushMatrix(); // Frontal Esq
            glTranslatef(-0.4f, -0.1f, 0.5f);
            glRotatef(90, 1.0f, 0.0f, 0.0f);
            glutSolidCone(0.25, 1.0, 20, 20);
        glPopMatrix();

        glPushMatrix(); // Frontal Dir
            glTranslatef(0.4f, -0.1f, 0.5f);
            glRotatef(90, 1.0f, 0.0f, 0.0f);
            glutSolidCone(0.25, 1.0, 20, 20);
        glPopMatrix();
        
        glPushMatrix(); // Traseira Esq
            glTranslatef(-0.4f, -0.1f, -0.5f);
            glRotatef(90, 1.0f, 0.0f, 0.0f);
            glutSolidCone(0.25, 1.0, 20, 20);
        glPopMatrix();

        glPushMatrix(); // Traseira Dir
            glTranslatef(0.4f, -0.1f, -0.5f);
            glRotatef(90, 1.0f, 0.0f, 0.0f);
            glutSolidCone(0.25, 1.0, 20, 20);
        glPopMatrix();
        
        // 4. Cauda
        glPushMatrix();
            glTranslatef(0.0f, 0.3f, -0.8f);
            glRotatef(250, 1.0f, 0.0f, 0.0f);
            glutSolidCone(0.1, 0.8, 10, 10);
        glPopMatrix();

    glPopMatrix();
}

void display() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    gluLookAt(0.0, 2.0, 5.0,  
              0.0, 0.0, 0.0,  
              0.0, 1.0, 0.0);

    atualizarCorLuzes();
    desenharGato();
    glutSwapBuffers();
}

void timer(int value) {
    if (animando) {
        anguloGato += 15.0f; 
        indiceCor++;
        if (indiceCor >= 6) indiceCor = 0;

        if (anguloGato >= 365.0f) {
            anguloGato = 45.0f; 
            if (!(tecla_u && tecla_i && tecla_a)) {
                animando = false;
                atualizarCorLuzes(); 
            }
        }
    }
    glutPostRedisplay();
    glutTimerFunc(1000/60, timer, 0);
}

// --- Controle de Teclado ---
void keyboard(unsigned char key, int x, int y) {
    if (key >= 'A' && key <= 'Z') key += 32;

    // [NOVO] Tocar sons ao detectar a tecla
    if (key == 'u') {
        tecla_u = true;
        if(som_u) Mix_PlayChannel(-1, som_u, 0); // Canal -1 (auto), loops 0
    }
    if (key == 'i') {
        tecla_i = true;
        if(som_i) Mix_PlayChannel(-1, som_i, 0);
    }
    if (key == 'a') {
        tecla_a = true;
        if(som_a) Mix_PlayChannel(-1, som_a, 0);
    }

    if (key == 'u' || key == 'i' || key == 'a') {
        if (!animando) {
            animando = true;
            anguloGato = 45.0f;
        }
    }
    
    if (key == 27) exit(0); // A função cleanup será chamada automaticamente pelo atexit
}

void keyboardUp(unsigned char key, int x, int y) {
    if (key >= 'A' && key <= 'Z') key += 32;

    if (key == 'u') tecla_u = false;
    if (key == 'i') tecla_i = false;
    if (key == 'a') tecla_a = false;
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90.0, (float)w / h, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    // [NOVO] Configurar limpeza automática ao sair
    atexit(cleanup);

    glutInit(&argc, argv);
    
    // [NOVO] Inicializa Áudio antes de criar a janela GLUT
    initAudio();

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Projeto Final: Gato Dancante 3D (Com Som)");

    configurarLuzes();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}