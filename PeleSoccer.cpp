#define TERMINALLINHAS  100
#define TERMINALCOLUNAS  64 //Tamanho do terminal
#define ESCALA 8 //Escala da tela em relacao ao terminal
#define NUMVAO 1 //Numero de vaos na lista de vao
#define CAMPODEVISAO 50//Campo de visao do jogador
#define TAMANHODOTIME 4//Tamanho maximo de cada time
#define FRAMERATE 30 //Define FPS
 
#include <glad/gl3w.h> //Versão minima e atualizada do GLEW
#include <GLFW/glfw3.h> //GLFW para renderizar a janela
#include <compilaShader.h> //Compila e linka programas de shaders utilizados
#include <Windows.h>
#include <string.h>
#include <time.h>


//Bibliotecas para operacoes com matrizes
#include <glm\glm.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//LISTA DE COISAS A FAZER:
//Basico:
//--
//1º:Estrutura pra representar e inicializar jogadores e bola:100% 
//2ºFuncoes para movimentacao de jogadores e time:100%
//3º:Interacao entre jogadores e a bola: 100%
//4º:Implementar 30 fps: 100%
//5º:Verificar condicoes de pontuacao e vitoria: 100%
//6º:Implemetnar goleiro:100%
//7º:Leitura de arquivos com formacoes de jogadores
//8º:Implementar controle do segundo time:100%
//9º:Implementar tabela de high scores
//--
//Extra:
//8º:Texturas
//9º:Som
//10º:Rescrever a funcão de desenho do jogador para permitir 3 angulos de chute(jogador "3x3" em 1 bloco)
//11º:Customização dos jogadores(chute mais longo/mais rapido etc): 80%
//12:º:Oponente controlado por computador
//Provavelmente nunca:
//??:Otimizar eficiencia do loop de desenho(não tem pq desenhar todos os 0s)
//??:Implementar 2d isometrico

#pragma region consts, variaveis e structs globais

//Proporcao da do campo de visao em relacao a tela toda
const float PROPORCAO = TERMINALLINHAS / (float)CAMPODEVISAO;
const float PASSOX = 0.03125f; //tamanho minimo de um passo X no plano 
const float PASSOY = 0.02f*PROPORCAO;//Tamanho minimo de um passo no Y do plano

//Matriz que representa o estado inicial da tela, para guardar os elementos fixos.
int vetorDeDadosInicial[TERMINALCOLUNAS][TERMINALLINHAS];
//Matriz que representa a tela
int vetorDeDadosRecebido[TERMINALCOLUNAS][TERMINALLINHAS];
//Indica se um vetor foi recem desenhado ou não
bool vetorDeDadosDesenhado[TERMINALCOLUNAS][TERMINALLINHAS];
//Matriz com os dados do campo de visão atual
int vetorDoCampoDeVisao[TERMINALCOLUNAS][CAMPODEVISAO];
//Vetor que representa a posicao X,Y da bola na matriz
int posBola[2];

//Placar do jogo, time1 eh [0], time2 eh [1]
int placar[2];

//Conta numero de frames até o momento
int contadordeframes = 0;

//Guarda quais teclas de movimento foram pressionadas pra movimentacao em diagonal
//0 direita(X+), 1 esquerda(X-), 2 BAIXO(Y+), 3 cima(Y-);
bool vetorDeSetas[4] = { 0,0,0,0 };
bool vetorDeSetas2[4] = { 0,0,0,0 };


//Limites do campo de visão
int inicioDoCampoDeVisao=30, fimDoCampoDeVisao=inicioDoCampoDeVisao+CAMPODEVISAO;

//Serve para regurlar a velocidade da atualização da bola em relacao aos jogadores
int limitadorDeVelocidadeBola;

//Matriz identidade que não modifica o vertice
const glm::mat4 matrixIdentidade;

//Quadrado basico que representa o tamanho de um caractere no terminal
typedef struct quadrado
{
	GLfloat posx, posy; // Define posx, e posy para guardar a posição do item na tela localmente
	GLfloat vertices[12];//Cria array de vertices
	unsigned int indices[6] = {  //Indices para definir como desenhar o quadrado nos vertices indicados
		0, 1, 3,   // Triangulo 1
		1, 2, 3    // Triangulo 2
	};
} quadrado;

//Estrutura que representa um jogador
typedef struct jogador 
{
	//Posicao x
	int x;
	
	//Posicao y
	int y;
	
	//1:azul, 2:Vermelho
	int time;

	//Posicao inicialX do jogador
	int posInicialX;

	//Posicao InicialY do jogador
	int posInicialY;
} jogador;

//Estrutura que representa uma bola
typedef struct bola
{
	int x;
	int y;
	int velX;
	int velY;
} bola;
#pragma endregion

#pragma region Prototipos

void atualizaGoleiro(jogador goleiroRecebido,jogador *goleiroRetornado);


//
//Retorna um time para sua posicao inicial
void limpaTime(jogador timeRecebido[TAMANHODOTIME]);

//
//Inicializa uma VAO só com EBO, precisa de info de atribs da vbo
GLint inicializaVAOVazia(unsigned int recebeEBO);

//
//inicializaQuadrado:(GLfloat x, GLfloat ) -> quadrado Quadrado
//Recebe um X e um Y e retorna uma estrutura quadrado com um array de vertices naquela posição
quadrado inicializaQuadrado(GLfloat x, GLfloat y);


//Função para receber as teclas pressionadas pelo usuario por callback
//Recebe um endereço na memoria de uma estrutura do tipo GLFW, e faz alguma coisa dependendo do estado atual de
//Alguma variavel definida nessa janela 
void recebeEntrada(GLFWwindow *window, int key, int scancode, int action, int mods);

//
//Prototipo da funcao de callback para redimensionar a janela
void framebuffer_size_callback(GLFWwindow* window, int width, int height);


//
//Posiciona um jogador especifico na posicao dada e retorna seu estado atualizado
void posicionaJogador(int x, int y, jogador jogadorRecebido, jogador *retorno);

//
//Inicializa um jogador
jogador inicializaJogador(int x, int y, int Time);

//
//Inicializa uma bola
bola inicializaBola(int x, int y,int velX,int velY);

