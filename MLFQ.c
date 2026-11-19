#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#define QUANTO 2
#define SOGLIA_AGING 5

//---------definizione struct processo e coda---------

typedef struct Processo {
    int pid;
    int durata;
    int priorita;
    int tempo_attesa;
    int ingresso;
    struct Processo* next; 
} Processo;


typedef struct Coda{
    Processo* head; 
    Processo* tail;
    int count;      
    int lvl_priorita;
} Coda;

//--------inizializzazione di un processo e di una coda vuota--------

void initProcesso(Processo* p, int pid, int durata, int priorita, int ingresso){
    p->pid = pid;
    p->durata = durata;
    p->priorita = priorita;
    p->tempo_attesa = 0;
    p->ingresso = ingresso;
    p->next = NULL;
}

void initCoda(Coda* q, int lvl_priorita) {
    q->head = NULL;
    q->tail = NULL;
    q->count = 0;
    q->lvl_priorita = lvl_priorita;
}

//---------funzioni di gestione e manipolazione della coda-----------

//controlla se la coda è vuota
bool isEmpty(Coda* q) {
    return (q->head == NULL);
}

//inserisce un processo in fondo alla coda (Enqueue)
void enqueue(Coda* q, Processo* p) {
    if (p == NULL) return;
    
    p->next = NULL; 

    if (isEmpty(q)) {
        q->head = p;
        q->tail = p;
    } else {
        q->tail->next = p;
        q->tail = p;
    }
    q->count++;
}

//estrae e restituisce il primo processo dalla coda (Dequeue)
Processo* dequeue(Coda* q) {
    if (isEmpty(q)) {
        return NULL; 
    }

    Processo* temp = q->head;
    q->head = q->head->next;

    if (q->head == NULL) {
        q->tail = NULL;
    }

    temp->next = NULL; 
    q->count--;
    return temp;
}

//stampa il contenuto di una coda
void stampaCoda(Coda* q) {
    printf("Coda Priorita %d (%d processi): ", q->lvl_priorita, q->count);
    Processo* curr = q->head;
    while (curr != NULL) {
        printf("[P%d | dur:%d] -> ", curr->pid, curr->durata);
        curr = curr->next;
    }
    printf("NULL\n");
}

//trova l'indice della coda non vuota a priorità più alta (-1 se tutte vuote)
int trovaCodaPiuAlta(Coda code[], int num_code) {
    for (int i = 0; i < num_code; i++) {
        if (!isEmpty(&code[i])) {
            return i;
        }
    }
    return -1;
}

//aging: scorre le code a priorità > 0 e promuove chi aspetta troppo
void applicaAging(Coda code[], int num_code, int soglia) {
    for (int i = 1; i < num_code; i++) {
        Processo* curr = code[i].head;
        Processo* prev = NULL;

        while (curr != NULL) {
            curr->tempo_attesa++;
            Processo* prossimo = curr->next;

            if (curr->tempo_attesa >= soglia) {
                if (prev == NULL) {
                    code[i].head = prossimo;
                } else {
                    prev->next = prossimo;
                }
                if (curr == code[i].tail) {
                    code[i].tail = prev;
                }
                code[i].count--;

                curr->priorita = i - 1;
                curr->tempo_attesa = 0;
                printf("  [AGING] P%d promosso da Coda %d a Coda %d\n", curr->pid, i, i - 1);
                enqueue(&code[i - 1], curr);
            } else {
                prev = curr;
            }
            curr = prossimo;
        }
    }
}

//legge i processi da file esterno
Processo* caricaProcessiDaFile(const char* nome_file, int* num_processi) {
    FILE* fp = fopen(nome_file, "r");
    if (fp == NULL) {
        perror("Errore nell'apertura del file");
        return NULL;
    }

    Processo* lista = NULL;
    int count = 0;
    int pid, durata, priorita, ingresso;

    while (fscanf(fp, "%d %d %d %d", &pid, &durata, &priorita, &ingresso) == 4) {
        Processo* temp = realloc(lista, (count + 1) * sizeof(Processo));
        if (temp == NULL) {
            perror("Errore di allocazione memoria");
            free(lista);
            fclose(fp);
            return NULL;
        }
        lista = temp;
        initProcesso(&lista[count], pid, durata, priorita, ingresso);
        count++;
    }

    fclose(fp);
    *num_processi = count;
    return lista;
}



