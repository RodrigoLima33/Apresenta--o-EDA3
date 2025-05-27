#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define ORDEM 3
#define TAM_CODIGO 10
#define MAX_USUARIOS 100

typedef struct
{
    int id;
    float mediaMensal;
    char localizacaoAtual[50];
} Usuario;

Usuario usuarios[MAX_USUARIOS];
int totalUsuarios = 0;

typedef struct
{
    char codigo[TAM_CODIGO];
    int idUsuario;
    float valor;
    char local[50];
    char horario[6];
} Transacao;

typedef struct
{
    char codigo[TAM_CODIGO];
    long posicao;
} Indice;

typedef struct NoB
{
    int n;
    Indice chaves[ORDEM - 1];
    struct NoB *filhos[ORDEM];
    bool folha;
} NoB;

NoB *raiz = NULL;

NoB *criarNo(bool folha)
{
    NoB *novo = (NoB *)malloc(sizeof(NoB));
    novo->folha = folha;
    novo->n = 0;
    for (int i = 0; i < ORDEM; i++)
        novo->filhos[i] = NULL;
    return novo;
}

void dividirFilho(NoB *x, int i)
{
    NoB *y = x->filhos[i];
    NoB *z = criarNo(y->folha);
    z->n = ORDEM / 2 - 1;

    for (int j = 0; j < ORDEM / 2 - 1; j++)
        z->chaves[j] = y->chaves[j + ORDEM / 2];

    if (!y->folha)
        for (int j = 0; j < ORDEM / 2; j++)
            z->filhos[j] = y->filhos[j + ORDEM / 2];

    y->n = ORDEM / 2 - 1;

    for (int j = x->n; j >= i + 1; j--)
        x->filhos[j + 1] = x->filhos[j];

    x->filhos[i + 1] = z;

    for (int j = x->n - 1; j >= i; j--)
        x->chaves[j + 1] = x->chaves[j];

    x->chaves[i] = y->chaves[ORDEM / 2 - 1];
    x->n++;
}

void inserirNaoCheio(NoB *x, Indice k)
{
    int i = x->n - 1;
    if (x->folha)
    {
        while (i >= 0 && strcmp(k.codigo, x->chaves[i].codigo) < 0)
        {
            x->chaves[i + 1] = x->chaves[i];
            i--;
        }
        x->chaves[i + 1] = k;
        x->n++;
    }
    else
    {
        while (i >= 0 && strcmp(k.codigo, x->chaves[i].codigo) < 0)
            i--;
        i++;
        if (x->filhos[i]->n == ORDEM - 1)
        {
            dividirFilho(x, i);
            if (strcmp(k.codigo, x->chaves[i].codigo) > 0)
                i++;
        }
        inserirNaoCheio(x->filhos[i], k);
    }
}

void inserirIndice(Indice k)
{
    if (raiz == NULL)
    {
        raiz = criarNo(true);
        raiz->chaves[0] = k;
        raiz->n = 1;
    }
    else
    {
        if (raiz->n == ORDEM - 1)
        {
            NoB *s = criarNo(false);
            s->filhos[0] = raiz;
            dividirFilho(s, 0);
            int i = 0;
            if (strcmp(k.codigo, s->chaves[0].codigo) > 0)
                i++;
            inserirNaoCheio(s->filhos[i], k);
            raiz = s;
        }
        else
        {
            inserirNaoCheio(raiz, k);
        }
    }
}

long buscarIndice(NoB *x, char *codigo)
{
    int i = 0;
    while (i < x->n && strcmp(codigo, x->chaves[i].codigo) > 0)
        i++;
    if (i < x->n && strcmp(codigo, x->chaves[i].codigo) == 0)
        return x->chaves[i].posicao;
    if (x->folha)
        return -1;
    return buscarIndice(x->filhos[i], codigo);
}

// Agora usamos array de usuários inicializado para média:
float calcularMediaUsuario(int id)
{
    for (int i = 0; i < totalUsuarios; i++)
    {
        if (usuarios[i].id == id)
            return usuarios[i].mediaMensal;
    }
    return 0;
}

