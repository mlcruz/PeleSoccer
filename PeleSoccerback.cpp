#define TERMINALLINHAS  100
#define TERMINALCOLUNAS  64 //Tamanho do terminal
#define ESCALA 6 //Escala da tela em relacao ao terminal
#define NUMVAO 1 //Numero de vaos na lista de vao
#define CAMPODEVISAO 60//Campo de visao do jogador
#define TAMANHODOTIME 3 //Tamanho maximo de cada time

#include <glad/gl3w.h> //Versão minima e atualizada do GLEW
#include <GLFW/glfw3.h> //GLFW para renderizar a janela
#include <compilaShader.h> //Compila e linka programas de shaders utilizados
#include <Windows.h>

//Bibliotecas para operacoes com matrizes
#include <glm\glm.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//LISTA DE COISAS A FAZER:
//Basico:
//--
//1º:Estrutura pra representar e inicializar jogadores e bola:100% 
//2ºFuncoes para movimentacao de jogadores e time:90%
//3º:Interacao entre jogadores e a bola
//4º:Implementar 60 fps
//5º:Logica de jogo, verificar condicoes de pontuacao e vitoria
//6º:Leitura de arquivos com formacoes de jogadores
//7º:Implementar controle do segundo time(facil)
//8º:Implementar tabela de high scores
//--
//Extra:
//8º:Texturas
//9º:Som
//10º:Rescrever a funcão de desenho do jogador para permitir 3 angulos de chute(jogador "3x3" em 1 bloco)
//11º:Customização dos jogadores(chute mais longo/mais rapido etc)
//12:º:Oponente controlado por computador
//Provavelmente nunca:
//??:Otimizar eficiencia do loop de desenho(não tem pq desenhar todos os 0s)
//??:Implementar 2d isometrico
//??:Usar tipos do opengl

#pragma region consts, variaveis e strucs globais

//Proporcao da do campo de visao em relacao a tela toda
const float PROPORCAO = TERMINALLINHAS / (float)CAMPODEVISAO;
const float PASSOX = 0.03125; //tamanho minimo de um passo X no plano 
const float PASSOY = 0.02*PROPORCAO;//Tamanho minimo de um passo no Y do plano

//Matriz que representa o estado inicial da tela, para guardar os elementos fixos.
int vetorDeDadosInicial[TERMINALCOLUNAS][TERMINALLINHAS];
//Matriz que representa a tela
int vetorDeDadosRecebido[TERMINALCOLUNAS][TERMINALLINHAS];
//Indica se um vetor foi recem desenhado ou não
bool vetorDeDadosDesenhado[TERMINALCOLUNAS][TERMINALLINHAS];
//Matriz com os dados do campo de visão atual
int vetorDoCampoDeVisao[TERMINALCOLUNAS][CAMPODEVISAO];
//Vetor que representa a posicao X,Y da bola na matriz
int posBola[2] = { 32,50 };

//Limites do campo de visão
int inicioDoCampoDeVisao=10, fimDoCampoDeVisao=inicioDoCampoDeVisao+CAMPODEVISAO;

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
};

//Estrutura que representa um jogador
typedef struct jogador 
{
	//Posicao x
	int x;
	
	//Posicao y
	int y;
	
	//1:azul, 2:Vermelho
	int time;
};

//Estrutura que representa uma bola
typedef struct bola
{
	int x;
	int y;
};
#pragma endregion

#pragma region Prototipos

//Inicializa uma VAO só com EBO, precisa de info de atribs da vbo
GLint inicializaVAOVazia(unsigned int recebeEBO);

//inicializaQuadrado:(GLfloat x, GLfloat ) -> quadrado Quadrado
//Recebe um X e um Y e retorna uma estrutura quadrado com um array de vertices naquela posição
quadrado inicializaQuadrado(GLfloat x, GLfloat y);

//Função para receber as teclas pressionadas pelo usuario
//Recebe um endereço na memoria de uma estrutura do tipo GLFW, e faz alguma coisa dependendo do estado atual de
//Alguma variavel definida nessa janela 
void recebeEntrada(GLFWwindow *window);

//Posiciona um jogador especifico na posicao dada e retorna seu estado atualizado
void posicionaJogador(int x, int y, jogador jogadorRecebido, jogador *retorno);

//Inicializa um jogador
jogador inicializaJogador(int x, int y, int Time);

//Inicializa uma bola
bola inicializaBola(int x, int y);

