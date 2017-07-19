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
int inicioDoCampoDeVisao = 30, fimDoCampoDeVisao = inicioDoCampoDeVisao + CAMPODEVISAO;

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

void atualizaGoleiro(jogador goleiroRecebido, jogador *goleiroRetornado);


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
bola inicializaBola(int x, int y, int velX, int velY);

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
void posicionaBola(bola *bolaRecebida, int x, int y);

//
//Move um time inteiro uma distancia X,Y especificada
int moveTime(int distanciaX, int distanciaY, jogador timeRecebido[TAMANHODOTIME], jogador *retorno, int fixador[8], bola *bolaRecebida, bool vetorDeSetas[4]);


//Mede os limites X e Y do time dado para fixar a formacao especificada. retorna uma lista de valores
//encodada em decimal da seguinte forma: [ipppipppipppippp] onde i é o id(0 a 9) e p a posicao(0 a 999) dos valore
//maxX,MinX,maxY,minY. Lembrando que qualquer valor pode então ser obtido a partidir de uma operação de modulo.
void fixaTime(jogador timeRecebido[TAMANHODOTIME], int *retorno);

//
//Atualiza a posição da bola no loop do jogo a cada frame. Verifica colisoes e gerencia a velocidade da bola
int atualizaBola(bola *bolaRecebida);
#pragma endregion

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
	int maiorX = 0, maiorY = 0, menorX = 180, menorY = 180;
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
	*(retorno + 1) = maiorX;
	*(retorno + 2) = idMenorX;
	*(retorno + 3) = menorX;
	*(retorno + 4) = idMaiorY;
	*(retorno + 5) = maiorY;
	*(retorno + 6) = idMenorX;
	*(retorno + 7) = maiorX;

}

void recebeEntrada(GLFWwindow *window, int key, int scancode, int action, int mods) {
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
	resultado[1] = -(CAMPODEVISAO / 2) *(originalY - 1);
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
			else if (j == 1 || j == 98 || j == 0 || j == 99)
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

bola inicializaBola(int x, int y, int velX, int velY)
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

void desenhaPeloCodigo(int codigo, Shader shaderUsado, int listaVAOs[NUMVAO], GLfloat x, GLfloat y)
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
		desenhaJogador(shaderUsado, listaVAOs[0], x, y, corJogador);
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

void desenhaJogador(Shader shaderUsado, int VAO, GLfloat x, GLfloat y, float cor[3]) {

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
	glUniform3f(glGetUniformLocation(shaderUsado.ID, "cor"), cor[0], cor[1], cor[2]);
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

void posicionaBola(bola *bolaRecebida, int x, int y)
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

int moveTime(int distanciaX, int distanciaY, jogador timeRecebido[TAMANHODOTIME], jogador *retorno, int fixador[8], bola *bolaRecebida, bool vetorDeSetasRecebidas[4])
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
	if (vetorDeDadosRecebido[bolaRecebida->x][bolaRecebida->y + 1] == 8 && bolaRecebida->velY > 1)
	{
		posicionaBola(bolaRecebida, bolaRecebida->x, bolaRecebida->y + bolaRecebida->velY);

		bolaRecebida->velY = 0;

		//Faz a bola "entrar no gol"
		vetorDeDadosRecebido[bolaRecebida->x][(bolaRecebida->y) + 2] = 3;
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