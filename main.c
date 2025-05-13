#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <locale.h>

#define MAX_TRANSACOES 100
#define MAX_USUARIOS 10
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

void carregarUsuarios()
{
    // Definindo usuários diretamente no código
    usuarios[0].id = 1;
    usuarios[0].mediaMensal = 500.00;
    strcpy(usuarios[0].localizacaoAtual, "Manaus");

    usuarios[1].id = 2;
    usuarios[1].mediaMensal = 3000.00;
    strcpy(usuarios[1].localizacaoAtual, "Amapa");

    usuarios[1].id = 3;
    usuarios[1].mediaMensal = 5000.00;
    strcpy(usuarios[1].localizacaoAtual, "Franca");

    totalUsuarios = 3; // Atualiza o contador de usuários
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
    FILE *index = fopen(INDEX_FILE, "a");
    if (fp && index)
    {
        fprintf(fp, "%s|%d|%.2f|%s\n", t->codigo, t->id, t->valor, t->localizacao);
        fprintf(index, "%s\n", t->codigo);
    }
    fclose(fp);
    fclose(index);
}

void listarTransacoes()
{
    FILE *fp = fopen(DADOS_FILE, "r");
    if (!fp)
        return;

    char linha[200];
    printf("\nLista de transacoes ordenadas por chave primaria (codigo):\n");
    while (fgets(linha, sizeof(linha), fp))
    {
        printf("%s", linha);
    }
    fclose(fp);
}

void buscarTransacao(char *codigo)
{
    FILE *fp = fopen(DADOS_FILE, "r");
    if (!fp)
        return;

    char linha[200];
    while (fgets(linha, sizeof(linha), fp))
    {
        if (strncmp(linha, codigo, strlen(codigo)) == 0)
        {
            printf("\nTransacao encontrada: %s", linha);
            fclose(fp);
            return;
        }
    }
    printf("\nTransacao com codigo %s nao encontrada.\n", codigo);
    fclose(fp);
}

void modificarTransacao(char *codigo)
{
    FILE *fp = fopen(DADOS_FILE, "r");
    FILE *tmp = fopen("temp.txt", "w");
    if (!fp || !tmp)
        return;

    char linha[200];
    bool encontrado = false;

    while (fgets(linha, sizeof(linha), fp))
    {
        if (strncmp(linha, codigo, strlen(codigo)) == 0)
        {
            Transacao t;
            sscanf(linha, "%[^|]|%d|%lf|%[^\n]", t.codigo, &t.id, &t.valor, t.localizacao);
            printf("Novo valor: ");
            scanf("%lf", &t.valor);
            printf("Nova localizacao: ");
            scanf("%s", t.localizacao);
            fprintf(tmp, "%s|%d|%.2f|%s\n", t.codigo, t.id, t.valor, t.localizacao);
            encontrado = true;
        }
        else
        {
            fprintf(tmp, "%s", linha);
        }
    }

    fclose(fp);
    fclose(tmp);

    remove(DADOS_FILE);
    rename("temp.txt", DADOS_FILE);

    if (encontrado)
        printf("\nTransacao modificada com sucesso.\n");
    else
        printf("\nTransacao nao encontrada.\n");
}

int main()
{
    setlocale(LC_ALL, "PORTUGUESE");
    system("chcp 65001 > nul");

    carregarUsuarios();

    int opcao;
    do
    {
        printf("\n--- MENU ---\n");
        printf("1. Inserir transacao\n");
        printf("2. Modificar transacao\n");
        printf("3. Buscar transacao\n");
        printf("4. Listar transacoes\n");
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
            if (u == NULL)
            {
                printf("Usuario nao encontrado.\n");
                continue;
            }

            if (transacaoSuspeita(&t, u))
            {
                printf(">> Transacao %s eh SUSPEITA para Usuario %d\n", t.codigo, u->id);
            }
            else
            {
                printf("Transacao %s esta OK\n", t.codigo);
            }

            salvarTransacaoEmArquivo(&t);
            totalTransacoes++;
        }
        else if (opcao == 2)
        {
            char codigo[10];
            printf("Codigo da transacao a modificar: ");
            scanf("%s", codigo);
            modificarTransacao(codigo);
        }
        else if (opcao == 3)
        {
            char codigo[10];
            printf("Codigo da transacao a buscar: ");
            scanf("%s", codigo);
            buscarTransacao(codigo);
        }
        else if (opcao == 4)
        {
            listarTransacoes();
        }

    } while (opcao != 0);

    return 0;
}