//
//Cria uma matriz altura CAMPODEVISAO e largura do campo, com base na matriz inteira, e atribui no endereo recebido
//Iniciando no 0,0 da tela, com CAMPODEVISAO/2 para cada lado da tela
//Retorno precisa ser uma matriz[64][CAMPODEVISAO] obrigatoriamente
void atuallizaMatrizCampoDeVisao(int matrizOriginal[TERMINALCOLUNAS][TERMINALLINHAS], int enderecoDoRetorno[TERMINALCOLUNAS][CAMPODEVISAO]);

//
//Prototipo da função que tranforma tamanos em unidades de terminal(64x100) para unidades normalizadas.
void normaliza(float originalX, float originalY, GLfloat *retorno);

//
//desenhaJogador:Desenha um quadrado com a cor especificada
void desenhaJogador(Shader shaderUsado, int VAO, GLfloat x, GLfloat y, float cor[3]);

//
//DesenhaPeloCodigo: recebe um int e desenha algo dependo do int recebido usando alguma outra funcao de desenho
void desenhaPeloCodigo(int codigo, Shader shaderUsado, int listaVAOs[NUMVAO], GLfloat x, GLfloat y);

//
//Recebe um valor em coordenadas normalizadas e retorna a posicao na matriz
void deNormaliza(float originalX, float originalY, GLfloat *retorno);

//
//Cria o estado padrao da tela na matriz
void zeraTela(int vetorDaTela[TERMINALCOLUNAS][TERMINALLINHAS]);

//
//Desenha um quadrado branco
void desenhaEmBranco(Shader shaderUsado, int VAO, GLfloat x, GLfloat y);

//
//Desenha um quadrado cinza
void desenhaEspacoGoleiro(Shader shaderUsado, int VAO, GLfloat x, GLfloat y);

//
//Desenha um quadrado branco-acinzentado
void desenhaFaixaDoCampo(Shader shaderUsado, int VAO, GLfloat x, GLfloat y);

//
//Desenha um quadrado com a cor de fundo
void desenhaVazio(Shader shaderUsado, int VAO, GLfloat x, GLfloat y);

//
//Altera a posicao da bola para as cordenadas dadas
void posicionaBola(bola *bolaRecebida,int x, int y);

//
//Move um time inteiro uma distancia X,Y especificada
int moveTime(int distanciaX, int distanciaY, jogador timeRecebido[TAMANHODOTIME], jogador *retorno,int fixador[8],bola *bolaRecebida, bool vetorDeSetas[4]);


//Mede os limites X e Y do time dado para fixar a formacao especificada. retorna uma lista de valores
//encodada em decimal da seguinte forma: [ipppipppipppippp] onde i é o id(0 a 9) e p a posicao(0 a 999) dos valore
//maxX,MinX,maxY,minY. Lembrando que qualquer valor pode então ser obtido a partidir de uma operação de modulo.
void fixaTime(jogador timeRecebido[TAMANHODOTIME],int *retorno);

//
//Atualiza a posição da bola no loop do jogo a cada frame. Verifica colisoes e gerencia a velocidade da bola
int atualizaBola(bola *bolaRecebida);
#pragma endregion

int main()
{
	
	#pragma region Inicializa Variaveis

	
	//A tela recebeu um comando de limpesa?
	int limpoRecentemente = 0;

	//A cada quantos "turnos" a bola perde um turno
	limitadorDeVelocidadeBola = 3;

	//Duracao da partida
	int tempoMaxDeJogo = 60;

	//Tamanho da janela em pixels, de acordo com o numero de colunas
	int tamanhoDaLarguraJanela = TERMINALCOLUNAS*ESCALA;

	//Tamanho da janela em pixels, de acordo com o numero de linhas
	int tamanhoDaAlturaJanela = CAMPODEVISAO*ESCALA;
	
	//Inicializa quadrado em x e y normalizado para servir como EBO padrão.
	quadrado Bola = inicializaQuadrado(0, 0);

	//Contadores
	int i, j;
	
	//Tempo atual desde o inicio do jogo
	int tempoDeJogo;

	
	//Inicializa vetor da tela
	zeraTela(vetorDeDadosRecebido);
	zeraTela(vetorDeDadosInicial);


	//Inicializa a bola nas coredenadas especificadas;
	bola bolaPadrao = inicializaBola(32, 49,0,0);

	//Cria time 1:
	jogador Carlos = inicializaJogador(26, 40, 1), Jorge = inicializaJogador(32, 48, 1), Marcos = inicializaJogador(38, 40, 1),lugo = inicializaJogador(38,36,1);
	jogador time1[TAMANHODOTIME] = { Carlos,Jorge,Marcos,lugo};

	//Cria time 2:
	jogador Joao = inicializaJogador(26, 60, 2), Pedro = inicializaJogador(32, 50, 2), Lopes = inicializaJogador(38, 60, 2), Lopeso = inicializaJogador(33, 60, 2);
	jogador time2[TAMANHODOTIME] = {Joao,Pedro,Lopes,Lopeso};

	//Goleiros
	jogador goleiro1 = inicializaJogador(32, 98, 2);
	jogador goleiro2 = inicializaJogador(32, 1, 1);




	//Lista de todas as VAOs usadas no programa
	GLint listaDeEnderecosVAO[NUMVAO];

	
#pragma endregion

	#pragma region Inicializa GLFW e GL3W

	//Inicializa glfw
	glfwInit();

	double tempoCorrido;
	//Variaveis para os relogios
	clock_t relogioInicial = clock(), relogioAtual;
	
	//Tempo limite da partida em segundos
	//Define versão do opengl para 3+
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);

	//Define versão do opengl para 3+
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	//Define core profile do opengl para ser utilizado
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//o tipo GLFWwindow é uma estrutura definiao pelo GLFW.
	//A função glfwCreateWindow recebe int altura, int largura, char[] nome, GLFWmonitor monitor(nulo), GLFWWINDOW share(nulo)
	//E cria uma estrutura do tipo GLFWwindow com os parametros recebidos, aloca esta na memoria
	//E devolve o endereço dessa estrutura na memoria
	//Cria a janela principal, em proporção ao tamanho da matriz de entrada, e guarda o seu local na memoria
	GLFWwindow* janela = glfwCreateWindow(tamanhoDaLarguraJanela, tamanhoDaAlturaJanela, "GLSOCCER", NULL, NULL);
	glfwMakeContextCurrent(janela); //Torna a janela janela a janela atual

	//define framzebuffer_size_callback como a funcao padrao de callback de redimensionamento
	glfwSetFramebufferSizeCallback(janela, framebuffer_size_callback);

	//Define funcao recebeEntrada como funcao de callback padrao de entrada
	glfwSetKeyCallback(janela, recebeEntrada);

	//Inicializa gl3w
	gl3wInit();

	//Posicao da criacao da janela ao lado do console
	glfwSetWindowPos(janela, 500, 100);


