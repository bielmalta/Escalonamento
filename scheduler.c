#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TAREFAS 100
#define LOGIN "ggm"

typedef struct {
    char nome[20];
    int periodo;
    int deadline;
    int burst;
    int restante;
    int deadline_absoluto;
    int completos;
    int perdidos;
    int killed;
} Tarefa;

void verificar_chegadas(Tarefa tarefas[], int total, int tempo) {
    for (int i = 0; i < total; i++) {
        if (tempo % tarefas[i].periodo == 0) {
            tarefas[i].restante = tarefas[i].burst;
            tarefas[i].deadline_absoluto = tempo + tarefas[i].deadline;
        }
    }
}

int escolher_rate(Tarefa tarefas[], int total) {
    int escolhida = -1;

    for (int i = 0; i < total; i++) {
        if (tarefas[i].restante > 0) {
            if (escolhida == -1 || tarefas[i].periodo < tarefas[escolhida].periodo) {
                escolhida = i;
            }
        }
    }
    return escolhida;
}

int escolher_edf(Tarefa tarefas[], int total) {
    int escolhida = -1;

    for (int i = 0; i < total; i++) {
        if (tarefas[i].restante > 0) {
            if (escolhida == -1 || tarefas[i].deadline_absoluto < tarefas[escolhida].deadline_absoluto) {
                escolhida = i;
            }
        }
    }
    return escolhida;
}

void verificar_deadlines(Tarefa tarefas[], int total, int tempo) {
    for (int i = 0; i < total; i++) {
        if (tarefas[i].restante > 0 && tempo == tarefas[i].deadline_absoluto) {
            tarefas[i].perdidos++;
            tarefas[i].restante = 0;
        }
    }
}

int main(int argc, char *argv[]) {

    if (argc != 3) {
        fprintf(stderr, "Erro: numero incorreto de argumentos.\n");
        return 1;
    }

    if (strcmp(argv[1], "rate") != 0 && strcmp(argv[1], "edf") != 0) {
        fprintf(stderr, "Erro: algoritmo invalido.\n");
        return 1;
    }

    FILE *arquivo = fopen(argv[2], "r");

    if (arquivo == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir o arquivo.\n");
        return 1;
    }

    int tempo_total;

    if (fscanf(arquivo, "%d", &tempo_total) != 1 || tempo_total <= 0) {
        fprintf(stderr, "Erro: tempo de simulacao invalido.\n");
        fclose(arquivo);
        return 1;
    }

    Tarefa tarefas[MAX_TAREFAS];
    int total_tarefas = 0;

    while (1) {

        int resultado = fscanf(arquivo, "%19s %d %d %d", tarefas[total_tarefas].nome, &tarefas[total_tarefas].periodo, &tarefas[total_tarefas].deadline,
             &tarefas[total_tarefas].burst
        );

        if (resultado == EOF)
            break;

        if (resultado != 4) {
            fprintf(stderr, "Erro: arquivo malformado.\n");
            fclose(arquivo);
            return 1;
        }

        if (tarefas[total_tarefas].periodo <= 0 || tarefas[total_tarefas].deadline <= 0 || tarefas[total_tarefas].burst <= 0) {
            fprintf(stderr, "Erro: valores invalidos.\n");
            fclose(arquivo);
            return 1;
        }

        if (tarefas[total_tarefas].deadline > tarefas[total_tarefas].periodo || tarefas[total_tarefas].burst > tarefas[total_tarefas].deadline) {
            fprintf(stderr, "Erro: tarefa invalida.\n");
            fclose(arquivo);
            return 1;
        }

        tarefas[total_tarefas].restante = 0;
        tarefas[total_tarefas].deadline_absoluto = 0;

        tarefas[total_tarefas].completos = 0;
        tarefas[total_tarefas].perdidos = 0;
        tarefas[total_tarefas].killed = 0;

        total_tarefas++;

        if (total_tarefas == MAX_TAREFAS)
            break;
    }
    fclose(arquivo);
    char nome_saida[30];

    snprintf(nome_saida, sizeof(nome_saida), "%s_%s.out", argv[1], LOGIN);

    FILE *saida = fopen(nome_saida, "w");

    if (saida == NULL) {
        fprintf(stderr, "Erro: nao foi possivel criar arquivo de saida.\n");
        return 1;
    }

    if (strcmp(argv[1], "rate") == 0)
        fprintf(saida, "EXECUTION BY RATE\n");
    else
        fprintf(saida, "EXECUTION BY EDF\n");

    int anterior = -2;
    int unidades = 0;

    for (int tempo = 0; tempo < tempo_total; tempo++) {
        int perdeu = 0;

        if (anterior >= 0 && tarefas[anterior].restante > 0 && tempo == tarefas[anterior].deadline_absoluto) {
            perdeu = 1;
        }

        verificar_deadlines(tarefas, total_tarefas, tempo);

        if (perdeu) {
            fprintf(saida, "[%s] for %d units - L\n", tarefas[anterior].nome, unidades); anterior = -2;
            unidades = 0;
        }

        verificar_chegadas(tarefas, total_tarefas, tempo);

        int atual;

        if (strcmp(argv[1], "rate") == 0)
            atual = escolher_rate(tarefas, total_tarefas);
        else
            atual = escolher_edf(tarefas, total_tarefas);

        if (atual != anterior) {

            if (anterior >= 0 && unidades > 0) {
                fprintf(saida, "[%s] for %d units - H\n", tarefas[anterior].nome, unidades);

            } else if (anterior == -1 && unidades > 0) {
                fprintf(saida, "idle for %d units\n", unidades);
            }
            anterior = atual;
            unidades = 0;
        }
        unidades++;

        if (atual >= 0) {
            tarefas[atual].restante--;

            if (tarefas[atual].restante == 0) {
                tarefas[atual].completos++;
                fprintf(saida, "[%s] for %d units - F\n", tarefas[atual].nome, unidades);

                anterior = -2;
                unidades = 0;
            }
        }
    }
    if (anterior == -1 && unidades > 0) {
        fprintf(saida, "idle for %d units\n", unidades);

    } else if (anterior >= 0 && unidades > 0) {
        fprintf(saida, "[%s] for %d units - K\n", tarefas[anterior].nome, unidades);
    }
    for (int i = 0; i < total_tarefas; i++) {
        if (tarefas[i].restante > 0)
            tarefas[i].killed++;
    }

    fprintf(saida, "LOST DEADLINES\n");

    for (int i = 0; i < total_tarefas; i++)
        fprintf(saida, "[%s] %d\n", tarefas[i].nome, tarefas[i].perdidos);

    fprintf(saida, "COMPLETE EXECUTION\n");

    for (int i = 0; i < total_tarefas; i++)
        fprintf(saida, "[%s] %d\n", tarefas[i].nome, tarefas[i].completos);

    fprintf(saida, "KILLED\n");

    for (int i = 0; i < total_tarefas; i++)
        fprintf(saida, "[%s] %d\n", tarefas[i].nome, tarefas[i].killed);

    fclose(saida);
    return 0;
}