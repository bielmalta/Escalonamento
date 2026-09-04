#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TAREFAS 100

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
    for (int tempo = 0; tempo < tempo_total; tempo++) {
        verificar_deadlines(tarefas, total_tarefas, tempo);
        verificar_chegadas(tarefas, total_tarefas, tempo);

        int atual = escolher_rate(tarefas, total_tarefas);

        if (atual != -1) {
            tarefas[atual].restante--;
            if (tarefas[atual].restante == 0) {
            tarefas[atual].completos++;
            }
        }
    }
    return 0;
}