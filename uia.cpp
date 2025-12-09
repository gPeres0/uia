/**
 * UNIVERSIDADE ESTADUAL DE LONDRINA
 * Trabalho Final: Gato Dançante - FINAL EXPLOSIVO
 * * * RECURSOS NECESSÁRIOS (pasta sounds/):
 * - u.mp3, ii.mp3, a.mp3
 * - uiia.mp3 (Combo)
 * - fase2.mp3 (Musica Fase 2 e Eterno)
 * - explosao.wav [NOVO] (Efeito sonoro curto)
 * * * COMANDOS:
 * - 'u'+'i'+'a': Inicia Combo.
 * - Segurar 10s: Ativa MODO ETERNO.
 * - Aguardar 36s no Modo Eterno: EXPLOSÃO.
 * - 'r': Reiniciar aplicação.
 * * * RUN: g++ uia.cpp -o uia -lGL -lGLU -lglut -lSDL2 -lSDL2_mixer && ./uia
 */

#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

int windowW = 800;
int windowH = 600;

// Rotação atual do gato
GLfloat anguloGato = 45.0f;
// Estado da animação
bool animando = false;
// Estados das teclas
bool tecla_u = false;
bool tecla_i = false;
bool tecla_a = false;

// Variáveis de tempo e Estados especiais
Uint32 tempoInicioCombo = 0;    // Armazena o momento exato (ms) que o combo começou
bool fase2 = false;             // Ativa após 10s
bool modoEterno = false;        // Ativa após 10s
float velocidadeGiro = 15.0f;   // Velocidade de giro base
bool explodido = false;         // Indica se o gato morreu
bool somExplosaoTocado = false;
float fatorExplosao = 0.0f;     // Controla a distância das partes na explosão

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
Mix_Music* som_explosao = NULL;

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
    som_explosao = Mix_LoadMUS("sounds/explosao.mp3");
}

// Limpeza de recursos de áudio
void cleanup() {
    if (som_u) Mix_FreeMusic(som_u);
    if (som_i) Mix_FreeMusic(som_i);
    if (som_a) Mix_FreeMusic(som_a);
    if (som_uiia) Mix_FreeMusic(som_uiia);
    if (som_fase2) Mix_FreeMusic(som_fase2);
    if (som_explosao) Mix_FreeMusic(som_explosao);
    Mix_CloseAudio();
    SDL_Quit();
    printf("Recursos de audio liberados.\n");
}

// Função para reiniciar o programa
void resetarPrograma() {
    anguloGato = 45.0f;
    animando = false;
    indiceCor = 0;
    
    tempoInicioCombo = 0;
    fase2 = false;
    modoEterno = false;
    explodido = false;
    somExplosaoTocado = false;
    
    velocidadeGiro = 15.0f;
    anguloLevitacao = 0.0f;
    alturaLevitacao = 0.0f;
    fatorExplosao = 0.0f;
    
    estadoAtual = PARADO;
    Mix_HaltMusic();
    Mix_HaltChannel(-1); // Para todos os efeitos sonoros
    
    // Reseta iluminação
    for (int i = 0; i < 3; i++) {
        glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, luzAmbiente);
        glLightfv(GL_LIGHT0 + i, GL_SPECULAR, luzAmbiente);
    }
    
    glutPostRedisplay();
    printf("Programa Reiniciado.\n");
}

// Função de gerenciamento responsivo de áudio
void gerenciarAudio() {
    if (explodido) return;

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
    // Se explodiu, luzes ficam vermelhas piscando rápido ou escuras
    if (explodido) {
        // Efeito estroboscópico de erro
        GLfloat corErro[] = {1.0f, 0.0f, 0.0f, 1.0f};
        if (indiceCor % 2 == 0) corErro[0] = 0.0f; // Pisca
        for (int i = 0; i < 3; i++) {
            glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, corErro);
            glLightfv(GL_LIGHT0 + i, GL_SPECULAR, corErro);
        }
        return;
    }

    const GLfloat* corAtual = animando ? arcoIris[indiceCor] : luzAmbiente;
    for (int i = 0; i < 3; i++) {
        glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, corAtual);
        glLightfv(GL_LIGHT0 + i, GL_SPECULAR, corAtual);
    }
}

