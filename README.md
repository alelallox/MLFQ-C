# Simulatore CPU Scheduler Multi-Level Feedback Queue in C

Simulatore in linguaggio C di uno **scheduler della CPU a code multilivello (MLFQ)** con supporto a:
- **3 livelli di priorità** (`0` = massima priorità, `1` = media, `2` = minima)
- **Prelazione (Preemption)** immediata all'arrivo o alla promozione di un processo più prioritario
- **Round Robin** intra-coda con quanto di tempo configurabile
- **Aging dinamico** per prevenire la *starvation* dei processi a bassa priorità
- **Caricamento dinamico dei processi** da file esterno

---

## Caratteristiche principali

1. **Code a Priorità (`3`):**
   I processi vengono smistati in tre *ready queue* separate (implementate tramite liste concatenate con puntatori `head` e `tail` per inserimenti ed estrazioni in tempo $O(1)$).
2. **Prelazione:**
   A ogni *tick* di clock, se nella coda a priorità più alta è presente un processo con priorità superiore rispetto a quello attualmente in CPU, il processo corrente viene interrotto, rimesso in fondo alla propria coda e sostituito.
3. **Round Robin:**
   Ogni processo può occupare la CPU per un massimo di `QUANTO` tick consecutivi. Se non termina entro il quanto, viene rimesso in fondo alla sua coda di appartenenza.
4. **Aging:**
   I processi in attesa nelle code `1` e `2` incrementano a ogni tick il proprio contatore `tempo_attesa`. Al raggiungimento di `SOGLIA_AGING`, il processo viene promosso alla coda superiore (`priorita - 1`) con `tempo_attesa` azzerato.
5. **Gestione arrivi asincroni e CPU Idle:**
   I processi entrano nel sistema al tick specificato dal campo `ingresso`. Se tutti i processi arrivati sono terminati ma ne restano altri da schedulare, la CPU entra in stato `IDLE` fino al prossimo arrivo.

---

## Struttura del File di Input

Il programma legge i processi da un file di testo in cui ogni riga contiene **4 valori interi** separati da spazi:

```text
<PID> <DURATA> <PRIORITA> <INGRESSO>
