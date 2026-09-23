# Projeto do jogo: Fuga da Prisão

## 1. Descrição do Sistema

No jogo fuga da prisão, o jogador tem como missão escapar de uma prisão representada por um labirinto. Para conseguir fugir, o jogador deverá explorar o mapa em busca das chaves necessárias para abrir a porta da prisão, enquanto evita os guardas responsáveis por patrulhar o local. Caso o jogador seja encontrado por um guarda ou atinja o nível máximo de suspeita, a partida será encerrada com derrota, então seja agil e cauteloso ou fuja antes que eles te pegem!            

Durante a partida, o jogador poderá se movimentar pelo mapa e interagir com elementos presentes no cenário. Algumas ações poderão aumentar o nível de suspeita, exigindo que o jogador tome cuidado com certas interações para conseguir avançar pelas fases sem ser capturado. Ao concluir a fase, a pontuação do jogador será calculada e poderá ser registrada no sistema de recordes.

Este jogo foi criado com o desígnio de explorar e aumentar as acuidades dos educandos na Unicesumar do curso de Engenharia de Software. Voltado a matéria "Linguagens e técnicas de programação" e intruido pelo Professor Dácio Machado, nosso projeto foi desenvolvido na linguagem C e criado para rodar em terminal.

## 2. Fluxo de Utilização Esperado para o Sistema

`1. Novo Jogo`<br>
`2. Carregar Jogo`<br>
`3. Recordes`<br>
`4. Como Jogar`<br>
`5. Sair`<br>

`Escolha uma opção:`<br>

**1. Novo Jogo:** inicia uma nova partida, colocando o jogador na primeira fase da prisão e apresentando o mapa, os guardas, as chaves e a porta de saída.

**2. Carregar Jogo:** verifica se existe um arquivo de salvamento. Se existir, recupera os dados da partida (fase, pontuação, posição, suspeita, chaves e guardas) e exibe o mapa da fase para o jogador continuar de onde parou. Se não existir, inicia uma nova partida na fase 1.

**3. Recordes:** apresenta as maiores pontuações obtidas nas partidas anteriores, realizando a leitura dos dados armazenados no arquivo de recordes.

**4. Como Jogar:** apresenta as instruções do jogo, incluindo os controles, objetivo da partida e regras relacionadas às chaves, porta, guardas, suspeita e pontuação.

**5. Sair:** encerra o programa. O sistema poderá solicitar uma confirmação antes de fechar o jogo.

Após a execução de qualquer opção que não seja `5. Sair`, o sistema poderá retornar ao menu inicial, permitindo que o jogador escolha outra funcionalidade.

## 3. Fluxograma da Lógica do Sistema

![Fluxograma Principal](fluxograma_fundo_branco.png)

## 4. Estrutura de Dados