#pragma endregion

	#pragma region  Inicializa OPENGL

	//Inicializa janela do opengl do tamanho da janela do GLFW
	glViewport(0, 0, tamanhoDaLarguraJanela, tamanhoDaAlturaJanela);
	
	//Define a cor da janela em RGBT
	glClearColor(0.32f, 0.61f, 0.002f, 1.0f);
	
	//Limpa janela
	glClear(GL_COLOR_BUFFER_BIT);
	
	//Limpa o buffer
	glfwSwapBuffers(janela);
	glClear(GL_COLOR_BUFFER_BIT);

	//Cria a EBO quadrado para definir como é a leitura dos indices dos vertices
	unsigned int EBOquadrado;

	//Cria uma EBO e atribui no int o endereço de memoria
	glGenBuffers(1, &EBOquadrado);
	
	//Binda EBO no buffer e EBO(e na VAO) 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOquadrado);
	
	//Define indicesDoQuadrado como fonte da EBO
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Bola.indices), Bola.indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	/////////////////Shaders,VAOs e VBOs//////////////

	//Shader padrão só multiplica os vertices pela matriz de transformação padrao
	//Compila shader padrão, e cria o objeto Shader	
	Shader shaderPadrao("verticePadrao.vs", "fragPadrao.frag");
	
	//Cria uma nova VAO sem nenhuma atribuição para como ler os dados
	GLint novaVAO = inicializaVAOVazia(EBOquadrado);
	
	//Coloca Vao na lista de vaos da maquina
	listaDeEnderecosVAO[0] = novaVAO;
	
	//Cria vetor para receber coordeadas normalizadas da funcao
	GLfloat XYnormalizado[2];

	//Define os limites dos times 1 e 2
	int limiteT1[8],limiteT2[8];
	fixaTime(time1,limiteT1);
	fixaTime(time2, limiteT2);
	//getchar();

	//Raw timer no comeco do programa
//	unsigned long int rawTime = glfwGetTimerValue();

#pragma endregion

	#pragma region Loop Principal do jogo

	//Enquanto a janela não fecha
	while (!glfwWindowShouldClose(janela)) 
	{
		
		#pragma region Inicializa no Loop

		glfwSetTime(0.0);

		//Atualiza campo de visao
		fimDoCampoDeVisao = inicioDoCampoDeVisao + CAMPODEVISAO;

		atuallizaMatrizCampoDeVisao(vetorDeDadosRecebido, vetorDoCampoDeVisao);
#pragma endregion	
				
		#pragma region Desenha na Tela

		//Loop de desenho na tela
		for (i = 0; i < TERMINALCOLUNAS; i++)
		{
			for (j = 0; j <CAMPODEVISAO; j++)
			{
				//Desenha chamando funcao de desenho dependendo do valor na matriz de dados
				
				normaliza((float)i, (float)j, XYnormalizado);
				desenhaPeloCodigo(vetorDoCampoDeVisao[i][j], shaderPadrao, listaDeEnderecosVAO, XYnormalizado[0],XYnormalizado[1]);
				
			}
		}
#pragma endregion

		#pragma region Detecta entrada
		
		//Recebe eventos por callback
		glfwPollEvents();

		//Detecta entrada
		if (glfwGetKey(janela, GLFW_KEY_DOWN) == GLFW_PRESS) {

			//Marca seta pra baixo como pressionada
			vetorDeSetas[2] = 1;

			//Movimenta o time especificado
			if (!moveTime(0, 1, time1, time1, limiteT1, &bolaPadrao, vetorDeSetas) && fimDoCampoDeVisao < 100)
			{
				inicioDoCampoDeVisao++;
			}

		}
		if (glfwGetKey(janela, GLFW_KEY_UP) == GLFW_PRESS) {

			//Marca seta pra cima como pressionada
			vetorDeSetas[3] = 1;

			//Muda a posicao do time, checa colisoes e atualiza campo de visao
			if (!moveTime(0, -1, time1, time1, limiteT1, &bolaPadrao, vetorDeSetas) &&inicioDoCampoDeVisao > 0)
			{
				inicioDoCampoDeVisao--;
			}
			
		}
		if (glfwGetKey(janela, GLFW_KEY_LEFT) == GLFW_PRESS) 
		{
			//Marca seta pra esquerda como pressionada
			vetorDeSetas[1] = 1;
			moveTime(1, 0, time1, time1,limiteT1, &bolaPadrao, vetorDeSetas);
		}
		if (glfwGetKey(janela, GLFW_KEY_RIGHT) == GLFW_PRESS) {


			//Marca seta pra direita como pressionada
			vetorDeSetas[0] = 1;
			//Muda a posicao da bola em 1 pra baixo
			moveTime(-1, 0, time1, time1, limiteT1, &bolaPadrao, vetorDeSetas);

			//posicionaBola(&bolaPadrao,bolaPadrao.x - 1, bolaPadrao.y);
		}
		if (glfwGetKey(janela, GLFW_KEY_W) == GLFW_PRESS) {

			//Marca seta pra cima como pressionada
			vetorDeSetas2[3] = 1;

			//Muda a posicao do time, checa colisoes e atualiza campo de visao
			moveTime(0, -1, time2, time2, limiteT2, &bolaPadrao, vetorDeSetas2);
			
		}
		if (glfwGetKey(janela, GLFW_KEY_S) == GLFW_PRESS) {

			//Marca seta pra baixo como pressionada
			vetorDeSetas2[2] = 1;

			//Movimenta o time especificado
			moveTime(0, 1, time2, time2, limiteT2, &bolaPadrao, vetorDeSetas2);
			
		}
		if (glfwGetKey(janela, GLFW_KEY_A) == GLFW_PRESS)
		{

			//Marca seta pra esquerda como pressionada
			vetorDeSetas2[1] = 1;
			moveTime(1, 0, time2, time2, limiteT2, &bolaPadrao, vetorDeSetas2);
		}
		if (glfwGetKey(janela, GLFW_KEY_D) == GLFW_PRESS)
		{
			//Marca seta pra direita como pressionada
			vetorDeSetas2[0] = 1;
			//Muda a posicao da bola em 1 pra baixo
			moveTime(-1, 0, time2, time2, limiteT2, &bolaPadrao, vetorDeSetas2);
		}
		
#pragma endregion
		#pragma region Atualizacoes
		//Atualiza valores do jogo
		if ((contadordeframes % limitadorDeVelocidadeBola)!=1)
		{
			limpoRecentemente = atualizaBola(&bolaPadrao);
			

		}

		//Velocidade do goleiro
		if ((contadordeframes % 2) == 1)
		{
			atualizaGoleiro(goleiro1, &goleiro1);
			atualizaGoleiro(goleiro2, &goleiro2);
		}

		//Limpa vetor de setas pressionadas a cada 0.2sec
		if (contadordeframes % (FRAMERATE/5) == 1)
		{
			vetorDeSetas[0] = 0;
			vetorDeSetas[1] = 0;
			vetorDeSetas[2] = 0;
			vetorDeSetas[3] = 0;

			vetorDeSetas2[0] = 0;
			vetorDeSetas2[1] = 0;
			vetorDeSetas2[2] = 0;
			vetorDeSetas2[3] = 0;
		}
	
		//Limpa tela se necessario
		if (limpoRecentemente == 1)
		{

			limpaTime(time1);
			limpaTime(time2);
			posicionaBola(&bolaPadrao, 32, 49);
			bolaPadrao.velX = 0;
			bolaPadrao.velY = 0;
			limpoRecentemente = 0;
			inicioDoCampoDeVisao = 30;
		}
#pragma endregion

		#pragma region Troca buffers

		//Troca buffers
		glfwSwapBuffers(janela);
#pragma endregion


		#pragma region Forca framerate
		contadordeframes++;
		system("@cls||clear");
		tempoCorrido = glfwGetTime();
		if (((1.0 / (double)FRAMERATE) - tempoCorrido) > 0)
		{
			
			Sleep((DWORD)(1.0 / (double)FRAMERATE - tempoCorrido)*1000);
		}
		printf("Framerate fixada: %.2lf\n", 1 / glfwGetTime());
		printf("Framerate original: %.2lf\n", 1 / tempoCorrido);
		tempoDeJogo = ((relogioAtual = clock()) - relogioInicial) / 1000;
		printf("Relogio: %i\nFaltam: %i segundos para o fim do jogo\n", tempoDeJogo,tempoMaxDeJogo-tempoDeJogo);
		printf("Time Azul %i - %i Time Vermelho\n", placar[1], placar[0]);
		

#pragma endregion		 
		

	}

