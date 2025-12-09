/**
 * UNIVERSIDADE ESTADUAL DE LONDRINA
 * Trabalho Final de Computação Gráfica
 * RUN: g++ uia.cpp -o uia -lGL -lGLU -lglut -lSDL2 -lSDL2_mixer && ./uia
 */

#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

// Rotação atual do gato
GLfloat anguloGato = 45.0f;
// Estado da animação
bool animando = false;
// Estados das teclas
bool tecla_u = false;
bool tecla_i = false;
bool tecla_a = false;

// Variáveis de Tempo e Estados Especiais [NOVO]
Uint32 tempoInicioCombo = 0;    // Armazena o momento exato (ms) que o combo começou
bool fase2 = false;             // Ativa após 10s
bool modoEterno = false;        // Ativa após 10s
float velocidadeGiro = 15.0f;   // Velocidade de giro base

// Variáveis de levitação
float anguloLevitacao = 0.0f; // O "relógio" da onda senoidal
float alturaLevitacao = 0.0f; // A altura Y calculada
float velocidadeLevitacao = 0.0f; // Quão rápido ele sobe e desce

// Cores das luzes
const GLfloat arcoIris[6][4] = {
    {1.0f, 0.0f, 0.0f, 1.0f}, // Vermelho
    {1.0f, 0.5f, 0.0f, 1.0f}, // Laranja
    {1.0f, 1.0f, 0.0f, 1.0f}, // Amarelo
    {0.0f, 1.0f, 0.0f, 1.0f}, // Verde
    {0.0f, 0.0f, 1.0f, 1.0f}, // Azul
    {0.5f, 0.0f, 1.0f, 1.0f}  // Roxo
};
int  indiceCor = 0;
const GLfloat luzAmbiente[] = {0.6f, 0.6f, 0.6f, 1.0f};

// Ponteiros para os sons
Mix_Music* som_u = NULL;
Mix_Music* som_i = NULL;
Mix_Music* som_a = NULL;
Mix_Music* som_uiia = NULL;
Mix_Music* som_fase2 = NULL;

// Enum para rastrear qual som está tocando
enum EstadoSom { PARADO, TOCANDO_U, TOCANDO_I, TOCANDO_A, TOCANDO_UIIA, TOCANDO_FASE2 };
EstadoSom estadoAtual = PARADO;


// Inicialização do SDL Audio
void initAudio() {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) exit(1);
    if (Mix_Init(MIX_INIT_MP3) == 0) printf("Erro Mix_Init: %s\n", Mix_GetError());
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) exit(1);

    som_u = Mix_LoadMUS("sounds/u.mp3");
    som_i = Mix_LoadMUS("sounds/ii.mp3");
    som_a = Mix_LoadMUS("sounds/a.mp3");
    som_uiia = Mix_LoadMUS("sounds/uiia.mp3");
    som_fase2 = Mix_LoadMUS("sounds/fase2.mp3");
}

// Limpeza de recursos de áudio
void cleanup() {
    if (som_u) Mix_FreeMusic(som_u);
    if (som_i) Mix_FreeMusic(som_i);
    if (som_a) Mix_FreeMusic(som_a);
    if (som_uiia) Mix_FreeMusic(som_uiia);
    if (som_fase2) Mix_FreeMusic(som_fase2);
    Mix_CloseAudio();
    SDL_Quit();
    printf("Recursos de audio liberados.\n");
}

// Função de gerenciamento responsivo de áudio
void gerenciarAudio() {
    // Se o MODO ETERNO estiver ligado, toca Fase 2 para sempre
    if (modoEterno) {
        if (estadoAtual != TOCANDO_FASE2) {
            Mix_PlayMusic(som_fase2, -1);
            estadoAtual = TOCANDO_FASE2;
        }
        return; // Sai da função, ignorando teclas soltas
    }

    // Maior prioridade (U + I + A)
    if (tecla_u && tecla_i && tecla_a) {
        // Só toca se já não estiver tocando
        if (estadoAtual != TOCANDO_UIIA) {
            Mix_PlayMusic(som_uiia, -1); // -1 = Loop Infinito
            estadoAtual = TOCANDO_UIIA;
        }
    }
    // Prioridades arbitrárias
    else if (tecla_u) {
        if (estadoAtual != TOCANDO_U) {
            Mix_PlayMusic(som_u, 0);
            estadoAtual = TOCANDO_U;
        }
    } else if (tecla_i) {
        if (estadoAtual != TOCANDO_I) {
            Mix_PlayMusic(som_i, 0);
            estadoAtual = TOCANDO_I;
        }
    } else if (tecla_a) {
        if (estadoAtual != TOCANDO_A) {
            Mix_PlayMusic(som_a, 0);
            estadoAtual = TOCANDO_A;
        }
    }
    // Nenhuma tecla relevante pressionada
    else {
        if (estadoAtual != PARADO) {
            if (estadoAtual == TOCANDO_UIIA) Mix_HaltMusic();
            estadoAtual = PARADO;
        }
    }
}

// Configurações de iluminação
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

