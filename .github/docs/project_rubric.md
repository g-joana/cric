# Critérios de Avaliação - ft_irc

## 1. Verificações Básicas (Mandatório)
* **Estrutura e Compilação**: Deve existir um Makefile e o projeto deve compilar corretamente com as opções exigidas, sendo escrito em C++ com o nome do executável conforme esperado.
* **Multiplexação (poll)**:
    * Deve haver apenas **um** `poll()` (ou equivalente) em todo o código.
    * O `poll()` deve ser chamado antes de cada operação de `accept`, `read/recv` e `write/send`.
    * O `errno` não deve ser usado para disparar ações específicas após essas chamadas (ex: tentar ler novamente após um `EAGAIN`).
* **Uso do fcntl**: Qualquer chamada para `fcntl()` deve ser feita estritamente no formato `fcntl(fd, F_SETFL, O_NONBLOCK);`. Outros usos são proibidos.
* **Nota Crítica**: Se qualquer um destes pontos básicos falhar, a avaliação é encerrada com nota 0.

## 2. Requisitos de Rede e Conectividade
* **Escuta do Servidor**: O servidor deve iniciar e escutar em todas as interfaces de rede na porta fornecida via linha de comando.
* **Comunicação com nc**: Deve ser possível conectar ao servidor usando a ferramenta `nc`, enviar comandos e receber respostas do servidor.
* **Cliente de Referência**: O grupo deve informar um cliente IRC de referência e ser capaz de se conectar ao servidor através dele.
* **Conexões Simultâneas**: O servidor deve lidar com múltiplas conexões ao mesmo tempo sem bloquear, respondendo a todas as demandas simultaneamente (testado com o cliente IRC e `nc` ao mesmo tempo).
* **Encaminhamento de Mensagens**: Ao entrar em um canal, as mensagens enviadas por um cliente devem ser transmitidas para todos os outros clientes que ingressaram no mesmo canal.

## 3. Situações Especiais de Rede
* **Comandos Parciais**: O servidor deve lidar corretamente com comandos enviados de forma parcial via `nc` (reconstruindo o comando) sem afetar outras conexões.
* **Quedas Inesperadas**:
    * Se um cliente for encerrado abruptamente (kill), o servidor deve continuar operacional para os outros clientes e aceitar novas conexões.
    * Se o `nc` for encerrado com apenas metade de um comando enviado, o servidor não deve ficar bloqueado ou em estado inconsistente.
* **Flood e Suspensão**: Ao suspender um cliente (`Ctrl-Z`) em um canal e inundar (flood) o canal com outro cliente, o servidor não deve travar. Quando o cliente suspenso retornar, os comandos acumulados devem ser processados normalmente.
* **Vazamento de Memória**: O servidor deve ser verificado quanto a vazamentos de memória (leaks) durante operações de carga/flood.

## 4. Comandos do Cliente
* **Funcionalidades Básicas**: Deve ser possível autenticar, definir um *nickname*, um *username* e entrar em um canal através do cliente de referência e do `nc`.
* **Mensagens Privadas (PRIVMSG)**: Devem estar totalmente funcionais com diferentes parâmetros.
* **Operadores de Canal**: 
    * Usuários regulares não devem ter privilégios para realizar ações de operador.
    * Todos os comandos de operação de canal (KICK, INVITE, TOPIC, MODE) devem ser testados e funcionar corretamente para operadores.

## 5. Parte Bônus
*Apenas avaliada se a parte mandatória estiver perfeita.*
* **Transferência de Arquivos**: Deve funcionar com o cliente IRC de referência.
* **Bot**: Deve existir um bot IRC funcional.

---
**Observações de Defesa**:
* Não são permitidos *segmentation faults* ou encerramentos inesperados do programa durante a defesa.
* Toda memória alocada na heap deve ser devidamente liberada antes do fim da execução.