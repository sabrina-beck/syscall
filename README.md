# Projeto 2 - Implementação de chamadas de sistema
## Grupo 7
- Caio Cesar de Campos Silva - 160138
- Lucas Cleto de Oliveira - 156345
- Sabrina Beck Angelini - 157240

## Descrição
Implementação de uma chamada de sistema que receba pares de chave/valor com tempo de vida determinado no momento da criação. A chave é ser um parâmetro inteiro e o valor uma cadeia de caracteres. São duas chamadas de sistema gettmpkey e settmpkey para armazerar e recuperar estes pares de chave/valor. A tabela de hash implementada utiliza lista ligada como solução para colisões.

`settmpkey`:
```
/* Retorna 0 se a operação foi bem sucedida e -1 caso contrário. */
asmlinkage long sys_settmpkey(int key, char* value, unsigned int lifespan);
```

`gettmpkey`:
```
/* Copia em value o valor da chave key com no máximo n caracteres.
   Retorna 0 se encontrou chave válida e -1 caso contrário. */
asmlinkage long sys_gettmpkey(int key, int n, char* value);
```

## Apresentação
### Observações
- O timeout de uma nova chave é especificado em segundos.
- O hash code é preparado para chaves inteiras, ou seja, qualquer valor que um `int` em C pode armazenar, positivo ou negativo.
- A cada consulta/edição de uma chave é feita a expiração de uma chave antiga caso necessário.

### Casos de Teste
**Caso 1**: Expiração de chave antiga pelo `gettmpkey`
- Criar a chave `1` com valor `Ola Mundo!` e tempo de vida de 10s, o resultado deve ser de sucesso.
- Recuperar a chave `1` dentro de seu tempo de vida (tamanho do valor é 10), deve retornar o valor.
- Esperar os 10s terminarem e recuperar a chave `1`, deve retornar erro.

**Caso 2**: Tentativa de criar chave duplicada
- Criar chave `-5` com valor `bla` e tempo de vida de `60s`, deve retornar sucesso.
- Recuperar a chave `-5` dentro de seu tempo de vida (tamanho do valor é 3), deve retornar o valor.
- Em menos de 60s, criar chave `-5` com valor `Maria tinha um carneirinho` e tempo de vida de `10s`, deve falhar pois a chave já existe.

**Caso 3**: Expiraço de chave antiga pelo `settmpkey`
- Criar a chave `52` com valor `Hello Moto!` e tempo de vida de 10s, o resultado deve ser de sucesso.
- Recuperar a chave `52` dentro de seu tempo de vida (tamanho do valor é 11), deve retornar o valor.
- Depois de 10s, criar a chave `52` com valor `Hello Linus!` e tempo de vida de 40s, o resultado deve ser de sucesso.
- Recuperar a chave `52` dentro de seu novo tempo de vida (tamanho do valor é 12), deve retornar o novo valor.
