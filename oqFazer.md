# Guia de início — Projeto Codexion (42)

> Nota importante: lendo o PDF do enunciado inteiro, **não existe uma seção de bonus definida** para este projeto (ela aparece apenas nas "Common Instructions" como regra genérica de todos os projetos da 42, mas o Chapter VI — Mandatory part — não define nenhum bonus específico do Codexion). Então, a não ser que a plataforma de correção (intra/Deepthought) mostre um bonus separadamente, trate este projeto como **apenas mandatory part**.

---

## 1. Entenda o problema antes de codar

Codexion é uma variação do clássico **dining philosophers**, mas com uma pegada diferente:

- Os "coders" são os filósofos.
- Os "dongles USB" são os garfos (forks).
- Compilar = comer (precisa de 2 dongles, um de cada lado).
- Debugar e refatorar = pensar / estados intermediários sem recursos.
- Burnout = morrer de fome (starvation).
- Existe um `dongle_cooldown` (algo que o dining philosophers clássico não tem): depois de largar um dongle, ele fica indisponível por X ms.
- Existe escolha de escalonador: `fifo` ou `edf` (Earliest Deadline First) — isso também não existe no dining philosophers clássico.

Antes de escrever uma linha de código, garanta que você consegue explicar de cabeça:
- Por que dining philosophers pode gerar deadlock (as 4 condições de Coffman).
- Por que pode gerar starvation mesmo sem deadlock.
- Como o `dongle_cooldown` e o `scheduler` afetam essas duas coisas.

---

## 2. Passo a passo sugerido

### Passo 1 — Parsing dos argumentos
Implemente e teste isso primeiro, isoladamente.

```
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
           time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

X - Valide que **todos** os 8 argumentos foram passados.
X - Rejeite números negativos, zero quando não fizer sentido, não-inteiros, overflow.
X - `scheduler` precisa ser exatamente `"fifo"` ou `"edf"` (com `strcmp`) — qualquer outra coisa é erro.
X - Faça isso retornar erro limpo (sem crash, sem leak) antes de seguir.

### Passo 2 — Modelar as estruturas de dados
Sem variáveis globais, então tudo precisa viajar via `struct` passada como `void *arg` para as threads. Pense em pelo menos:

X - `t_simulation` (ou nome parecido): guarda os parâmetros, o array de coders, o array de dongles, o horário de início da simulação, mutex de log, flag de "simulação deve parar", mutex/cond para essa flag.
X - `t_coder`: id, número de compiles feitos, timestamp do último início de compile (`last_compile_start`), ponteiros para o dongle da esquerda e da direita, ponteiro de volta pra simulação, thread id.
X - `t_dongle`: mutex próprio, estado (livre/ocupado), timestamp de quando ficou livre (para o cooldown), fila de espera (para FIFO/EDF).
X - Estrutura de **heap/priority queue** própria (proibido usar qualquer priority queue de biblioteca padrão) — usada para decidir quem pega o dongle primeiro.

### Passo 3 — Dongles e mutexes
- Se `number_of_coders == 1`: só existe **um** dongle na mesa (regra explícita do enunciado) — esse coder nunca vai conseguir compilar (não tem os 2 dongles), então ele vai burnar. Isso é esperado, trate como caso válido, não como bug.
- Se houver N > 1 coders: N dongles, um entre cada par de coders vizinhos (coder 1 vizinho de coder N, formando o círculo).
- Cada dongle tem seu próprio `pthread_mutex_t`.
- Implemente `take_dongle()` e `release_dongle()`:
  - `release_dongle`: marca o timestamp de liberação; o dongle só pode ser pego de novo depois de `dongle_cooldown` ms.
  - `take_dongle`: bloqueia (via `pthread_cond_wait`/`pthread_cond_timedwait`) até o dongle estar livre **e** o cooldown ter passado **e** ser a vez do coder segundo o `scheduler`.

### Passo 4 — Evitando deadlock (as 4 condições de Coffman)
Pense em como quebrar pelo menos uma condição:
1. **Exclusão mútua** — inevitável aqui (dongle é exclusivo), não dá pra quebrar essa.
2. **Hold and wait** — evite "segurar um dongle enquanto espera o outro". Estratégias clássicas:
   - Pedir os dois dongles numa ordem consistente (ex.: sempre pega primeiro o de menor índice), quebra o ciclo circular clássico do dining philosophers.
   - Ou um coder "par" pega esquerda→direita e um coder "ímpar" pega direita→esquerda.
   - Ou usar um mutex/semáforo geral que limita quantos coders podem tentar pegar dongles ao mesmo tempo (N-1 no máximo).
3. **No preemption** — normalmente mantido (não tira dongle à força), tudo bem.
4. **Circular wait** — a estratégia de ordenação acima (passo anterior) já resolve isso.

Documente no README **qual estratégia você escolheu** — isso é pedido explicitamente.

### Passo 5 — Evitando starvation / garantindo liveness
O enunciado exige explicitamente: "no coder should be starved of dongles and burn out under edf scheduling, provided the parameters are feasible."

- Com `edf`: sempre que múltiplos coders esperam o mesmo dongle, o dongle deve ir para quem tem o **deadline mais próximo** (`last_compile_start + time_to_burnout` mais cedo). Implemente o heap comparando por esse valor.
- Trate empates (tie-breaker) de forma determinística — por exemplo, em caso de deadlines iguais, desempate pelo menor id de coder ou por ordem de chegada do request. O enunciado pede explicitamente uma regra de desempate.
- Com `fifo`: simplesmente respeite ordem de chegada do pedido — uma fila.
- Cuidado: um scheduler "ganancioso" (dar sempre pro mesmo coder) pode gerar starvation dos outros. Teste com muitos coders competindo.

### Passo 6 — Ciclo de vida do coder (thread)
Pseudocódigo do loop de cada thread coder:

```
inicializar last_compile_start = tempo de início da simulação
enquanto simulação não deve parar:
    pedir dongle esquerdo  -> log "has taken a dongle"
    pedir dongle direito   -> log "has taken a dongle"
    last_compile_start = agora
    log "is compiling"
    usleep(time_to_compile * 1000)
    soltar os dois dongles (dispara cooldown)
    incrementar contador de compiles
    log "is debugging"
    usleep(time_to_debug * 1000)
    log "is refactoring"
    usleep(time_to_refactor * 1000)
    // volta pro topo do loop e tenta compilar de novo imediatamente
