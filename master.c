#include "includes/includes.h"
#include "utils/utils.h"
#include "utils/semaphore.h"
#include "utils/shared_memory.h"
#include "utils/message_queue.h"

int end = 1;
int termination;

void sig_handler(int sig) {
    /* gestione sezione critica:dontrollo che la flag end==1 per evitare che successivi segnali possano
     *cambiare la variabile di terminazione.
     * NON È POSSIBILE UTILIZZARE SIGPROCMASK NEL HANDLER DATO CHE GLI EFFETTI
     * DI CAMBIAMENTO NELLA MASCHERE VENGONO UNSETTATI ALL'USCITA DELL'HANDLER
    */

    if (end == 1) {
        switch (sig) {
            case SIGALRM:
                termination = TIMEOUT;
                end = 0;
                break;
            case SIGUSR1:
                termination = MELTDOWN;
                end = 0;
                break;
            default:
                termination = -1;
                break;
        }
    }
}

/*stampa stato corrente della simulazione*/
void printStats(Stats *stats) {
    printf("--------------------------------TOTALI---------------------------------------------------------------\n");
    printf("\n");
    printf("\033[K"); // Pulisce la riga
    printf("Attivazioni: %d Scissioni: %d Energia Prodotta: %d Energia Consumata: %d Scorie Prodotte: %d\n",
           stats->attivazioni_totali, stats->scissioni_totali, stats->energia_prodotta_totale,
           stats->energia_consumata_totale, stats->scorie_totali);
    printf("\n");
    printf("-------------------------------AL SECONDO------------------------------------------------------------\n");
    printf("\n");
    printf("\033[K"); // Pulisce la riga
    printf("Attivazioni: %d Scissioni: %d Energia Prodotta: %d Energia Consumata: %d Scorie Prodotte: %d\n",
           stats->attivazioni_ultimo_secondo, stats->scissioni_ultimo_secondo, stats->energia_prodotta_ultimo_secondo,
           stats->energia_consumata_ultimi_secondo, stats->scorie_ultimo_secondo);
    printf("\n");
    printf("-------------------------------ENERGIA TOTALE---------------------------------------------------------\n");
    printf("\n");
    printf("\033[K"); // Pulisce la riga
    printf("ENERGIA TOTALE : %d", stats->energia);
    printf("\n");
    printf("\n");
    fflush(stdout);
    printf("\033[12A");
}

/*rimozione risorse ipcs */
void deallocate_resources(int id_sem, int id_msq, int id_sh) {
    msgctl(id_msq, IPC_RMID, NULL);
    semctl(id_sem, 0, IPC_RMID, 0);
    shmctl(id_sh, IPC_RMID, NULL);
}


int main() {
    struct Conf c;
    int scelta;
    /*scelta dei file di configurazione tramite vaiabile scelta*/
    while (1) {
        printf("Scegli un numero tra 1 e 4 per selezionare la configurazione desiderata(0 per uscire):\n");
        printf("1-TIMEOUT\n2-BLACKOUT\n3-EXPLODE\n4-MELTDOWN\n");
        printf("\nSCELTA:");
        char buffer[10];
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            printf("ERRORE\n");
            exit(EXIT_SUCCESS);
        }
        if (sscanf(buffer, "%d", &scelta) == -1 || (scelta > 4 || scelta < 0)) {
            printf("\nInput non valido. Inserisci un numero tra 1 e 5 o 0 per uscire.\n\n");
            continue;
        } else if (scelta == 0) {
            printf("\nUSCITA\n");
            exit(EXIT_SUCCESS);
        } else {
            break;
        }

    }
    switch (scelta) {
        case 1:
            printf("HAI SCELTO: TIMEOUT\n\n.");
            c = reader_conf_File("Timeout.conf");
            break;
        case 2:
            printf("HAI SCELTO: BLACKOUT\n\n");
            c = reader_conf_File("Blackout.conf");
            break;
        case 3:
            printf("HAI SCELTO: EXPLODE\n\n");
            c = reader_conf_File("Explode.conf");
            break;
        case 4:
            printf("HAI SCELTO: MELTDOWN\n\n");
            c = reader_conf_File("Meltdown.conf");
            break;
        default:
            exit(EXIT_FAILURE);
    }

/*inizializzazione risorse ipcs*/
    pid_t pid;
    pid_t children[2];
    Stats *st;

    int id_sem = semget(IPC_PRIVATE, 4, IPC_CREAT | 0600);
    int id_msq = msgget(IPC_PRIVATE, IPC_CREAT | 0600);
    int id_sh = shmget(IPC_PRIVATE, sizeof(Stats), IPC_CREAT | 0600);
    if ((st = (Stats *) shmat(id_sh, NULL, 0)) == (Stats *) -1) {
        TEST_ERROR;
        exit(EXIT_FAILURE);
    }
