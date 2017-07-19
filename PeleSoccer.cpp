#include "Biblioteca.h"

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
//9º:Implementar tabela de high scores: 100%

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


int main()
{
	#pragma region Menu
	char recebemenu = 0;
	int tempoDeJogo = 60;
	char formacao[64];
	char nomeDoJogador1[17], nomeDoJogador2[17];
	do
	{
		system("cls");
		printf("1-Jogar\n2-Opcoes\n3-Highscores\n4-Sair\n");
		switch (recebemenu)
		{
		case '2':
			//escreveArquivoDePontos(nome1, nome2, placarteste);
			//getchar();
			break;
		case '3':
			
			leArquivoDePontos();
			getchar();
			break;
		case '4':
			//Unico ponto de retorno da função, sai do jogo
			return 0;
			break;
		}



	} while ((recebemenu = getc(stdin)) != '1');

	fputs("Nome do jogador nº 1:", stdout);
	scanf("%s", nomeDoJogador1);

	fputs("Nome do jogador nº 2:", stdout);
	scanf("%s", nomeDoJogador2);

	fputs("local do arquivo de formacao:", stdout);
	scanf("%s", &formacao);
	

#pragma endregion
	
	#pragma region Inicializa Variaveis



	//Guarda quais teclas de movimento foram pressionadas pra movimentacao em diagonal
	//0 direita(X+), 1 esquerda(X-), 2 BAIXO(Y+), 3 cima(Y-);
	bool vetorDeSetas[4] = { 0,0,0,0 };
	bool vetorDeSetas2[4] = { 0,0,0,0 };
	
	//A tela recebeu um comando de limpesa?
	int limpoRecentemente = 0;

	//A cada quantos "turnos" a bola perde um turno
	limitadorDeVelocidadeBola = 3;

	//Duracao da partida
	int tempoMaxDeJogo = 60;

	//Conta numero de frames até o momento
	int contadordeframes = 0;

	//Tamanho da janela em pixels, de acordo com o numero de colunas
	int tamanhoDaLarguraJanela = TERMINALCOLUNAS*ESCALA;

	//Tamanho da janela em pixels, de acordo com o numero de linhas
	int tamanhoDaAlturaJanela = CAMPODEVISAO*ESCALA;
	
	//Inicializa quadrado em x e y normalizado para servir como EBO padrão.
	quadrado Bola = inicializaQuadrado(0, 0);

	//Contadores
	int i, j;
	
	//Tempo atual desde o inicio do jogo

	
	//Inicializa vetor da tela
	zeraTela(vetorDeDadosRecebido);
	zeraTela(vetorDeDadosInicial);


	//Inicializa a bola nas coredenadas especificadas;
	bola bolaPadrao = inicializaBola(32, 49,0,0);

	jogador *time2 = leFormacao(formacao,2);
	
	jogador *time1 = leFormacao(formacao, 1);
	
	//Cria time 1:
	


	
	
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
		printf("Time Azul(%s) %i - %i Time Vermelho(%s)\n",nomeDoJogador2, placar[1], placar[0],nomeDoJogador1);
		if (tempoMaxDeJogo - tempoDeJogo < 0)
		{
			if (placar[0] == placar[1])
			{
				printf("FIM DE JOGO!\nEMPATE!\n");
				escreveArquivoDePontos(nomeDoJogador1, nomeDoJogador2, placar);
			}

			if (placar[0] > placar[1])
			{
				printf("FIM DE JOGO!\nVITORIA DE %s!\n",nomeDoJogador1);
				escreveArquivoDePontos(nomeDoJogador1, nomeDoJogador2, placar);
			}
			if (placar[0] < placar[1])
			{
				printf("FIM DE JOGO!\nVITORIA DE %s!\n", nomeDoJogador2);
				escreveArquivoDePontos(nomeDoJogador1, nomeDoJogador2, placar);
			}
			printf("\n Pressione x para sair\n");
			while (getchar() != 'x')
			{
			}
			
			glfwSetWindowShouldClose(janela, true);

		}

#pragma endregion		 
		

	}

#pragma endregion

}