```

Pontos de atenção:
- As duas mensagens "has taken a dongle" (uma por dongle) devem sair **antes** de "is compiling".
- Nunca deixe o coder "compilando", "debugando" e "refatorando" ao mesmo tempo — são estados mutuamente exclusivos (o enunciado insiste nisso).
- `usleep` de forma precisa: prefira medir com `gettimeofday`/`clock_gettime` em vez de confiar cegamente no `usleep` (que pode dormir um pouco mais que o pedido).

### Passo 7 — Thread monitora (monitor) e detecção de burnout
- Uma thread separada, dedicada, que fica checando periodicamente (ex.: a cada 1 ms ou menos) se algum coder passou de `last_compile_start + time_to_burnout` sem começar a compilar de novo.
- Precisão exigida: o log de burnout deve sair em até **10 ms** do burnout real — então o intervalo de polling do monitor precisa ser bem menor que isso (ex.: 1 ms).
- Quando detectar burnout: logar, sinalizar para todas as threads pararem (flag protegida por mutex/cond), e então dar `pthread_join` em todas.
- Também é o monitor (ou uma lógica equivalente) que verifica se **todos** os coders já compilaram `number_of_compiles_required` vezes, e nesse caso encerra a simulação normalmente (sem burnout).

### Passo 8 — Logging serializado
- Um único mutex de log (`log_mutex`) protegendo todo `printf`/`write`.
- Formato exato, sempre `timestamp_in_ms X mensagem`.
- Calcule o timestamp relativo ao início da simulação (ms desde o `t0`), não o horário absoluto do sistema.
- Cuidado extra: depois que a simulação "deve parar" (burnout ou fim), evite logar mensagens de outros coders que ainda estavam no meio do loop — verifique a flag de parada antes de logar, dentro da região protegida.

### Passo 9 — Encerramento limpo
- `pthread_join` de todas as threads coder + a thread monitor.
- Destruir todos os mutexes e condition variables (`pthread_mutex_destroy`, `pthread_cond_destroy`).
- `free` de tudo que foi alocado (rode com `valgrind --leak-check=full` frequentemente, não só no fim).
- Rode também com um sanitizer de threads se puder (ex. `-fsanitize=thread`, ainda que não seja pedido explicitamente, ajuda MUITO a pegar race conditions antes da defesa) — só lembre de tirar antes de submeter se o Makefile final não permitir flags extras.

### Passo 10 — Makefile
Regras obrigatórias: `$(NAME)`, `all`, `clean`, `fclean`, `re`.
- Compile com `cc -Wall -Wextra -Werror -pthread`.
- Sem relink desnecessário (dependências de `.o` corretas, não recompile tudo à toa).
- Nome do binário: `codexion`.
- Não existe seção de bonus explícita no subject — então, salvo indicação contrária da equipe de avaliação, você não precisa da regra `bonus` nem de `*_bonus.c/h`.

### Passo 11 — README.md (obrigatório, em inglês)
Estrutura mínima exigida:
1. Primeira linha, em itálico: `*This project has been created as part of the 42 curriculum by <login1>[, <login2>...]*`
2. `## Description` — o que é o projeto, objetivo, visão geral.
3. `## Instructions` — como compilar/rodar (`make`, `./codexion ...`).
4. `## Resources` — referências clássicas (docs de pthreads, artigos sobre dining philosophers, Coffman conditions) **e** uma descrição de como você usou IA — para quais tarefas, em quais partes.
5. `## Blocking cases handled` (específico deste projeto) — deadlock (Coffman), starvation, cooldown, precisão de detecção de burnout, serialização de logs.
6. `## Thread synchronization mechanisms` (específico deste projeto) — quais mutexes/cond vars você usou, para o quê, com exemplos de como evitou race conditions e como coders/monitor se comunicam.
- Todo o README precisa estar em inglês.