/*settaggio statische in memoria condivisa */
    st->energia = 0;
    st->attivazioni_totali = 0;
    st->scissioni_totali = 0;
    st->energia_prodotta_totale = 0;
    st->energia_consumata_totale = 0;
    st->scorie_totali = 0;
    st->attivazioni_ultimo_secondo = 0;
    st->scissioni_ultimo_secondo = 0;
    st->energia_prodotta_ultimo_secondo = 0;
    st->energia_consumata_ultimi_secondo = 0;
    st->scorie_ultimo_secondo = 0;
    st->pid_master = getpid();/*salvo pid del master utile per i processi per il meltdown*/

    /*inizializzazione semaforo per le far partire in contemporane tutti processi allo stesso momento */
    sem_setVal(id_sem, ID_READY, c.N_ATOMI_INIT + 3);

    /*inizializzazione semafori per le statische per la  lettura e la scrittura su memoria condivisa in modo concorrente*/
    sem_setVal(id_sem, ID_ACTIVATIONS, 1);
    sem_setVal(id_sem, ID_SPLIT, 1);
    sem_setVal(id_sem, ID_WASTE, 1);

    /*settaggio semafori per la mutua esclusione*/
    struct sembuf sop[3];
    sop[0].sem_flg = 0;
    sop[0].sem_num = ID_ACTIVATIONS;
    sop[1].sem_flg = 0;
    sop[1].sem_num = ID_SPLIT;
    sop[2].sem_flg = 0;
    sop[2].sem_num = ID_WASTE;

    char *env[] = {NULL};
    char sem_id_str[BUFFERSZ];
    char msg_id_str[BUFFERSZ];
    char shm_id_str[BUFFERSZ];
    char min_num_str[BUFFERSZ];
    char step_alimentazione_str[BUFFERSZ];
    char step_attivatore_str[BUFFERSZ];
    char new_atoms[BUFFERSZ];
    char atom_num_max[BUFFERSZ];
    char atomic_num_str[BUFFERSZ];
    int atomic_num;

    /*inizializzazione semafori per le statische per la  lettura e la scrittura su memoria condivisa*/
    struct sigaction sa;
    sa.sa_handler = sig_handler;
    sigset_t mask;
    sa.sa_flags = 0;
    /*maschera per SIGUSR!:evita che un segnale di ALARM(avviso di timeout)possa interrompere l'esecuzione del handler
     che sta agendo su variabili di terminazione che sono in sezione critica*/
    sigaddset(&mask, SIGUSR1);
    sa.sa_mask = mask;
    sigaction(SIGALRM, &sa, NULL);
    /*maschera per ALARM:evita che un segnale di SIGUSR1(avviso di meltdwon)possa interrompere l'esecuzione del handler
       che sta agendo su variabili di terminazione che sono in sezione critica*/
    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);
    sa.sa_mask = mask;
    sigaction(SIGUSR1, &sa, NULL);

    /*settaggi della maschera per i segnali di SIGALARM E SIGUSR1:utile quando avviene un explode o un blackout
     * ed eivtare che venga eseguito l'hadler dei segnali che agiscono sulla variabile di terminazione
     */
    sigaddset(&mask, SIGALRM);
    sigaddset(&mask, SIGUSR1);

    sprintf(sem_id_str, "%d", id_sem);
    sprintf(msg_id_str, "%d", id_msq);
    sprintf(shm_id_str, "%d", id_sh);
    sprintf(min_num_str, "%d", c.MIN_N_ATOMICO);
    sprintf(step_attivatore_str, "%ld", c.STEP_ATTIVATORE);
    sprintf(step_alimentazione_str, "%ld", c.STEP_ALIMENTAZIONE);
    sprintf(new_atoms, "%d", c.N_NUOVI_ATOMI);
    sprintf(atom_num_max, "%d", c.N_ATOM_MAX);


    /*fork del master per la generazione dei figli atomi iniziali*/
    for (int i = 0; i < c.N_ATOMI_INIT && end==1; i++) {
        srand(getpid() + i);
        atomic_num = rand() % c.N_ATOM_MAX + 1;
        switch (fork()) {
            case -1:
                //fprintf(stderr, "ERRORE FORK\n");
                end=0;
                termination=MELTDOWN;
                break;
            case 0:
                sprintf(atomic_num_str, "%d", atomic_num);
                char *args[] = {"atomo", sem_id_str, msg_id_str, atomic_num_str, shm_id_str, min_num_str, NULL};
                execve("./atomo", args, env);
                break;
            default:
                break;
        }
    }


    /*fork del master per la generazione dell'attivatore e alimentazione*/
    for (int i = 0; i < 2 && end==1; i++) {
        switch (pid = fork()) {
            case -1:
               // fprintf(stderr, "ERRORE FORK\n");
                end=0;
                termination=MELTDOWN;
                break;
            case 0:
                if (i == 0) {
                    char *args[] = {"attivatore", sem_id_str, msg_id_str, shm_id_str, step_attivatore_str, NULL};
                    execve("./attivatore", args, env);
                } else {
                    char *args[] = {"alimentazione", sem_id_str, msg_id_str, new_atoms, atom_num_max, shm_id_str,
                                    step_alimentazione_str, min_num_str, NULL};
                    execve("./alimentazione", args, env);

                }
                break;
            default:
                /*master salva i pid dell'attivatore e alimentazione che gli serviranno
                 * quando dovra terminarli
                 */
                children[i] = pid;
                break;
        }
    }

    struct timespec timespec;
    timespec.tv_sec = 1;
    timespec.tv_nsec = 0;

    /*si mette in attesa sun una waitfor0 per attendere che tutti i processi abbiano terminato la loro
     * inizializzazione
     */