#pragma endregion
}

#pragma region Funcoes

void atualizaGoleiro(jogador goleiroRecebido, jogador *goleiroRetornado)
{
	//Se o goleiro está no gol e pode ir para direita
	if (vetorDeDadosRecebido[goleiroRecebido.x + 1][goleiroRecebido.y] == 8 && posBola[0] > goleiroRecebido.x)
	{
		posicionaJogador(goleiroRecebido.x + 1, goleiroRecebido.y, goleiroRecebido, goleiroRetornado);
	}

	//Se o goleiro está no gol e pode ir para esquerda
	if (vetorDeDadosRecebido[goleiroRecebido.x - 1][goleiroRecebido.y] == 8 && posBola[0] < goleiroRecebido.x)
	{
		posicionaJogador(goleiroRecebido.x - 1, goleiroRecebido.y, goleiroRecebido, goleiroRetornado);
	}
}

void fixaTime(jogador timeRecebido[TAMANHODOTIME], int *retorno)
{
	int c;
	int maiorX=0, maiorY=0, menorX=180, menorY=180;
	int idMaiorX, idMaiorY, idMenorX, idMenorY;
//	int retornotmp[8];
	//Variavel para guardar o retorno. devolve IDmaiorX,maiorX,idMenorX,menorX,idMaiorY,maiorY,idMenorY,menorY


	for (c = 0; c < TAMANHODOTIME; c++)
	{
		if (timeRecebido[c].x > maiorX)
		{
			maiorX = timeRecebido[c].x;
			idMaiorX = c;
		}

		if (timeRecebido[c].x < menorX)
		{
			menorX = timeRecebido[c].x;
			idMenorX = c;
		}

		if (timeRecebido[c].y < menorY)
		{
			menorY = timeRecebido[c].y;
			idMenorY = c;
		}

		if (timeRecebido[c].y > maiorY)
		{
			maiorY = timeRecebido[c].y;
			idMaiorY = c;
		}


	}

	//printf("MaiorX:%i MenorX:%i MaiorY:%i MenorY:%i \n idMaiorX:%i idMenorX:%i idMaiorY:%i idMenorY:%i", maiorX, menorX, maiorY, menorY,idMaiorX,idMenorX,idMaiorY,idMenorY);
	

	*retorno = idMaiorX;
	*(retorno+1) = maiorX;
	*(retorno+2) = idMenorX;
	*(retorno+3) = menorX;
	*(retorno + 4) = idMaiorY;
	*(retorno + 5) = maiorY;
	*(retorno + 6) = idMenorX;
	*(retorno + 7) = maiorX;

}

void recebeEntrada(GLFWwindow *window, int key, int scancode, int action, int mods){
//glfwGetKey:
//recebe: GLFWwindow, GLFWKEY(paratro unico para cada tecla que indica o estado atual(PRESS, RELEASE, HOLD...) da tecla	
//retorna:Bool indicando se a tecla está no estado passado para a função

//Verifica se a tecla escape foi pressionada

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
		 //Se sim, sai da janela
	}	
}

void normaliza(float originalX, float originalY, GLfloat *retorno)
{

	GLfloat resultado[2];
	resultado[0] = 1 - (originalX / (TERMINALCOLUNAS / 2));
	resultado[1] = 1 - (originalY / (CAMPODEVISAO / 2));
	*retorno = resultado[0];
	*(retorno + 1) = resultado[1];

}

void deNormaliza(float originalX, float originalY, GLfloat *retorno)
{

	GLfloat resultado[2];
	resultado[0] = -(TERMINALCOLUNAS / 2) *(originalX - 1);
	resultado[1] = -(CAMPODEVISAO/ 2) *(originalY - 1);
	*retorno = resultado[0];
	*(retorno + 1) = resultado[1];

}

