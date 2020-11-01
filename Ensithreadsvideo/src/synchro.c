#include "synchro.h"
#include "ensitheora.h"
#include <pthread.h>
bool fini;

extern int windowsx;
extern int windowsy;

int compteur = 0;
/* les variables pour la synchro, ici */
/************** le même mutex pour tous les threads ***********************/
pthread_mutex_t fenetre;
bool affichage = false;
bool texture = false;
/************** Condition d'affichage *************************************/
pthread_cond_t display;
pthread_cond_t graphic;
pthread_cond_t cons;
pthread_cond_t prod;
/* l'implantation des fonctions de synchro ici */
/******************************************************************************/
void envoiTailleFenetre(th_ycbcr_buffer buffer) {
  /* debut section critique */
  pthread_mutex_lock(&fenetre);

  windowsx = buffer[0].width;
  windowsy = buffer[0].height;
  affichage = true;
  /* on réveille le processus qui attend la taille de la fenetre */
  pthread_cond_signal(&display);

  pthread_mutex_unlock(&fenetre);
  /* fin section critique */
}
/******************************************************************************/
void attendreTailleFenetre() {
  /* debut section critique */
  pthread_mutex_lock(&fenetre);

  while(!affichage)
  {
    /* si les dimensions ne sont pas affectées : on attend */
    pthread_cond_wait(&display, &fenetre);
  }

  pthread_mutex_unlock(&fenetre);
  /* fin section critique */
}
/******************************************************************************/
void signalerFenetreEtTexturePrete() {
  /* debut section critique */
  pthread_mutex_lock(&fenetre);
  texture = true;
  /* on réveille le processus qui attend la texture de la fenetre */
  pthread_cond_signal(&graphic);
  pthread_mutex_unlock(&fenetre);
  /* fin section critique */
}
/******************************************************************************/
void attendreFenetreTexture() {
  /* debut section critique */
  pthread_mutex_lock(&fenetre);

  while(!texture)
  {
    /* si les dimensions ne sont pas affectées : on attend */
    pthread_cond_wait(&graphic, &fenetre);
  }

  pthread_mutex_unlock(&fenetre);
  /* fin section critique */
}
/******************************************************************************/
void debutConsommerTexture() {
  /* debut section critique */
  pthread_mutex_lock(&fenetre);
  while(compteur == 0)
  {
    pthread_cond_wait (&prod, &fenetre);
  }
  pthread_mutex_unlock(&fenetre);
  /* fin section critique */
}
/******************************************************************************/
void finConsommerTexture() {
  /* debut section critique */
  pthread_mutex_lock(&fenetre);
  compteur --;
  pthread_cond_signal(&cons);
  pthread_mutex_unlock(&fenetre);
  /* fin section critique */
}
/******************************************************************************/

void debutDeposerTexture() {
  /* debut section critique */
  pthread_mutex_lock(&fenetre);
  while(compteur == NBTEX)
  {
    pthread_cond_wait (&cons, &fenetre);
  }
  pthread_mutex_unlock(&fenetre);
  /* fin section critique */
}
/******************************************************************************/
void finDeposerTexture() {
  /* debut section critique */
  pthread_mutex_lock(&fenetre);
  compteur ++;
  pthread_cond_signal(&prod);
  pthread_mutex_unlock(&fenetre);
  /* fin section critique */
}