if(end==1) {


    //printf("Processo MASTER con PID %d: Sono pronto.\n", getpid());
    sem_reserve(id_sem, ID_READY);
    sem_wait(id_sem, ID_READY);

    //printf("Processo MASTER con PID %d: Sono Partito.\n", getpid());

    /*alarm per iniziare la simulazione*/
    alarm(c.SIM_DURATION);

    while (end) {
        /*ogni secondo detrae energia e stampa le statistiche*/
        if (nanosleep(&timespec, NULL) == -1) {
            if (errno == EINTR)
                continue;

        }

        /*acquisizione di tutti i semafori per eseguire operazioni sulle statisiche*/

        if (sem_reserve_all(id_sem, sop, 3) == -1) {
            /*se interrotto da un segnale,torna all'inizio del ciclo pe controllare
             *se deve terminare
             */
            if (errno == EINTR)
                continue;

        }

        st->energia_consumata_totale += c.ENERGY_DEMAND;
        st->energia_consumata_ultimi_secondo += c.ENERGY_DEMAND;
        /*controllo sull'energia per blackout*/
        if ((st->energia -= c.ENERGY_DEMAND) <= 0 && end == 1) {
            /*maschero i segnali di SIGUSR1 E ALARM per modificare lo stato di terminazione in BLACKOUT*/
            sigprocmask(SIG_BLOCK, &mask, NULL);
            st->energia = 0;
            end = 0;
            termination = BLACKOUT;
            /*controllo sull'energia per EXPLODE*/
        } else if (st->energia >= c.ENERGY_EXPLODE_THRESHOLD && end == 1) {
            /*maschero i segnali di SIGUSR1 E ALARM per modificare lo stato di terminazione in EXPLODE*/
            sigprocmask(SIG_BLOCK, &mask, NULL);
            end = 0;
            termination = EXPLODE;
        }

        printStats(st);
        /*setto le statiche al secondo a 0  */
        st->energia_prodotta_ultimo_secondo = 0;
        st->scissioni_ultimo_secondo = 0;
        st->scorie_ultimo_secondo = 0;
        st->attivazioni_ultimo_secondo = 0;
        st->energia_consumata_ultimi_secondo = 0;
        /*controllo la flag di terminazione per non rilasciare il semaforo agli altri
         * processi ed evitare che scrivano in memorio condivisa anche se la simulazione è terminata
         */
        if (end == 0) {
            break;
        }

        if (sem_release_all(id_sem, sop, 3) == -1) {
            if (errno == EINTR)
                continue;

        }


    }
    /*blocco segnali di SIGUSR1 e ALARM dato che la simulazione è terminata*/
    sigprocmask(SIG_BLOCK, &mask, NULL);
    printf("\033[12B");
    /*dealloco tutte le risosrse IPC E MANDO SEGNALE DI SIGALARM all'attivatore
     * e alla alimentazione per farli terminare
     */
}
    deallocate_resources(id_sem, id_msq, id_sh);
    kill(children[0], SIGALRM);
    kill(children[1], SIGALRM);

    /*attende che tutti i figli siano terminati*/
    while (waitpid(-1, NULL, 0) != -1);

    if (errno == ECHILD) {
        /*controlla lo stato di terminazione*/
        switch (termination) {
            case TIMEOUT:
                printf("[TIMEOUT] Simulazione TERMINATA PER TIMEOUT\n");
                break;
            case BLACKOUT:
                printf("[BLACKOUT] Simulazione TERMINATA PER BLACKOUT\n");
                break;
            case EXPLODE:
                printf("[EXPLODE] Simulazione TERMINATA PER EXPLODE\n");
                break;
            case MELTDOWN:
                printf("[MELTDOWN] Simulazione TERMINATA PER MELTDOWN\n");
                break;
            default:
                printf("[ERRORE] Simulazione TERMINATA PER ERRORE\n");
                break;
        }


        exit(EXIT_SUCCESS);


    } else {
        TEST_ERROR
        // printf("[MASTER]PID=%d NON SONO TERMINATO CORRETTAMENTE PER UN ALARM\n",getpid());
        exit(EXIT_FAILURE);
    }

}