int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <nome_file.txt>\n", argv[0]);
        return 1;
    }

    Coda code[3];
    for (int i = 0; i < 3; i++) {
        initCoda(&code[i], i);
    }

    int num_processi = 0;

    //passa argv[1] direttamente alla funzione di caricamento
    Processo* processi = caricaProcessiDaFile(argv[1], &num_processi);
    if (processi == NULL || num_processi == 0) {
        printf("Nessun processo caricato da '%s'. Uscita.\n", argv[1]);
        return 1;
    }

    //inserimento dei processi che arrivano al tick 0
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < num_processi; j++) {
            if (code[i].lvl_priorita == processi[j].priorita && processi[j].ingresso == 0) {
                enqueue(&code[i], &processi[j]);  
            }
        }
    }

    puts("=== READY QUEUE ===");
    for (int i = 0; i < 3; i++) {
        stampaCoda(&code[i]);
    }

    int tick = 0;
    Processo* corrente = NULL;
    int tick_quanto = 0;
    int completati = 0;

    printf("\n=== INIZIO SIMULAZIONE (Quanto=%d, Aging=%d) ===\n", QUANTO, SOGLIA_AGING);

    while (completati < num_processi) {
        tick++;

        // === fase A: aggiungo i processi del tick corrente ===
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < num_processi; j++) {
                if (code[i].lvl_priorita == processi[j].priorita && tick == processi[j].ingresso) {
                    enqueue(&code[i], &processi[j]);  
                }  
            }
        }

        // === fase B: estrazione del processo con gestione prelazione ===
        int top_coda = trovaCodaPiuAlta(code, 3);

        if (corrente != NULL && top_coda != -1 && top_coda < corrente->priorita) {
            printf("  [PRELAZIONE] P%d (priorita %d) rimesso in coda per priorita superiore (processo P%d)\n",
                   corrente->pid, corrente->priorita, code[top_coda].head->pid);
            enqueue(&code[corrente->priorita], corrente);
            printf("\n");
            for (int i = 0; i < 3; i++) stampaCoda(&code[i]);
            printf("\n");
            corrente = NULL;
            tick_quanto = 0;
        }

        if (corrente == NULL && top_coda != -1) {
            corrente = dequeue(&code[top_coda]);
            tick_quanto = 0;
        }

        // === fase C: esecuzione per 1 tick ===
        if (corrente != NULL) {
            corrente->durata--;
            corrente->tempo_attesa = 0;
            tick_quanto++;
            printf("Tick %2d | Esegue P%d (Coda %d) | Durata rimasta: %d | Quanto: %d/%d\n",
                   tick, corrente->pid, corrente->priorita, corrente->durata, tick_quanto, QUANTO);

            // === fase D: aging sui processi rimasti nelle code ===
            for (int i = 0; i < 3; i++) {
                Processo* p = code[i].head;
                while (p != NULL) {
                    if (i == 0) p->tempo_attesa++;
                    p = p->next;
                }
            }
            applicaAging(code, 3, SOGLIA_AGING);

            // === fase E: controllo fine processo o scadenza del quanto ===
            if (corrente->durata == 0) {
                printf("  [FINE] P%d completato!\n", corrente->pid);
                corrente = NULL;
                tick_quanto = 0;
                completati++;
            } else if (tick_quanto == QUANTO) {
                printf("  [QUANTO SCADUTO] P%d torna in fondo a Coda %d\n",
                       corrente->pid, corrente->priorita);
                enqueue(&code[corrente->priorita], corrente);
                corrente = NULL;
                tick_quanto = 0;
            }
        } else {
            printf("Tick %2d | CPU IDLE (in attesa di nuovi processi)\n", tick);
        }

        printf("\n");
        for (int i = 0; i < 3; i++) stampaCoda(&code[i]);
        printf("\n");
    }

    printf("=== SIMULAZIONE COMPLETATA IN %d TICK ===\n", tick);

    free(processi); 
    return 0;
}