#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <SDL2/SDL.h>
#include "stream_common.h"
#include "oggstream.h"
#include <stdlib.h>

pthread_mutex_t hashmap;
int main(int argc, char *argv[]) {
    int res;

    if (argc != 2) {
	fprintf(stderr, "Usage: %s FILE", argv[0]);
	exit(EXIT_FAILURE);
    }
    assert(argc == 2);


    // Initialisation de la SDL
    res = SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_EVENTS);
    atexit(SDL_Quit);
    assert(res == 0);

    // start the two stream readers
    pthread_t tid1, tid2;
    pthread_create(&tid2, NULL, (void* (*)(void *))&vorbisStreamReader, (void*)(argv[1]));
    pthread_create(&tid1, NULL, (void* (*)(void *))&theoraStreamReader, (void*)(argv[1]));


    // wait audio thread

    // int status = 5000;
    // while(status != 5000)
    // {
    //   pthread_join(tid2, (void*)&status);
    // }
    pthread_join(tid2, NULL);
    // 1 seconde de garde pour le son,
    sleep(1);

    // tuer les deux threads videos si ils sont bloqués
    pthread_cancel(tid1);
    pthread_cancel(tid);
    // attendre les 2 threads videos
    pthread_join(tid1, NULL);
    pthread_join(tid, NULL);
    exit(EXIT_SUCCESS);
}
