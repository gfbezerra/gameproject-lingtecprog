# Projeto do jogo: Fuga da Prisão

## 1. Descrição do Sistema

No jogo fuga da prisão, o jogador tem como missão escapar de uma prisão representada por um labirinto. Para conseguir fugir, o jogador deverá explorar o mapa em busca das chaves necessárias para abrir a porta da prisão, enquanto evita os guardas responsáveis por patrulhar o local. Caso o jogador seja encontrado por um guarda ou atinja o nível máximo de suspeita, a partida será encerrada com derrota, então seja agil e cauteloso ou fuja antes que eles te pegem!            

Durante a partida, o jogador poderá se movimentar pelo mapa e interagir com elementos presentes no cenário. Algumas ações poderão aumentar o nível de suspeita, exigindo que o jogador tome cuidado com certas interações para conseguir avançar pelas fases sem ser capturado. Ao concluir a fase, a pontuação do jogador será calculada e poderá ser registrada no sistema de recordes.

Este jogo foi criado com o desígnio de explorar e aumentar as acuidades dos educandos na Unicesumar do curso de Engenharia de Software. Voltado a matéria "Linguagens e técnicas de programação" e intruido pelo Professor Dácio Machado, nosso projeto foi desenvolvido na linguagem C e criado para rodar em terminal.

## 2. Fluxo de Utilização Esperado para o Sistema

Ao iniciar o programa, o jogador visualizará o menu inicial, que será responsável por apresentar as principais opções de interação com o sistema:

`1. Novo Jogo`    
`2. Recordes`  
`3. Como Jogar`  
`4. Sair`

`Escolha uma opção:`

**1. Novo Jogo:** inicia uma nova partida, colocando o jogador na primeira fase da prisão e apresentando o mapa, os guardas, as chaves e a porta de saída.

**2. Recordes:** apresenta as maiores pontuações obtidas nas partidas anteriores, realizando a leitura dos dados armazenados no arquivo de recordes.

**3. Como Jogar:** apresenta as instruções do jogo, incluindo os controles, objetivo da partida e regras relacionadas às chaves, porta, guardas, suspeita e pontuação.

**4. Sair:** encerra o programa. O sistema poderá solicitar uma confirmação antes de fechar o jogo.

Após a execução de qualquer opção que não seja `4. Sair`, o sistema poderá retornar ao menu inicial, permitindo que o jogador escolha outra funcionalidade.

## 3. Fluxograma da Lógica do Sistema

## 4. Estrutura de Dados