void desenharTextoCentro(const char* texto) {
    // Desabilita luzes e profundidade para o texto ficar branco e por cima de tudo
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    // Muda para projeção ortogonal (2D)
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowW, 0, windowH);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Cor Branca
    glColor3f(1.0f, 1.0f, 1.0f);

    // Calcula a largura do texto em pixels para centralizar
    int larguraTexto = 0;
    for (const char* c = texto; *c != '\0'; c++) {
        larguraTexto += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, *c);
    }

    // Posiciona no centro da tela
    int x = (windowW - larguraTexto) / 2;
    int y = windowH / 2;
    glRasterPos2i(x, y);

    // Desenha caractere por caractere
    for (const char* c = texto; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    // Restaura as matrizes anteriores (Volta para 3D)
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    // Reabilita o que foi desligado
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

// Modelagem hierárquica do gato
void desenharGato() {
    glPushMatrix();
        glTranslatef(0.0f, alturaLevitacao, 0.0f);
        if (explodido) {
            glRotatef(anguloGato, 1.0f, 1.0f, 1.0f);
        } else {
            glRotatef(anguloGato, 0.0f, 1.0f, 0.0f);
        }
        glPushMatrix(); // Corpo
            glScalef(0.8f, 0.8f, 1.0f);
            glutSolidSphere(1.0, 40, 40);
        glPopMatrix();
        glPushMatrix(); // Cabeça
            // Adiciona o fatorExplosao aos vetores de translação
            glTranslatef(0.0f, 0.8f + fatorExplosao, 0.8f + fatorExplosao);
            // Gira a cabeça aleatoriamente se explodindo
            if(explodido) glRotatef(anguloGato * 2, 1, 0, 0);
            glutSolidSphere(0.6, 40, 40);
            glPushMatrix(); // Orelha esq
                glTranslatef(-0.3f, 0.5f, 0.0f);
                glRotatef(30, 0.0f, 0.0f, 1.0f);
                glRotatef(-90, 1.0f, 0.0f, 0.0f);
                glutSolidCone(0.2, 0.2, 20, 20);
            glPopMatrix();
            glPushMatrix(); // Orelha dir
                glTranslatef(0.3f, 0.5f, 0.0f);
                glRotatef(-30, 0.0f, 0.0f, 1.0f);
                glRotatef(-90, 1.0f, 0.0f, 0.0f);
                glutSolidCone(0.2, 0.2, 20, 20);
            glPopMatrix();
            glPushMatrix(); // Olhos
                glTranslatef(-0.2f, 0.1f, 0.5f);
                glutSolidSphere(0.1, 10, 10);
            glPopMatrix();
            glPushMatrix();
                glTranslatef(0.2f, 0.1f, 0.5f);
                glutSolidSphere(0.1, 10, 10);
            glPopMatrix();
        glPopMatrix();
        // Pernas
        glPushMatrix(); // Frontal esq
            glTranslatef(-0.4f - fatorExplosao, -0.1f - fatorExplosao, 0.5f + fatorExplosao); 
            if(explodido) glRotatef(anguloGato * 3, 0, 1, 0);
            glRotatef(90, 1.0f, 0.0f, 0.0f);
            glutSolidCone(0.25, 1.0, 20, 20);
        glPopMatrix();
        glPushMatrix(); // Frontal dir
            glTranslatef(0.4f + fatorExplosao, -0.1f - fatorExplosao, 0.5f + fatorExplosao);
            if(explodido) glRotatef(anguloGato * 3, 0, 1, 0);
            glRotatef(90, 1.0f, 0.0f, 0.0f);
            glutSolidCone(0.25, 1.0, 20, 20);
        glPopMatrix();
        glPushMatrix(); // Traseira esq
            glTranslatef(-0.4f - fatorExplosao, -0.1f - fatorExplosao, -0.5f - fatorExplosao);
            if(explodido) glRotatef(anguloGato * 3, 0, 1, 0);
            glRotatef(90, 1.0f, 0.0f, 0.0f);
            glutSolidCone(0.25, 1.0, 20, 20);
        glPopMatrix();
        glPushMatrix(); // Traseira dir
            glTranslatef(0.4f + fatorExplosao, -0.1f - fatorExplosao, -0.5f - fatorExplosao);
            if(explodido) glRotatef(anguloGato * 3, 0, 1, 0);
            glRotatef(90, 1.0f, 0.0f, 0.0f);
            glutSolidCone(0.25, 1.0, 20, 20);
        glPopMatrix();
        // Rabo
        glPushMatrix();
            glTranslatef(0.0f, 0.3f + fatorExplosao, -0.8f - (fatorExplosao*1.5)); 
            if(explodido) glRotatef(anguloGato * 5, 0, 0, 1);
            glRotatef(250, 1.0f, 0.0f, 0.0f);
            glutSolidCone(0.1, 0.8, 10, 10);
        glPopMatrix();
    glPopMatrix();
}

void display() {
    // Fundo da tela
    if (explodido) {
        // Fundo vermelho escuro dramático
        glClearColor(0.2f, 0.0f, 0.0f, 1.0f);
    } else if (fase2 || modoEterno) {
        const GLfloat* corFundo = arcoIris[(indiceCor + 3) % 6];
        glClearColor(corFundo[0]*0.7, corFundo[1]*0.7, corFundo[2]*0.7, 1.0f);
    } else {
        // Fundo preto padrão
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    float distCamera = explodido ? 8.0f : 5.0f;
    gluLookAt(0.0, 2.0, 5.0,  
              0.0, 0.0, 0.0,  
              0.0, 1.0, 0.0);

    atualizarCorLuzes();
    desenharGato();

    if (explodido) desenharTextoCentro("PRESSIONE R PARA REINICIAR");     

    glutSwapBuffers();
}

void timer(int value) {
    // Lógica de explosão
    if (explodido) {
        // Se já explodiu, aumenta a distância dos pedaços infinitamente
        fatorExplosao += 0.2f; 
        anguloGato += 10.0f; // Gira a cena da explosão
        
        // Pisca luzes
        indiceCor++;
        if (indiceCor >= 6) indiceCor = 0;
        
        glutPostRedisplay();
        glutTimerFunc(1000/60, timer, 0);
        return; // Retorn para não processar o resto da lógica normal
    }

    bool comboAtivo = (tecla_u && tecla_i && tecla_a);
    // Lógica de contagem de tempo
    if (comboAtivo && !modoEterno) {
        if (tempoInicioCombo == 0) {
            tempoInicioCombo = SDL_GetTicks(); // Começa contador
        }
        
        Uint32 tempoDecorrido = SDL_GetTicks() - tempoInicioCombo;
        
        // 5 Segundos -> FASE 2
        if (tempoDecorrido > 4000) velocidadeGiro = 35.0f;

        // 10 Segundos -> MODO ETERNO
        if (tempoDecorrido > 8000) {
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

    if (modoEterno) {
        animando = true;

        Uint32 tempoTotal = SDL_GetTicks() - tempoInicioCombo;
        // 8000ms (ativar eterno) + 25000ms (musica) = 33000ms
        if (tempoTotal > 33000) {
            explodido = true;
            Mix_HaltMusic(); // Para a música
            if (!somExplosaoTocado) {
                Mix_PlayMusic(som_explosao, 0);
                somExplosaoTocado = true;
                printf("KABOOM! Pressione 'r' para reiniciar.\n");
            }
        }
    }

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
            if (!comboAtivo && !modoEterno) {
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
    
    if (key == 'r') { resetarPrograma(); return; }
    if (explodido) { if (key == 27) exit(0); return; }
    
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
    windowW = w;
    windowH = h;
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
    glutInitWindowSize(windowW, windowH);
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