quadrado inicializaQuadrado(GLfloat x, GLfloat y)
{
	//Cria quadrado para retorno
	quadrado retorno;

	//Preenche os vertices do quadrado
	GLfloat seguravertices[12] = {
		x + PASSOX,  y, 0.0f,  // Topo direita
		x + PASSOX, y - PASSOY, 0.0f,  // Baixo Direuta
		x, y - PASSOY, 0.0f,  // Baixo Esuqerda
		x,  y, 0.0f   // Topo Esquerda 
	};

	retorno.posx = x;
	retorno.posy = y;
	memcpy(retorno.vertices, seguravertices, sizeof(seguravertices));

	return retorno;
}

void zeraTela(int vetorDaTela[TERMINALCOLUNAS][TERMINALLINHAS])
{
	int i, j;

	//Preenche o campo
	for (i = 0; i < TERMINALCOLUNAS; i++)
	{
		for (j = 0; j < TERMINALLINHAS; j++)
		{
		
			//Faz linhas brancas verticais(fora do campo)
			if (i == 2 || i == 63)
			{
				*(*(vetorDaTela + i) + j) = 9;
			}

			//Faz linhas brancas horizontais(gol)
			else if (j == 1 || j == 98 || j == 0 || j == 99	)
			{
				//Cria o espaco do goleiro
				if (i > 27 && i < 37)
				{
					*(*(vetorDaTela + i) + j) = 8;
				}
				//Não desenha nos cantos
				else if (i != 1) 
				{
					*(*(vetorDaTela + i) + j) = 9;
				}
			}
			//Desenha meio de campo
			else if (j == 49)
			{
				if (i != 1)
				*(*(vetorDaTela + i) + j) = 7;
			}

			//Preenche com 0
			else
			{
				*(*(vetorDaTela + i) + j) = 0;
			}
		}

	}

	//Cria meio do campo de baixo
	vetorDaTela[38][50] = 7;
	vetorDaTela[37][51] = 7;
	vetorDaTela[36][52] = 7;
	vetorDaTela[35][53] = 7;
	vetorDaTela[34][54] = 7;
	vetorDaTela[33][55] = 7;
	vetorDaTela[32][56] = 7;

	vetorDaTela[26][50] = 7;
	vetorDaTela[27][51] = 7;
	vetorDaTela[28][52] = 7;
	vetorDaTela[29][53] = 7;
	vetorDaTela[30][54] = 7;
	vetorDaTela[31][55] = 7;
	vetorDaTela[32][56] = 7;



	
	//Cria meio de campo de cima
	vetorDaTela[25][49] = 7;
	vetorDaTela[26][48] = 7;
	vetorDaTela[27][47] = 7;
	vetorDaTela[28][46] = 7;
	vetorDaTela[29][45] = 7;
	vetorDaTela[30][44] = 7;
	vetorDaTela[31][43] = 7;
	vetorDaTela[32][42] = 7;


	vetorDaTela[39][49] = 7;
	vetorDaTela[38][48] = 7;
	vetorDaTela[37][47] = 7;
	vetorDaTela[36][46] = 7;
	vetorDaTela[35][45] = 7;
	vetorDaTela[34][44] = 7;
	vetorDaTela[33][43] = 7;
	vetorDaTela[32][42] = 7;

}

GLint inicializaVAOVazia(unsigned int recebeEBO)
{
	unsigned int EBOVAO;//Cria a EBO quadrado para definir como é a leitura dos indices dos vertices
	glGenBuffers(1, &EBOVAO);//Cria uma EBO e atribui no int o endereço de memoria
	EBOVAO = recebeEBO;
	GLuint VBOVAO, VAOVAO; //Cria int para receber endereço da VBO que representa o objeto
	glGenVertexArrays(1, &VAOVAO);//Gera um vertex array vazio object e salva endereço dele no int espeficiado
	glGenBuffers(1, &VBOVAO);//Gera um vertex buffer vazio object e salva o endereço dele no int especificado 
							 //Binda no buffer de VAO o vao especificado, copiando o valor do vao para o buffer(vazio), e alterando a VAO 
							 //bindada a cada alteração no buffer. Cada VAO guarda uma configuração especifica de como interpretar um VBO
							 //Especifico e passar para o shader especificado, e todas as alterações e binds a seguir continuam
							 //na VAO apos ser unbindada
	glBindVertexArray(VAOVAO);

	//Binda EBO no buffer e EBO(e na VAO) 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOVAO);
	//Binda VBO no buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBOVAO);

	//Define como a VAO vai interpretar os VBOS e EBOS espeficados anteriormente
	//glvertexattribpointer(indice,size,type,normalized,stride,pointer: Define como interpretar dados de VBOS.
	//int indice:indice dos dados para ser passado para o shader
	//int size: numero de componentes por vertice(1,2,3,4)
	//type: Tipo do dado
	//Normalizar de -1 a 1?, sim ou não
	//Tamanho do salto de um vertice para o outro na vbo em bytes
	//em que posição começa a leitura de cada vertice na VBO
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	//Habilita o modo de leitura de dados especificado acima no indice 0 para o GLSL
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
	return VAOVAO;
}

bola inicializaBola(int x, int y,int velX, int velY)
{
	bola retorno;
	retorno.x = x;
	retorno.y = y;
	retorno.velX = velX;
	retorno.velY = velY;
	posBola[0] = x;
	posBola[1] = y;
	vetorDeDadosRecebido[x][y] = 3;

	return retorno;
}

void desenhaPeloCodigo(int codigo,Shader shaderUsado,int listaVAOs[NUMVAO],GLfloat x, GLfloat y)
{
	
	float corJogador[3] = { 0.1f, 0.1f, 0.2f };
	float XYdenormalizado[2];
	deNormaliza(x, y, XYdenormalizado);
	switch (codigo)
	{
	case 0:
		//if (vetorDeDadosDesenhado[(int)XYdenormalizado[0]][(int)XYdenormalizado[1]] == 1)
		//{
			desenhaVazio(shaderUsado, listaVAOs[0], x, y);	

		//}
		
		break;
	//Se o codigo for 1, desenha um Jogador usando a vao 0 da lista
	case 1:
		//Coloca R do RGB como 1, cria jogador(time vermelho)
		corJogador[0] = 0.90f;
		desenhaJogador(shaderUsado, listaVAOs[0], x ,y,corJogador);
		break;
	case 2:
		//Coloca B do RGB como 1, cria jogador(time azul)
		corJogador[2] = 0.90f;
		desenhaJogador(shaderUsado, listaVAOs[0], x, y, corJogador);
		break;
	case 3:
		//Coloca RBG do RBG como 1, cria jogador(bola branca)
		corJogador[0] = 1.0f;
		corJogador[1] = 1.0f;
		corJogador[2] = 1.0f;
		desenhaJogador(shaderUsado, listaVAOs[0], x, y, corJogador);
		break;

	case 7:
		desenhaFaixaDoCampo(shaderUsado, listaVAOs[0], x, y);
		break;
	case 8:
		desenhaEspacoGoleiro(shaderUsado, listaVAOs[0], x, y);
		break;
	case 9:
		desenhaEmBranco(shaderUsado, listaVAOs[0], x, y);
		break;
	default:
		break;
	}

	//Limpa estado de desenhado
	vetorDeDadosDesenhado[(int)XYdenormalizado[0]][(int)XYdenormalizado[1]] = 0;
}

