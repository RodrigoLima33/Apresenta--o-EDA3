#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <locale.h>

#define MAX_TRANSACOES 100
#define MAX_USUARIOS 10
#define ORDEM 3
#define DADOS_FILE "transacoes.txt"
#define INDEX_FILE "indices.txt"

typedef struct
{
    int id;
    double valor;
    char localizacao[50];
    char codigo[10];
} Transacao;

typedef struct
{
    int id;
    double mediaMensal;
    char localizacaoAtual[50];
} Usuario;

Transacao transacoes[MAX_TRANSACOES];
Usuario usuarios[MAX_USUARIOS];
int totalTransacoes = 0;
int totalUsuarios = 0;

// Estrutura da arvore-B
typedef struct NoB
{
    int numChaves;
    char chaves[ORDEM - 1][10];
    struct NoB *filhos[ORDEM];
    bool folha;
} NoB;

NoB *raiz = NULL;

NoB *criarNo(bool folha)
{
    NoB *no = (NoB *)malloc(sizeof(NoB));
    no->folha = folha;
    no->numChaves = 0;
    for (int i = 0; i < ORDEM; i++)
        no->filhos[i] = NULL;
    return no;
}

void dividirFilho(NoB *pai, int i, NoB *y)
{
    NoB *z = criarNo(y->folha);
    z->numChaves = 1;

    strcpy(z->chaves[0], y->chaves[1]);

    if (!y->folha)
    {
        z->filhos[0] = y->filhos[2];
        z->filhos[1] = y->filhos[3];
    }

    y->numChaves = 1;

    for (int j = pai->numChaves; j >= i + 1; j--)
        pai->filhos[j + 1] = pai->filhos[j];
    pai->filhos[i + 1] = z;

    for (int j = pai->numChaves - 1; j >= i; j--)
        strcpy(pai->chaves[j + 1], pai->chaves[j]);
    strcpy(pai->chaves[i], y->chaves[1]);
    pai->numChaves++;
}

void inserirNaoCheio(NoB *no, char *chave)
{
    int i = no->numChaves - 1;

    if (no->folha)
    {
        while (i >= 0 && strcmp(chave, no->chaves[i]) < 0)
        {
            strcpy(no->chaves[i + 1], no->chaves[i]);
            i--;
        }
        strcpy(no->chaves[i + 1], chave);
        no->numChaves++;
    }
    else
    {
        while (i >= 0 && strcmp(chave, no->chaves[i]) < 0)
            i--;
        i++;
        if (no->filhos[i]->numChaves == ORDEM - 1)
        {
            dividirFilho(no, i, no->filhos[i]);
            if (strcmp(chave, no->chaves[i]) > 0)
                i++;
        }
        inserirNaoCheio(no->filhos[i], chave);
    }
}

void inserirArvoreB(char *chave)
{
    if (!raiz)
    {
        raiz = criarNo(true);
        strcpy(raiz->chaves[0], chave);
        raiz->numChaves = 1;
    }
    else
    {
        if (raiz->numChaves == ORDEM - 1)
        {
            NoB *s = criarNo(false);
            s->filhos[0] = raiz;
            dividirFilho(s, 0, raiz);

            int i = 0;
            if (strcmp(chave, s->chaves[0]) > 0)
                i++;
            inserirNaoCheio(s->filhos[i], chave);
            raiz = s;
        }
        else
        {
            inserirNaoCheio(raiz, chave);
        }
    }
}

bool buscarArvoreB(NoB *no, char *chave)
{
    int i = 0;
    while (i < no->numChaves && strcmp(chave, no->chaves[i]) > 0)
        i++;

    if (i < no->numChaves && strcmp(chave, no->chaves[i]) == 0)
        return true;

    if (no->folha)
        return false;

    return buscarArvoreB(no->filhos[i], chave);
}

void carregarUsuarios()
{
    usuarios[0].id = 1;
    usuarios[0].mediaMensal = 500.00;
    strcpy(usuarios[0].localizacaoAtual, "Manaus");

    usuarios[1].id = 2;
    usuarios[1].mediaMensal = 3000.00;
    strcpy(usuarios[1].localizacaoAtual, "Amapa");

    totalUsuarios = 2;
}

Usuario *encontrarUsuario(int id)
{
    for (int i = 0; i < totalUsuarios; i++)
    {
        if (usuarios[i].id == id)
            return &usuarios[i];
    }
    return NULL;
}

bool transacaoSuspeita(Transacao *t, Usuario *u)
{
    bool valorAlto = t->valor > 3 * u->mediaMensal;
    bool localDiferente = strcmp(t->localizacao, u->localizacaoAtual) != 0;
    bool internacionalComBaixaMedia = strstr(t->localizacao, "internacional") != NULL && u->mediaMensal < 1000;
    return valorAlto || localDiferente || internacionalComBaixaMedia;
}

void salvarTransacaoEmArquivo(Transacao *t)
{
    FILE *fp = fopen(DADOS_FILE, "a");
    if (fp)
    {
        fprintf(fp, "%s|%d|%.2f|%s\n", t->codigo, t->id, t->valor, t->localizacao);
        fclose(fp);
    }
    inserirArvoreB(t->codigo);
}

void buscarTransacao(char *codigo)
{
    if (buscarArvoreB(raiz, codigo))
    {
        FILE *fp = fopen(DADOS_FILE, "r");
        char linha[200];
        while (fgets(linha, sizeof(linha), fp))
        {
            if (strncmp(linha, codigo, strlen(codigo)) == 0)
            {
                printf("\nTransacao encontrada: %s", linha);
                break;
            }
        }
        fclose(fp);
    }
    else
    {
        printf("\nTransacao com codigo %s nao encontrada.\n", codigo);
    }
}

void listarTransacoes()
{
    FILE *fp = fopen(DADOS_FILE, "r");
    if (!fp)
        return;

    char linha[200];
    printf("\nLista de transacoes:\n");
    while (fgets(linha, sizeof(linha), fp))
    {
        printf("%s", linha);
    }
    fclose(fp);
}

int main()
{
    setlocale(LC_ALL, "C");

    carregarUsuarios();

    int opcao;
    do
    {
        printf("\n--- MENU ---\n");
        printf("1. Inserir transacao\n");
        printf("2. Buscar transacao\n");
        printf("3. Listar transacoes\n");
        printf("0. Sair\n");
        printf("Escolha: ");
        scanf("%d", &opcao);
        getchar();

        if (opcao == 1)
        {
            Transacao t;
            printf("ID do Usuario: ");
            scanf("%d", &t.id);
            printf("Valor: ");
            scanf("%lf", &t.valor);
            printf("Localizacao: ");
            scanf("%s", t.localizacao);
            sprintf(t.codigo, "TX%02d", totalTransacoes + 1);

            Usuario *u = encontrarUsuario(t.id);
            if (!u)
            {
                printf("Usuario nao encontrado.\n");
                continue;
            }

            if (transacaoSuspeita(&t, u))
                printf(">> Transacao %s eh SUSPEITA para Usuario %d\n", t.codigo, u->id);
            else
                printf("Transacao %s esta OK\n", t.codigo);

            salvarTransacaoEmArquivo(&t);
            totalTransacoes++;
        }
        else if (opcao == 2)
        {
            char codigo[10];
            printf("Codigo da transacao a buscar: ");
            scanf("%s", codigo);
            buscarTransacao(codigo);
        }
        else if (opcao == 3)
        {
            listarTransacoes();
        }

    } while (opcao != 0);

    return 0;
}