### Passo 12 — Testes antes da defesa
Cenários que você deve testar manualmente:
- 1 coder → deve sempre burnar (só existe 1 dongle).
- 2 coders → caso simples, bom pra validar lógica básica.
- Número ímpar grande de coders (ex.: 5, 7) → clássico caso de deadlock do dining philosophers se você não tratou hold-and-wait/circular-wait direito.
- `time_to_burnout` bem apertado vs `time_to_compile + time_to_debug + time_to_refactor` → testa se seu scheduler realmente evita burnout quando é tecnicamente possível.
- `dongle_cooldown` alto → verifica se você realmente respeita o tempo de cooldown e não deixa reuso antes da hora.
- `fifo` vs `edf` com muitos coders competindo pelo mesmo par de dongles → compare os padrões de log.
- Argumentos inválidos (negativos, letras, scheduler errado) → deve rejeitar sem crash.
- Valgrind em cada cenário acima.
- Cronometrar manualmente (com o log) se o "burned out" saiu dentro de 10 ms do horário esperado.

### Passo 13 — Preparo pra defesa / recode
- Consiga explicar, sem olhar o código, por que sua solução não deadlocka e não gera starvation.
- Tenha pronto um "modo mental" de fazer pequenas mudanças ao vivo (ex.: adicionar um novo estado de log, mudar um parâmetro, adaptar a estrutura de dados) — é exatamente o que o "Recode instructions" do subject avisa que pode acontecer.
- Releia a seção de "AI Instructions" (Capítulo II do PDF): você precisa conseguir justificar e explicar **qualquer** trecho gerado com IA, senão zera o projeto na avaliação.

---

## Resumo rápido (ordem de execução)
1. Parsing + validação de argumentos
2. Structs (sem globais)
3. Heap/priority queue própria (fifo/edf)
4. Lógica de dongles com mutex + cooldown
5. Estratégia anti-deadlock (ordem de aquisição ou similar)
6. Loop do coder (compile → debug → refactor)
7. Thread monitor (burnout + condição de parada por compiles)
8. Logging serializado
9. Encerramento limpo (join + destroy + free)
10. Makefile
11. README.md
12. Testes + valgrind
13. Preparo de defesa