void desenhaJogador(Shader shaderUsado, int VAO,GLfloat x, GLfloat y, float cor[3]) {

	//Binda a VAO recebida no buffer
	glBindVertexArray(VAO);
	//Inicializa um quadrado na posicao recebida na funcao
	quadrado jogadorAtivo = inicializaQuadrado(x, y);
	//Passa EBO do quadrado criado como indice do elemento
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(jogadorAtivo.indices), jogadorAtivo.indices, GL_STATIC_DRAW);
	//Passa o quadrado jogador ativo como valor da VBO da VAO recebida
	glBufferData(GL_ARRAY_BUFFER, sizeof(jogadorAtivo.vertices), jogadorAtivo.vertices, GL_STATIC_DRAW);
	//Define como o shader vai interpretar os dados do VAO
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	//unbinda VBO e VAO
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	
	//Define a cor, passando pro uniform
	glUniform3f(glGetUniformLocation(shaderUsado.ID, "cor"),cor[0],cor[1],cor[2]);
	//Define a matrix de transformação passada pro uniform
	glUniformMatrix4fv(glGetUniformLocation(shaderUsado.ID, "transforma"),1, GL_FALSE,glm::value_ptr(matrixIdentidade));
	//Usa o shader padrao como shader
	shaderUsado.use();
	//Binda a VAO criada acima
	glBindVertexArray(VAO);
	//Desenha o triangulo
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);//Unbinda a VAO	
}

void desenhaEmBranco(Shader shaderUsado, int VAO, GLfloat x, GLfloat y) {

	glBindVertexArray(VAO);
	quadrado jogadorAtivo = inicializaQuadrado(x, y);
	glBufferData;
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(jogadorAtivo.indices), jogadorAtivo.indices, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, sizeof(jogadorAtivo.vertices), jogadorAtivo.vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);


	//Define a cor, passando pro uniform
	glUniform3f(glGetUniformLocation(shaderUsado.ID, "cor"), 1.0f, 1.0f, 1.0f);
	//Define a matrix de transformação passada pro uniform
	glUniformMatrix4fv(glGetUniformLocation(shaderUsado.ID, "transforma"), 1, GL_FALSE, glm::value_ptr(matrixIdentidade));
	//Usa o shader padrao como shader
	shaderUsado.use();
	//Binda a VAO criada acima
	glBindVertexArray(VAO);
	//Desenha o triangulo
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);//Unbinda a VAO	
}

void desenhaEspacoGoleiro(Shader shaderUsado, int VAO, GLfloat x, GLfloat y) {

	glBindVertexArray(VAO);
	quadrado jogadorAtivo = inicializaQuadrado(x, y);
	glBufferData;
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(jogadorAtivo.indices), jogadorAtivo.indices, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, sizeof(jogadorAtivo.vertices), jogadorAtivo.vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	
	//Define a cor, passando pro uniform
	glUniform3f(glGetUniformLocation(shaderUsado.ID, "cor"), 0.4f, 0.486f, 0.482f);
	//Define a matrix de transformação passada pro uniform
	glUniformMatrix4fv(glGetUniformLocation(shaderUsado.ID, "transforma"), 1, GL_FALSE, glm::value_ptr(matrixIdentidade));
	//Usa o shader padrao como shader
	shaderUsado.use();
	//Binda a VAO criada acima
	glBindVertexArray(VAO);
	//Desenha o triangulo
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);//Unbinda a VAO	
}

void desenhaFaixaDoCampo(Shader shaderUsado, int VAO, GLfloat x, GLfloat y) {

	glBindVertexArray(VAO);
	quadrado jogadorAtivo = inicializaQuadrado(x, y);
	glBufferData;
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(jogadorAtivo.indices), jogadorAtivo.indices, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, sizeof(jogadorAtivo.vertices), jogadorAtivo.vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);


	//Define a cor, passando pro uniform
	glUniform3f(glGetUniformLocation(shaderUsado.ID, "cor"), 0.91f, 0.85f, 0.93f);
	//Define a matrix de transformação passada pro uniform
	glUniformMatrix4fv(glGetUniformLocation(shaderUsado.ID, "transforma"), 1, GL_FALSE, glm::value_ptr(matrixIdentidade));
	//Usa o shader padrao como shader
	shaderUsado.use();
	//Binda a VAO criada acima
	glBindVertexArray(VAO);
	//Desenha o triangulo
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);//Unbinda a VAO	
}

void desenhaVazio(Shader shaderUsado, int VAO, GLfloat x, GLfloat y) {

	glBindVertexArray(VAO);
	quadrado jogadorAtivo = inicializaQuadrado(x, y);
	
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(jogadorAtivo.indices), jogadorAtivo.indices, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, sizeof(jogadorAtivo.vertices), jogadorAtivo.vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	
	//Define a cor, passando pro uniform
	glUniform3f(glGetUniformLocation(shaderUsado.ID, "cor"), 0.32f, 0.61f, 0.002f);
	//Define a matrix de transformação passada pro uniform
	glUniformMatrix4fv(glGetUniformLocation(shaderUsado.ID, "transforma"), 1, GL_FALSE, glm::value_ptr(matrixIdentidade));
	//Usa o shader padrao como shader
	shaderUsado.use();
	//Binda a VAO criada acima
	glBindVertexArray(VAO);
	//Desenha o triangulo
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);//Unbinda a VAO	
	

}

