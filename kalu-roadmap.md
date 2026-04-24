# Roadmap Inicial (Kalu)

## Backlog de Tarefas Faltantes

1. Gestão de Memória e Recursos (Cleanup)
    - [x] Remover do Map: No Server::run(), ao detectar desconexão (bytesReads <= 0), remover a entrada correspondente no std::map<int, Client*> _clients.
    - [x] Deletar Objeto: Chamar delete para o ponteiro do objeto Client para liberar a memória alocada no accept.
    - [x] Ajustar Loop do Poll: Garantir que, ao remover um elemento do vector<pollfd>, o índice i seja decrementado corretamente para não pular o próximo FD na lista.

2. Refinamento da Classe Client
    - [x] Corrigir setNickname: Mudar a assinatura para void ou garantir que ela realmente altere o atributo _nickname (atualmente o código está com /*todo*/).
    - [x] Implementar isRegistered: Mudar de std::string para bool (ou manter string, mas garantir a lógica de alteração para "true" após o handshake inicial).
    - [x] Atributos de Usuário: Adicionar campos para username, realname e hostname.

3. Parsing e Gerenciamento de Buffer
    - [x] Acúmulo de Dados: Ativar c->appendToBuffer(buffer) dentro do loop do servidor.
    - [x] Busca por Delimitadores: Criar função para procurar \n ou \r\n dentro da _buffer do cliente.
    - [x] BUG: cenario com mais de um cliente. se um deles digita ctrl + d, server bloqueia os outros clients. 
    - [ ] Extração de Comando: Criar lógica para:
        - Extrair a string até o primeiro \n.
        - Tratar essa string como um comando completo.
        - Manter o que sobrar na _buffer do cliente (dados do próximo comando).
    - [ ] Split de Parâmetros: Criar função para separar a linha em COMMAND, PARAMS e TRAILING (a parte que começa com :).

4. Comandos de Registro (Autenticação Inicial)
    - [ ] Comando PASS:
        - Verificar se a senha enviada bate com a senha do servidor.
        - Impedir qualquer outro comando se a senha estiver errada ou não tiver sido enviada.
    - [ ] Comando NICK:
        - Validar caracteres permitidos.
        - Verificar se o nickname já está em uso por outro cliente no mapa.
    - [ ] Comando USER:
        - Capturar username e realname.
    - [ ] Envio de RPL_WELCOME (001):
        - Criar função sendResponse para enviar mensagens formatadas ao cliente.
        - Enviar a mensagem numérica 001 após PASS, NICK e USER serem validados.

5. Lógica de Mensageria (PRIVMSG)
    - [ ] Busca de Destinatário: Implementar busca no mapa de clientes pelo nickname (e não apenas pelo FD).
    - [ ] Mensagem Privada (Usuário para Usuário): Encaminhar o texto para o FD do destino.
    - [ ] Tratar Erros: Enviar ERR_NOSUCHNICK se o destinatário não existir.

6. Gestão de Canais (Canais e Operadores)
    - [ ] Classe Channel: Criar classe para armazenar:
        - Nome do canal.
        - Lista de FDs dos membros.
        - Lista de FDs dos operadores.
        - Senha do canal (se houver).

    - [ ] Comando JOIN:
        - Adicionar cliente à lista do canal.
        - Enviar a mensagem para todos os membros avisando da entrada.

    - [ ] Comando PART: Remover cliente do canal.
    - [ ] Comando TOPIC: Ver ou alterar o tópico do canal.

7. Comandos de Moderação (Obrigatórios)
    - [ ] Comando KICK: Expulsar usuário (apenas se quem enviou for operador).
    - [ ] Comando INVITE: Convidar para canal modo +i.
    - [ ] Comando MODE: Implementar flags:

        - i: Canal apenas para convidados.
        - t: Tópico restrito a operadores.
        - k: Definir/remover senha do canal.
        - o: Dar/retirar privilégio de operador.
        - l: Definir limite de usuários.

8. Finalização e Robustez
    - [ ] Sinais do Sistema: Tratar SIGINT (Ctrl+C no servidor) para fechar todos os sockets e limpar a memória antes de sair.
    - [ ] Leaks de Memória: Rodar com valgrind para garantir que nenhuma desconexão deixe rastros na RAM.

---

## Comandos Implementados

 Autenticação/Registro
  - `PASS` enviar senha do servidor
  - `NICK` definir nickname
  - `USER` definir username

  Mensagens
  - `PRIVMSG` mensagens privadas e para canais

  Canais
  - `JOIN` entrar em um canal
  - `PART` sair de um canal
  - `QUIT` desconectar do servidor

  Comandos de operador de canal
  - `KICK` expulsar usuário do canal
  - `INVITE` convidar usuário para canal
  - `TOPIC` ver/mudar tópico do canal
  - `MODE` mudar modo do canal (i, t, k, o, l)