//Cria uma matriz altura CAMPODEVISAO e largura do campo, com base na matriz inteira, e atribui no endereo recebido
//Iniciando no 0,0 da tela, com CAMPODEVISAO/2 para cada lado da tela
//Retorno precisa ser uma matriz[64][CAMPODEVISAO] obrigatoriamente
void atuallizaMatrizCampoDeVisao(int matrizOriginal[TERMINALCOLUNAS][TERMINALLINHAS], int enderecoDoRetorno[TERMINALCOLUNAS][CAMPODEVISAO]);


//Prototipo da função que tranforma tamanos em unidades de terminal(64x100) para unidades normalizadas.
void normaliza(float originalX, float originalY, GLfloat *retorno);

//desenhaJogador:Desenha um quadrado com a cor especificada
void desenhaJogador(Shader shaderUsado, int VAO, GLfloat x, GLfloat y, float cor[3]);

//DesenhaPeloCodigo: recebe um int e desenha algo dependo do int recebido usando alguma outra funcao de desenho
void desenhaPeloCodigo(int codigo, Shader shaderUsado, int listaVAOs[NUMVAO], GLfloat x, GLfloat y);

//Recebe um valor em coordenadas normalizadas e retorna a posicao na matriz
void deNormaliza(float originalX, float originalY, GLfloat *retorno);

//Cria o estado padrao da tela na matriz
void zeraTela(int vetorDaTela[TERMINALCOLUNAS][TERMINALLINHAS]);

//Desenha um quadrado branco
void desenhaEmBranco(Shader shaderUsado, int VAO, GLfloat x, GLfloat y);

//Desenha um quadrado cinza
void desenhaEspacoGoleiro(Shader shaderUsado, int VAO, GLfloat x, GLfloat y);

//Desenha um quadrado branco-acinzentado
void desenhaFaixaDoCampo(Shader shaderUsado, int VAO, GLfloat x, GLfloat y);

//Desenha um quadrado com a cor de fundo
void desenhaVazio(Shader shaderUsado, int VAO, GLfloat x, GLfloat y);

//Altera a posicao da bola para as cordenadas dadas
void posicionaBola(int x, int y);

//Move um time inteiro uma distancia X,Y especificada
void moveTime(int distanciaX, int distanciaY, jogador timeRecebido[TAMANHODOTIME], jogador *retorno);

#pragma endregion