void atuallizaMatrizCampoDeVisao(int matrizOriginal[TERMINALCOLUNAS][TERMINALLINHAS], int enderecoDoRetorno[TERMINALCOLUNAS][CAMPODEVISAO])
{

//	int matrizDeRetorno[TERMINALCOLUNAS][CAMPODEVISAO];
	int i, j;
	for (i = 0; i < TERMINALCOLUNAS; i++)
	{
		for (j = 0; j < CAMPODEVISAO; j++)
		{


			*(*(enderecoDoRetorno + i) + j) = matrizOriginal[i][j + inicioDoCampoDeVisao];
		}
	}


}

void posicionaJogador(int x, int y, jogador jogadorRecebido, jogador *retorno)
{
	//int colisao = 0;
	
	
	//Verifica se é possivel posicionar o jogador no local desejado
	//if (vetorDeDadosRecebido[x][y] == 0 || vetorDeDadosRecebido[x][y] == 7)
	//{
		//Posicao futura que o elemento vai ser desenhado, marcado como desenhado
		vetorDeDadosDesenhado[x][y] = 1;

		//Posicao do elemento atualizado
		vetorDeDadosRecebido[x][y] = jogadorRecebido.time;

		//Limpa posicao original(antiga) do jogador recebido
		vetorDeDadosRecebido[jogadorRecebido.x][jogadorRecebido.y] = vetorDeDadosInicial[jogadorRecebido.x][jogadorRecebido.y];

		//Atualiza posição antiga do elemento
		jogadorRecebido.x = x;
		jogadorRecebido.y = y;

		//Retorna valor
		*retorno = jogadorRecebido;

	//}
}

jogador inicializaJogador(int x, int y, int Time)
{

	jogador retorno;
	retorno.x = x;
	retorno.y = y;
	retorno.posInicialX = x;
	retorno.posInicialY = y;
	retorno.time = Time;
	vetorDeDadosRecebido[x][y] = Time;
	return retorno;

}

void posicionaBola(bola *bolaRecebida,int x, int y)
{
	
	//Verifica se é possivel posicionar a bola no local desejado
	if (vetorDeDadosRecebido[x][y] == 0 || vetorDeDadosRecebido[x][y] == 7)
	{
		//Posicao futura da bola vai ser desenhada, marcada como desenhada
		vetorDeDadosDesenhado[x][y] = 1;
		//Posicao da bola atualizada
		vetorDeDadosRecebido[x][y] = 3;
		//Usa posicao antiga da bola pra atualizar o espaco antigo da bola com seu original
		vetorDeDadosRecebido[posBola[0]][posBola[1]] = vetorDeDadosInicial[posBola[0]][posBola[1]];

		//Atualiza posição antiga da bola
		posBola[0] = x;
		posBola[1] = y;

		//Atualiza bolaRecebida
		
		bolaRecebida->x = x;
		//bolaRecebida.x = x;
		bolaRecebida->y = y;
	}
}

int moveTime(int distanciaX, int distanciaY, jogador timeRecebido[TAMANHODOTIME], jogador *retorno, int fixador[8],bola *bolaRecebida,bool vetorDeSetasRecebidas[4])
{
	//Cria vetor de retorno
//	jogador timeDeRetorno[TAMANHODOTIME];

	int i, colisao = 0, colisaobola = 0, chuteBolaX = 0, chuteBolaY = 0;

	//checa colisoes na matriz
	//Verifica se o jogador com o maior X pode aumentar X
	if (vetorDeDadosRecebido[timeRecebido[fixador[0]].x + distanciaX][timeRecebido[fixador[0]].y + distanciaY] == 9 || vetorDeDadosRecebido[timeRecebido[fixador[0]].x + distanciaX][timeRecebido[fixador[0]].y + distanciaY] == 8)
	{
		colisao = 1;
	}
	//Verifica se o jogador com o menor pode diminuir X
	if (vetorDeDadosRecebido[timeRecebido[fixador[2]].x + distanciaX][timeRecebido[fixador[2]].y + distanciaY] == 9 || vetorDeDadosRecebido[timeRecebido[fixador[2]].x + distanciaX][timeRecebido[fixador[2]].y + distanciaY] == 8)
	{
		colisao = 1;
	}
	//Verifica se o jogador com o maior Y pode aumentar Y
	if (vetorDeDadosRecebido[timeRecebido[fixador[4]].x + distanciaX][timeRecebido[fixador[4]].y + distanciaY] == 9 || vetorDeDadosRecebido[timeRecebido[fixador[4]].x + distanciaX][timeRecebido[fixador[4]].y + distanciaY] == 8)
	{
		//Ignora a colisão do jogador mais extremo ao norte para evitar travamentos 
		//colisao = 1;
	}
	//Verifica se o jogador com o menor Y pode mudar Y
	if (vetorDeDadosRecebido[timeRecebido[fixador[6]].x + distanciaX][timeRecebido[fixador[6]].y + distanciaY] == 9 || vetorDeDadosRecebido[timeRecebido[fixador[6]].x + distanciaX][timeRecebido[fixador[6]].y + distanciaY] == 8)
	{
		colisao = 1;
	}
	//Preenche vetor de retorno com os dados atualizados pela funcao
	
	
	//Verifica colisão nos pontos que não estão nas bordas mas que são intransponiveis
	for (i = 0; i < TAMANHODOTIME; i++)
	{
		if (vetorDeDadosRecebido[timeRecebido[i].x + distanciaX][timeRecebido[i].y + distanciaY] == 2 || vetorDeDadosRecebido[timeRecebido[i].x + distanciaX][timeRecebido[i].y + distanciaY] == 1)
		{
			colisao = 1;
		}

	}

	//Verifica colisão dos jogadores com a bola
	for (i = 0; i < TAMANHODOTIME; i++)
	{
		if (vetorDeDadosRecebido[timeRecebido[i].x + distanciaX][timeRecebido[i].y + distanciaY] == 3)
		{
			colisao = 1;
			colisaobola = 1;
			//Diz sentido do chute da bola
			chuteBolaX = distanciaX;
			chuteBolaY = distanciaY;
		}

	}

	//Movimenta a bola
	if (colisaobola == 1)
	{
		//Só vai detectar a diagonal durante o pressionamento das setas de esquerda ou direita, já que o flag
		//da tecla cima estar pressionada antecede a mesma, logo a velocidade Y é sempre a que é complementada
		vetorDeSetas;

		//detecta se a tecla pra baixo e esquerda foram pressonadas ao mesmo tempo
		if (vetorDeSetasRecebidas[2] == 1 && vetorDeSetasRecebidas[1] == 1)
		{
			bolaRecebida->velY = chuteBolaX * 5;;
			bolaRecebida->velX = bolaRecebida->velY;
		}

		//detecta se a tecla pra baixo e direita foram pressonadas ao mesmo tempo
		else if (vetorDeSetasRecebidas[2] == 1 && vetorDeSetasRecebidas[0] == 1)
		{
			bolaRecebida->velY = chuteBolaX * -5;
			bolaRecebida->velX = bolaRecebida->velY*-1;
		}

		//detecta se a tecla pra cima e direita foram pressonadas ao mesmo tempo
		else if (vetorDeSetasRecebidas[3] == 1 && vetorDeSetasRecebidas[0] == 1)
		{
			bolaRecebida->velY = (chuteBolaX * 5);
			bolaRecebida->velX = bolaRecebida->velY;
		}

		//detecta se a tecla pra cima e esquerda foram pressonadas ao mesmo tempo
		else if (vetorDeSetasRecebidas[3] == 1 && vetorDeSetasRecebidas[1] == 1)
		{
			bolaRecebida->velY = chuteBolaX * -5;
			bolaRecebida->velX = bolaRecebida->velY*-1;
		}
		else
		{
			bolaRecebida->velX = chuteBolaX * 5;
			bolaRecebida->velY = chuteBolaY * 5;
		}
	}

	//Movimenta os jogadores
	if (colisao == 0)
	{
		for (i = 0; i < TAMANHODOTIME; i++)
		{
			posicionaJogador(timeRecebido[i].x + distanciaX, timeRecebido[i].y + distanciaY, timeRecebido[i], &timeRecebido[i]);
		}

	}
	//Retorna se houve colisao ou não
	return colisao;
}

