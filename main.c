#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define ORDEM 3
#define TAM_CODIGO 6
#define MAX_TRANSACOES 1000

typedef struct
{
    char codigo[TAM_CODIGO];
    int idUsuario;
    float valor;
    char local[50];
    char horario[6];
} Transacao;

typedef struct Indice
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

// ---------------------- ARVORE B ----------------------

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

// ---------------------- TRANSACOES ----------------------

float calcularMediaUsuario(int id)
{
    FILE *f = fopen("transacoes.txt", "r");
    if (!f)
        return 0;
    char linha[200];
    float soma = 0;
    int count = 0, uid;
    float val;
    while (fgets(linha, sizeof(linha), f))
    {
        sscanf(linha, "TX%*[^|]|%d|%f|%*[^|]|%*[^\n]", &uid, &val);
        if (uid == id)
        {
            soma += val;
            count++;
        }
    }
    fclose(f);
    return count ? soma / count : 0;
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
        sscanf(linha, "%[^|]|%d|%f|%[^|]|%[^\n]", t.codigo, &t.idUsuario, &t.valor, t.local, t.horario);
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

bool localDiferente(int id, const char *localAtual)
{
    FILE *f = fopen("transacoes.txt", "r");
    if (!f)
        return false;
    char linha[200];
    int uid;
    char loc[50];
    while (fgets(linha, sizeof(linha), f))
    {
        sscanf(linha, "TX%*[^|]|%d|%*f|%[^|]|%*[^\n]", &uid, loc);
        if (uid == id && strcmp(loc, localAtual) != 0)
        {
            fclose(f);
            return true;
        }
    }
    fclose(f);
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
    scanf("%s", t.local);
    printf("Horario (HH:MM): ");
    scanf("%s", t.horario);

    if (transacaoEhSuspeita(&t))
        printf(">> Transacao %s eh SUSPEITA para Usuario %d\n", codigo, t.idUsuario);
    else
        printf("Transacao %s esta OK\n", codigo);

    FILE *f = fopen("transacoes.txt", "a+");
    fseek(f, 0, SEEK_END);
    long posicao = ftell(f);
    fprintf(f, "%s|%d|%.2f|%s|%s\n", codigo, t.idUsuario, t.valor, t.local, t.horario);
    fclose(f);
    printf("Transacao salva em transacoes.txt\n");

    salvarIndice(codigo, posicao);
    printf("Indice %s salvo com sucesso em indices.txt\n", codigo);
}

void buscarTransacao()
{
    char codigo[TAM_CODIGO];
    printf("Codigo da transacao a buscar: ");
    scanf("%s", codigo);
    long pos = buscarIndice(raiz, codigo);
    if (pos == -1)
    {
        printf("Transacao com codigo %s nao encontrada no indice.\n", codigo);
        return;
    }
    FILE *f = fopen("transacoes.txt", "r");
    fseek(f, pos, SEEK_SET);
    char linha[200];
    fgets(linha, sizeof(linha), f);
    fclose(f);
    printf("Transacao encontrada: %s", linha);
}

void listarTransacoes()
{
    FILE *f = fopen("transacoes.txt", "r");
    if (!f)
        return;
    char linha[200];
    printf("\nLista de transacoes:\n");
    while (fgets(linha, sizeof(linha), f))
        printf("%s", linha);
    fclose(f);
}

int main()
{
    carregarIndices();
    int opcao;
    do
    {
        printf("\n--- MENU ---\n");
        printf("1. Inserir transacao\n");
        printf("2. Buscar transacao\n");
        printf("3. Listar transacoes\n");
        printf("0. Sair\n");
        printf("Escolha: ");
        if (scanf("%d", &opcao) != 1)
        {
            while (getchar() != '\n')
                ;
            printf("Entrada invalida!\n");
            continue;
        }
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
        case 0:
            printf("Saindo...\n");
            break;
        default:
            printf("Opcao invalida!\n");
        }
    } while (opcao != 0);
    return 0;
}