int main()
{
	
#pragma region Inicializa Variaveis

	//Tamanho da janela em pixels, de acordo com o numero de colunas
	int tamanhoDaLarguraJanela = TERMINALCOLUNAS*ESCALA;

	//Tamanho da janela em pixels, de acordo com o numero de linhas
	int tamanhoDaAlturaJanela = CAMPODEVISAO*ESCALA;
	
	//Contadores
	int i, j;

	//Inicializa vetor da tela
	zeraTela(vetorDeDadosRecebido);
	zeraTela(vetorDeDadosInicial);

	//Inicializa a bola nas coredenadas especificadas;
	bola BolaPadrao = inicializaBola(32, 49);

	//Cria time 1:
	jogador Carlos = inicializaJogador(26, 40, 1), Jorge = inicializaJogador(32, 48, 1), Marcos = inicializaJogador(38, 40, 1);
	jogador time1[TAMANHODOTIME] = { Carlos,Jorge,Marcos };

	//Cria time 2:
	jogador Joao = inicializaJogador(26, 60, 2), Pedro = inicializaJogador(32, 50, 2), Lopes = inicializaJogador(38, 60, 2);
	jogador time2[TAMANHODOTIME] = {Joao,Pedro,Lopes};

	//Lista de todas as VAOs usadas no programa
	GLint listaDeEnderecosVAO[NUMVAO];

#pragma endregion

#pragma region Inicializa GLFW e GL3W

	//Inicializa glfw
	glfwInit();
	double tempoCorrido;
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
	GLFWwindow* janela = glfwCreateWindow(tamanhoDaLarguraJanela, tamanhoDaAlturaJanela, "PELESOCCER", NULL, NULL);
	glfwMakeContextCurrent(janela); //Torna a janela janela a janela atual
	
	//Inicializa gl3w
	gl3wInit();
	
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
	
	//Inicializa quadrado em x e y normalizado para servir como EBO padrão.
	quadrado Bola = inicializaQuadrado(0,0);

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
	
#pragma endregion


#pragma region Loop Principal do jogo

	while (!glfwWindowShouldClose(janela)) 
	{
		
		glfwSetTime(0.0);

		//Atualiza campo de visao
		fimDoCampoDeVisao = inicioDoCampoDeVisao + CAMPODEVISAO;
		atuallizaMatrizCampoDeVisao(vetorDeDadosRecebido, vetorDoCampoDeVisao);
		

		//Passa a janela para a função que reaje a eventos
		recebeEntrada(janela);
		
		//Recebe eventos
		glfwPollEvents();
		

		//Loop de desenho na tela
		for (i = 0; i < TERMINALCOLUNAS; i++)
		{
			for (j = 0; j <CAMPODEVISAO; j++)
			{
				//Desenha chamando funcao de desenho dependendo do valor na matriz de dados
				
				normaliza(i, j, XYnormalizado);
				desenhaPeloCodigo(vetorDoCampoDeVisao[i][j], shaderPadrao, listaDeEnderecosVAO, XYnormalizado[0],XYnormalizado[1]);
				
			}
		}

		
		//Detecta entrada
		if (glfwGetKey(janela, GLFW_KEY_DOWN) == GLFW_PRESS) {

			//Muda a posicao da bola em 1 pra baixo
			//posicionaBola(posBola[0], posBola[1] + 1);

			//Atualiza posicao do jogador
			//posicionaJogador(Carlos.x, Carlos.y + 1, Carlos, &Carlos);

			//Movimenta o time especificado
			moveTime(0, 1, time2, time2);

			if (fimDoCampoDeVisao < 100) {
				inicioDoCampoDeVisao++;
			}
		}
		if (glfwGetKey(janela, GLFW_KEY_UP) == GLFW_PRESS) {

			//Muda a posicao da bola em 1 pra cima

			//posicionaBola(posBola[0], posBola[1] - 1);
			if (inicioDoCampoDeVisao > 0) {
				inicioDoCampoDeVisao--;
			}

			moveTime(0, -1, time2, time2);
		}
		if (glfwGetKey(janela, GLFW_KEY_LEFT) == GLFW_PRESS) {

		//	posicionaBola(posBola[0] + 1, posBola[1]);
			moveTime(1, 0, time2, time2);
		}
		if (glfwGetKey(janela, GLFW_KEY_RIGHT) == GLFW_PRESS) {

			//Muda a posicao da bola em 1 pra baixo
			moveTime(-1, 0, time2, time2);
			//posicionaBola(posBola[0] - 1, posBola[1]);
		}


		//Checa framerate
		tempoCorrido = glfwGetTime();
		


		//Troca buffers
		glfwSwapBuffers(janela);

	}
#pragma endregion
	
   
}

#pragma region Funcoes

void recebeEntrada(GLFWwindow *window){
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

bola inicializaBola(int x, int y)
{
	bola retorno;
	retorno.x = x;
	retorno.y = y;

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
		//Coloca R do RBG como 1, cria jogador(time vermelho)
		corJogador[0] = 0.90f;
		desenhaJogador(shaderUsado, listaVAOs[0], x ,y,corJogador);
		break;
	case 2:
		//Coloca B do RBG como 1, cria jogador(time azul)
		corJogador[1] = 0.90f;
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
	vetorDeDadosDesenhado[(int)XYdenormalizado[0]][(int)XYdenormalizado[1]] == 0;
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

	int matrizDeRetorno[TERMINALCOLUNAS][CAMPODEVISAO];
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
	//Verifica se é possivel posicionar o jogador no local desejado
	if (vetorDeDadosRecebido[x][y] == 0 || vetorDeDadosRecebido[x][y] == 7)
	{
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

	}
}

jogador inicializaJogador(int x, int y, int Time)
{

	jogador retorno;
	retorno.x = x;
	retorno.y = y;
	retorno.time = Time;
	vetorDeDadosRecebido[x][y] = Time;
	return retorno;

}

void posicionaBola(int x, int y)
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

	}
}

void moveTime(int distanciaX, int distanciaY, jogador timeRecebido[TAMANHODOTIME], jogador *retorno)
{
	//Cria vetor de retorno
	jogador timeDeRetorno[TAMANHODOTIME];

	int i;

	//Preenche vetor de retorno com os dados atualizados pela funcao
	for (i = 0; i < TAMANHODOTIME; i++)
	{

		posicionaJogador(timeRecebido[i].x + distanciaX, timeRecebido[i].y + distanciaY, timeRecebido[i], &timeRecebido[i]);
	}


}
#pragma endregion