bool transacaoEhDuplicada(Transacao *nova)
{
    FILE *f = fopen("transacoes.txt", "r");
    if (!f)
        return false;
    char linha[200];
    Transacao t;
    while (fgets(linha, sizeof(linha), f))
    {
        sscanf(linha, "%[^|]|%d|%f|%[^|]|%[^|\n]", t.codigo, &t.idUsuario, &t.valor, t.local, t.horario);
        if (t.idUsuario == nova->idUsuario &&
            t.valor == nova->valor &&
            strcmp(t.local, nova->local) == 0 &&
            strcmp(t.horario, nova->horario) == 0)
        {
            fclose(f);
            return true;
        }
    }
    fclose(f);
    return false;
}

// Verifica se usuário já tem transação com local diferente, usando array usuarios
bool localDiferente(int id, const char *localAtual)
{
    for (int i = 0; i < totalUsuarios; i++)
    {
        if (usuarios[i].id == id)
        {
            if (strcmp(usuarios[i].localizacaoAtual, localAtual) != 0)
                return true;
            else
                return false;
        }
    }
    return false;
}

bool horarioIncomum(const char *horario)
{
    int h, m;
    sscanf(horario, "%d:%d", &h, &m);
    return h < 6 || h > 22;
}

bool transacaoEhSuspeita(Transacao *t)
{
    if (t->valor > 3 * calcularMediaUsuario(t->idUsuario))
        return true;
    if (localDiferente(t->idUsuario, t->local))
        return true;
    if (strcmp(t->local, "internacional") == 0 && t->valor < 1000)
        return true;
    if (horarioIncomum(t->horario))
        return true;
    if (transacaoEhDuplicada(t))
        return true;
    return false;
}

void salvarIndice(const char *codigo, long posicao)
{
    FILE *f = fopen("indices.txt", "a");
    if (f)
    {
        fprintf(f, "%s %ld\n", codigo, posicao);
        fclose(f);
    }
    Indice idx;
    strcpy(idx.codigo, codigo);
    idx.posicao = posicao;
    inserirIndice(idx);
}

void carregarIndices()
{
    FILE *f = fopen("indices.txt", "r");
    if (f)
    {
        Indice idx;
        while (fscanf(f, "%s %ld", idx.codigo, &idx.posicao) != EOF)
            inserirIndice(idx);
        fclose(f);
    }
}

void inserirTransacao()
{
    Transacao t;
    static int contador = 1;
    char codigo[TAM_CODIGO];
    sprintf(codigo, "TX%03d", contador++);

    printf("ID do Usuario: ");
    scanf("%d", &t.idUsuario);
    printf("Valor: ");
    scanf("%f", &t.valor);
    printf("Localizacao: ");
    scanf("%49s", t.local);
    printf("Horario (HH:MM): ");
    scanf("%5s", t.horario);

    if (transacaoEhSuspeita(&t))
        printf(">> Transacao %s eh SUSPEITA\n", codigo);
    else
        printf("Transacao %s esta OK\n", codigo);

    FILE *f = fopen("transacoes.txt", "a+");
    fseek(f, 0, SEEK_END);
    long posicao = ftell(f);
    fprintf(f, "%s|%d|%.2f|%s|%s\n", codigo, t.idUsuario, t.valor, t.local, t.horario);
    fclose(f);
    salvarIndice(codigo, posicao);
    printf("Transacao salva.\n");
}

void buscarTransacao()
{
    char codigo[TAM_CODIGO];
    printf("Codigo da transacao: ");
    scanf("%s", codigo);
    long pos = buscarIndice(raiz, codigo);
    if (pos == -1)
    {
        printf("Transacao nao encontrada.\n");
        return;
    }
    FILE *f = fopen("transacoes.txt", "r");
    fseek(f, pos, SEEK_SET);
    char linha[200];
    fgets(linha, sizeof(linha), f);
    fclose(f);
    printf(">> %s", linha);
}