// Modelagem hierárquica do gato
void desenharGato() {
    glPushMatrix();
        glTranslatef(0.0f, alturaLevitacao, 0.0f);
        glRotatef(anguloGato, 0.0f, 1.0f, 0.0f);
        // Corpo
        glPushMatrix();
            glScalef(0.8f, 0.8f, 1.0f);
            glutSolidSphere(1.0, 40, 40);
        glPopMatrix();
        // Cabeça
        glPushMatrix();
            glTranslatef(0.0f, 0.8f, 0.8f);
            glutSolidSphere(0.6, 40, 40);
            // Orelha Esquerda
            glPushMatrix();
                glTranslatef(-0.3f, 0.5f, 0.0f);
                glRotatef(30, 0.0f, 0.0f, 1.0f);
                glRotatef(-90, 1.0f, 0.0f, 0.0f);
                glutSolidCone(0.2, 0.2, 20, 20);
            glPopMatrix();
            // Orelha Direita
            glPushMatrix();
                glTranslatef(0.3f, 0.5f, 0.0f);
                glRotatef(-30, 0.0f, 0.0f, 1.0f);
                glRotatef(-90, 1.0f, 0.0f, 0.0f);
                glutSolidCone(0.2, 0.2, 20, 20);
            glPopMatrix();
            // Olhos
            glPushMatrix();
                glTranslatef(-0.2f, 0.1f, 0.5f);
                glutSolidSphere(0.1, 10, 10);
            glPopMatrix();
            glPushMatrix();
                glTranslatef(0.2f, 0.1f, 0.5f);
                glutSolidSphere(0.1, 10, 10);
            glPopMatrix();
        glPopMatrix();
        // Pernas
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
        // Rabo
        glPushMatrix();
            glTranslatef(0.0f, 0.3f, -0.8f);
            glRotatef(250, 1.0f, 0.0f, 0.0f);
            glutSolidCone(0.1, 0.8, 10, 10);
        glPopMatrix();
    glPopMatrix();
}

void display() {
    // Fundo da tela
    if (fase2 || modoEterno) {
        const GLfloat* corFundo = arcoIris[(indiceCor + 3) % 6];
        glClearColor(corFundo[0]*0.7, corFundo[1]*0.7, corFundo[2]*0.7, 1.0f);
    } else {
        // Fundo preto padrão
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    }

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
    bool comboAtivo = (tecla_u && tecla_i && tecla_a);
    // Lógica de contagem de tempo
    if (comboAtivo && !modoEterno) {
        if (tempoInicioCombo == 0) {
            tempoInicioCombo = SDL_GetTicks(); // Começa contador
        }
        
        Uint32 tempoDecorrido = SDL_GetTicks() - tempoInicioCombo;
        
        // 5 Segundos -> FASE 2
        if (tempoDecorrido > 5000) {
            velocidadeGiro = 35.0f;
        }

        // 10 Segundos -> MODO ETERNO
        if (tempoDecorrido > 10000) {
            fase2 = true;
            modoEterno = true;
            velocidadeGiro = 55.0f;
            printf("MODO ETERNO ATIVADO! (Esc para sair)\n");
        }
    } else {
        // Se soltar as teclas ANTES de virar eterno, reseta
        if (!modoEterno) {
            tempoInicioCombo = 0;
            fase2 = false;
            velocidadeGiro = 15.0f;
        }
    }

    if (modoEterno) animando = true;

    // A levitação só acontece se for Combo, Fase 2 ou Modo Eterno
    if (comboAtivo || modoEterno) {
        // Define a velocidade da onda senoidal
        if (modoEterno) {
            velocidadeLevitacao = 0.5f; // Muito Rápido
        } else if (fase2) {
            velocidadeLevitacao = 0.2f; // Rápido
        } else {
            velocidadeLevitacao = 0.1f; // Lento (Normal)
        }
        
        // Atualiza o ângulo e calcula a altura Y
        anguloLevitacao += velocidadeLevitacao;
        // sin(angulo) varia de -1 a 1. Multiplicamos por 0.3 para não pular alto demais.
        alturaLevitacao = sin(anguloLevitacao) * 0.3f; 
    } else {
        // Se soltar as teclas (e não for eterno), volta suavemente pro chão
        if (fabs(alturaLevitacao) > 0.01f) {
            alturaLevitacao *= 0.8f; // Amortecimento suave até 0
        } else {
            alturaLevitacao = 0.0f;
            anguloLevitacao = 0.0f;
        }
    }
    
    gerenciarAudio();

    if (animando) {
        anguloGato += velocidadeGiro;
        indiceCor++;
        if (indiceCor >= 6) indiceCor = 0;

        if (anguloGato >= 365.0f) {
            anguloGato = 45.0f;
            if (!(tecla_u && tecla_i && tecla_a && modoEterno)) {
                animando = false;
                atualizarCorLuzes(); 
            }
        }
    
    }
    glutPostRedisplay();
    glutTimerFunc(1000/60, timer, 0);
}

// Controles de teclado
void keyboard(unsigned char key, int x, int y) {
    if (key >= 'A' && key <= 'Z') key += 32;

    if (key == 'u') tecla_u = true;
    if (key == 'i') tecla_i = true;
    if (key == 'a') tecla_a = true;
    
    if (key == 'u' || key == 'i' || key == 'a') {
        if (!animando) {
            animando = true;
            anguloGato = 45.0f;
        }
    }

    if (key == 27) exit(0); // Chama função cleanup automaticamente pelo atexit
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
    //Limpeza automática ao sair
    atexit(cleanup);
    glutInit(&argc, argv);
    // Inicializa áudio antes de criar a janela GLUT
    initAudio();

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("UIIA");

    configurarLuzes();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}