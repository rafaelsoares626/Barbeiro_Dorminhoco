//BARBEIRO DORMINHOCO
#include "unistd.h"
#include "pthread.h" //biblioteca que chama funções de threads
#include "semaphore.h" //biblioteca que chama funções de semáforos
#include "stdio.h"
#include "stdlib.h"

#define CADEIRAS 5 //número de cadeiras para os clientes à espera
#define TRUE 1 //indica que um semáforo será compartilhado com threads de outros processos

sem_t clientes; //clientes esperando atendimento
sem_t barbeiros; //barbeiro esperando cliente
sem_t mutex; //semáforo de exclusão mútua
int cliente_aguardando = 0; //clientes que estão esperando nas cadeiras

//cabeçalhos das funções
void chegou_cliente();
void cortar_cabelo();
void dorme_barbeiro();
void atende_cliente();
void aguardando_atendimento();
void desistiu_corte();
void* barbeiro(void *arg);
void* cliente(void *arg);

int main(){
    //inicializando os semáforos
    sem_init(&clientes, TRUE, 0); //sem_init(&nome_do_semafáro, FALSE = será compartilhado somente com as threads do processo / TRUE = também será compartilhado com threads de outros processos, valor com o qual será inicializado o semáforo); 1 = livre / 0 = ocupado
    sem_init(&barbeiros, TRUE, 0);
    sem_init(&mutex, TRUE, 1);

    pthread_t barb; //variável struct do tipo thread para o barbeiro
    pthread_t cli; //variável struct do tipo thread para clientes

    //o código considera a existência de apenas um barbeiro (função da biblioteca pthread.h)
    pthread_create(&barb, NULL, (void *) barbeiro, NULL);

    /*
    pthread_create (thread, attr, start_routine, arg)
        thread: um identificador único para uma nova thread;
        attr: argumento utilizado para especificar atributos de uma nova thread, que pode ser NULL para valores padrão;
        start_routine: a rotina que a thread executará depois de criada;
        arg: um argumento único que pode ser passado para start_routine, deve ser passado por referência como um ponteiro de void.
    */

    //criação de clientes
    while(TRUE){
        pthread_create(&cli, NULL, (void *) cliente, NULL);
        sleep(2);
        aguardando_atendimento(); //total de clientes que aguardam atendimento
    }
    return 0;
}

/*
int sem_wait(&semáforo): utilizada para fazer um P/down sobre um semáforo, ou seja, decrementa seu valor e bloqueia se o valor estava em zero.
int sem_post(&semáforo): utilizada para fazer um V/up sobre um semáforo, ou seja, incrementa o seu valor. Se o semáforo valia 0 e algum processo/thread estava bloqueado neste semáforo, então será liberado e poderá executar quando selecionado pelo escalonador.
*/

void* barbeiro(void *arg){
    while(TRUE){
        sem_wait(&clientes); //sinaliza que está atendendo um cliente. O barbeiro dorme se o número de clientes for 0
        sem_wait(&mutex); //acessa a região crítica
        cliente_aguardando = cliente_aguardando - 1; //o número de clientes esperando diminui em razão do atendimento do barbeiro
        sem_post(&barbeiros); //sinaliza o barbeiro para cortar o cabelo       
        sem_post(&mutex); //libera a região crítica para o cliente       
        cortar_cabelo(); //corta o cabelo (fora da região crítica) 
        if(cliente_aguardando == 0){
            dorme_barbeiro();
        }
    }
    pthread_exit(NULL);
}

void* cliente(void *arg){     
    sem_wait(&mutex); //entra na região crítica
    if(cliente_aguardando < CADEIRAS){ //só entra no if se houver cadeiras de espera vazias
        chegou_cliente();        
        cliente_aguardando = cliente_aguardando + 1; //aumenta a qtd de clientes aguardando
        sem_post(&clientes); //sinaliza vaga de atendimento aos clientes em espera. Acorda o barbeiro se necessário
        sem_post(&mutex); //libera a região crítica
        sem_wait(&barbeiros); //Sinaliza que o barbeiro ficará ocupado. Vai dormir se o número de barbeiros livres for 0
        atende_cliente(); //o cliente está sendo atendido        
    }else{
        sem_post(&mutex); //a barbearia está cheia. O cliente terá que ir embora
        desistiu_corte();
    }
    pthread_exit(NULL);
}

void chegou_cliente(){
    printf("Um cliente chegou para cortar cabelo.\n");
}

void cortar_cabelo(){
    printf("O barbeiro vai cortar o cabelo do cliente.\n");
    sleep(3);
    printf("O barbeiro concluiu o corte do cabelo do cliente.\n");
}

void dorme_barbeiro(){
    printf("O barbeiro dorme.\n");    
}

void atende_cliente(){
    printf("O cliente esta sendo atendido.\n");
}

void aguardando_atendimento(){
    printf("Existem %d clientes aguardando atendimento.\n", cliente_aguardando);
}

void desistiu_corte(){
    printf("Um cliente chegou no salão e desistiu! Nao ha mais cadeiras disponiveis.\n");
}