int atualizaBola(bola *bolaRecebida)
{

	//Atualiza posicao da bola a cada frame, dependendo da velocidade X e Y da bola
	int foiLimpo = 0;

	//Sentido esquerda
	if (bolaRecebida->velX > 0)
	{
		posicionaBola(bolaRecebida, (bolaRecebida->x) + 1, (bolaRecebida->y) + 0);
		bolaRecebida->velX--;
	}
	//Sentido direita
	if (bolaRecebida->velX < 0)
	{
		posicionaBola(bolaRecebida, (bolaRecebida->x) - 1, (bolaRecebida->y) + 0);
		bolaRecebida->velX++;
	}
	//Sentido baixo
	if (bolaRecebida->velY > 0)
	{
		posicionaBola(bolaRecebida, (bolaRecebida->x) + 0, (bolaRecebida->y) + 1);
		bolaRecebida->velY--;
	}
	//Sentido cima
	if (bolaRecebida->velY < 0)
	{
		posicionaBola(bolaRecebida, (bolaRecebida->x) + 0, (bolaRecebida->y) - 1);
		bolaRecebida->velY++;
	}

	//Verifica colisao da bola com o gol
	//Checa gol de baixo
	if (vetorDeDadosRecebido[bolaRecebida->x][bolaRecebida->y+1] == 8 && bolaRecebida->velY > 1)
	{
		posicionaBola(bolaRecebida, bolaRecebida->x, bolaRecebida->y + bolaRecebida->velY);
		
		bolaRecebida->velY = 0;

		//Faz a bola "entrar no gol"
		vetorDeDadosRecebido[bolaRecebida->x][(bolaRecebida->y)+ 2] = 3;
		vetorDeDadosRecebido[bolaRecebida->x][bolaRecebida->y] = vetorDeDadosInicial[bolaRecebida->x][bolaRecebida->y];
		placar[0]++;
		foiLimpo = 1;

	}
	//Checa gol de cima 
	if (vetorDeDadosRecebido[bolaRecebida->x][bolaRecebida->y - 1] == 8 && bolaRecebida->velY < -1)
	{
		posicionaBola(bolaRecebida, bolaRecebida->x, bolaRecebida->y + bolaRecebida->velY);

		bolaRecebida->velY = 0;

		//Faz a bola "entrar no gol"
		vetorDeDadosRecebido[bolaRecebida->x][(bolaRecebida->y) - 2] = 3;
		vetorDeDadosRecebido[bolaRecebida->x][bolaRecebida->y] = vetorDeDadosInicial[bolaRecebida->x][bolaRecebida->y];
		placar[1]++;
		foiLimpo = 1;
		
	}

	//Verifica colisão da bola com as paredes
	//Simplesmente inverte o sentido da bola dependendo da direção da parece colidida

	//Se ao subir a bola no plano (y-1) a bola encontrar uma parede, encontrou o limite superior
	if (vetorDeDadosRecebido[bolaRecebida->x][bolaRecebida->y - 1] == 9)
	{
		//Inverte sentido da velocidade Y
		bolaRecebida->velY = bolaRecebida->velY*-1;
	}

	//Se ao descer a bola no plano (y+1) a bola encontrar uma parede, encontrou o limite inferior
	if (vetorDeDadosRecebido[bolaRecebida->x][bolaRecebida->y + 1] == 9)
	{
		//Inverte sentido da velocidade Y
		bolaRecebida->velY = bolaRecebida->velY*-1;
	}

	//Checa canto esquerdo
	if (vetorDeDadosRecebido[(bolaRecebida->x) + 1][bolaRecebida->y] == 9)
	{
		bolaRecebida->velX = bolaRecebida->velX*-1;
	}

	//Checa canto direito
	if (vetorDeDadosRecebido[(bolaRecebida->x) - 1][bolaRecebida->y] == 9)
	{
		bolaRecebida->velX = bolaRecebida->velX*-1;
	}

	return foiLimpo;

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void limpaTime(jogador timeRecebido[TAMANHODOTIME])
{
	int i;

	//Retorna todos os jogadores de ambos os times para a suas posicoes inicias
	for (i = 0; i < TAMANHODOTIME; i++)
	{
		/*
		vetorDeDadosRecebido[timeRecebido[i].x][timeRecebido[i].y] = vetorDeDadosInicial[timeRecebido[i].x][timeRecebido[i].y];
		timeRecebido[i].x = timeRecebido[i].posInicialX;
		timeRecebido[i].y = timeRecebido[i].posInicialY;
		vetorDeDadosRecebido[timeRecebido[i].x][timeRecebido[i].y] = vetorDeDadosInicial[timeRecebido[i].x][timeRecebido[i].y];
		*/

		posicionaJogador(timeRecebido[i].posInicialX, timeRecebido[i].posInicialY, timeRecebido[i], &timeRecebido[i]);
	}
}

 #pragma endregion