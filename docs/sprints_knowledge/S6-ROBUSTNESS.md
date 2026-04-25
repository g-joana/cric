# S6 - Robustez & Entrega Final

## Decisões de Implementação

### S6-T1: SIGINT Handler
- Usado ponteiro global `g_server` para acesso ao Server do signal handler
- Função `_cleanupAndExit()` fecha todos os fds e deleta todos os clients/channels
- Handler chama `exit(0)` após cleanup para terminar进程

### S6-T2: Memory Management
- Destructor ~Server já limpar todos os clients e channels
- Cada cliente disconnectado deleta objeto Client no _removeClient
- Channels deletados automaticamente quandoMemberCount == 0

### S6-T3: Flood Handling
- poll() non-blocking mode permite handling de muitas conexões
- Non-blocking I/O evita blocking em flood scenarios

### S6-T4: IRC RFC 1459 Compliance
- 001 Welcome message format
- MODE +i/-i, +t/-t, +k/-k, +o/-o, +l/-l
- KICK, INVITE, TOPIC handlers

## Edge Cases Tratedos

1. **MODE +i**: Bloqueia join se não inviteado (473 ERR_INVITEONLYCHAN)
2. **MODE +k**: RequSenha correta ou ERR_BADCHANNELKEY (475)
3. **MODE +l**: Bloqueia se no limite (ERR_CHANNELISFULL - 471)
4. **MODE +t**: Não-Operator não pode mudar topic (ERR_CHANOPRIVSNEEDED - 482)
5. **KICK**: Apenas operators podem kickar (482)

## Correções S5

- **Bug Fix**: MODE +i não era verificado no JOIN
- Correção: Added checks em _handleJOIN para +i, +k, +l modes antes de addMember

## Lições Aprendidas

1. poll() precisa ser non-blocking para server responsivo
2. Every client disconnect precisa ser limpo corretamente
3. Signal handlers precisam ser simples - apenas set flag ou cleanup rápido