void listarTransacoes()
{
    FILE *f = fopen("transacoes.txt", "r");
    if (!f)
        return;
    char linha[200];
    printf("\nTransacoes:\n");
    while (fgets(linha, sizeof(linha), f))
        printf("%s", linha);
    fclose(f);
}

void modificarTransacao()
{
    char codigo[TAM_CODIGO];
    printf("Codigo da transacao a modificar: ");
    scanf("%s", codigo);
    long pos = buscarIndice(raiz, codigo);
    if (pos == -1)
    {
        printf("Transacao nao encontrada.\n");
        return;
    }

    FILE *f = fopen("transacoes.txt", "r");
    if (!f)
    {
        printf("Erro ao abrir o arquivo de transacoes.\n");
        return;
    }

    char linha[200];
    Transacao transacoes[1000];
    char codigos[1000][TAM_CODIGO];
    int count = 0;

    while (fgets(linha, sizeof(linha), f))
    {
        sscanf(linha, "%[^|]|%d|%f|%[^|]|%s", codigos[count], &transacoes[count].idUsuario,
               &transacoes[count].valor, transacoes[count].local, transacoes[count].horario);
        count++;
    }
    fclose(f);

    int i;
    for (i = 0; i < count; i++)
    {
        if (strcmp(codigos[i], codigo) == 0)
            break;
    }

    if (i == count)
    {
        printf("Transacao nao encontrada na lista.\n");
        return;
    }

    printf("Novo ID do Usuario: ");
    scanf("%d", &transacoes[i].idUsuario);
    printf("Novo Valor: ");
    scanf("%f", &transacoes[i].valor);
    printf("Nova Localizacao: ");
    scanf("%49s", transacoes[i].local);
    printf("Novo Horario (HH:MM): ");
    scanf("%5s", transacoes[i].horario);

    f = fopen("transacoes.txt", "w");
    if (!f)
    {
        printf("Erro ao reabrir o arquivo de transacoes.\n");
        return;
    }

    long novaPosicao;
    for (int j = 0; j < count; j++)
    {
        long posAtual = ftell(f);
        fprintf(f, "%s|%d|%.2f|%s|%s\n", codigos[j],
                transacoes[j].idUsuario, transacoes[j].valor,
                transacoes[j].local, transacoes[j].horario);
        if (j == i)
            novaPosicao = posAtual;
    }
    fclose(f);

    FILE *fi = fopen("indices.txt", "w");
    if (!fi)
    {
        printf("Erro ao atualizar arquivo de indices.\n");
        return;
    }
    for (int j = 0; j < count; j++)
    {
        fprintf(fi, "%s %ld\n", codigos[j], j * 50L); // Exemplo: atualizar posição arbitrária
    }
    fclose(fi);

    // Recarregar indices na arvore (simplificação):
    raiz = NULL;
    carregarIndices();

    printf("Transacao modificada com sucesso.\n");
}

void inicializarUsuarios()
{
    usuarios[0].id = 1;
    usuarios[0].mediaMensal = 500;
    strcpy(usuarios[0].localizacaoAtual, "Manaus");

    usuarios[1].id = 2;
    usuarios[1].mediaMensal = 1500;
    strcpy(usuarios[1].localizacaoAtual, "Amapa");

    usuarios[2].id = 3;
    usuarios[2].mediaMensal = 2000;
    strcpy(usuarios[2].localizacaoAtual, "internacional");

    totalUsuarios = 3;
}

int main()
{
    inicializarUsuarios();
    carregarIndices();

    int opcao;
    do
    {
        printf("\nMenu:\n1-Inserir\n2-Buscar\n3-Listar\n4-Modificar\n0-Sair\n");
        scanf("%d", &opcao);

        switch (opcao)
        {
        case 1:
            inserirTransacao();
            break;
        case 2:
            buscarTransacao();
            break;
        case 3:
            listarTransacoes();
            break;
        case 4:
            modificarTransacao();
            break;
        case 0:
            printf("Saindo...\n");
            break;
        default:
            printf("Opcao invalida\n");
        }
    } while (opcao != 0);

    return 0;
}
