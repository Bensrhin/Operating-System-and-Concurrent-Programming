/*****************************************************
 * Copyright Grégory Mounié 2008-2015                *
 *           Simon Nieuviarts 2002-2009              *
 * This code is distributed under the GLPv3 licence. *
 * Ce code est distribué sous la licence GPLv3+.     *
 *****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "variante.h"
#include "readcmd.h"

#ifndef VARIANTE
#error "Variante non défini !!"
#endif
#include <fcntl.h>
#include <unistd.h>
/* Pour perror() et errno */
#include <errno.h>
/* Pour le type pid_t */
#include <sys/types.h>
#include <sys/wait.h>
 #include <wordexp.h>
/* Guile (1.8 and 2.0) is auto-detected by cmake */
/* To disable Scheme interpreter (Guile support), comment the
 * following lines.  You may also have to comment related pkg-config
 * lines in CMakeLists.txt.
 */



/**************** la création d'un nouveau processus *************************/
 pid_t create_process(void){
     /* On crée une nouvelle valeur de type pid_t */
     pid_t pid;
     /* On fork() tant que l'erreur est EAGAIN */
     do {
     pid = fork();
     } while ((pid == -1) && (errno == EAGAIN));
     /* On retourne le PID du processus ainsi créé */
     return pid;
 }
 /* La fonction child_process effectue les actions du processus fils */

 void child_process(char *commande, char *arg[]){
     if (execvp(commande, arg) == -1) {
     perror("execvp");
     exit(EXIT_FAILURE);
 	}
 }
/*******************************************************************************/
struct list_proc{
    pid_t pid;
    char *name;
		int status;
    int no;
		bool done;
		enum {PLUS, MINUS, NOTHING} etat;
    struct list_proc *suiv;
		struct list_proc *prev;
};
/*********** pointeur vers la tête de la liste chainée des processus **********/
struct list_proc *proc = NULL;
/**** compteur des processus en cours d'exécution *********/
int cpt = 1;
/******************************************************************************/
/********** affiche et retourne la nouvelle tête des processus ****************/
void affichage_processus(struct list_proc * proc)
{
  while (proc != NULL)
  {
    /** selon l'état du processus dans la liste on écrit un plus ou un moins **/
    if(proc->etat == PLUS)
      printf("[%d]+ ", proc->no);
    else if (proc->etat == MINUS)
      printf("[%d]- ", proc->no);
    else
      printf("[%d]  ", proc->no);
    /********** stat est zero si le processus est en cours d'exécution *******/
    if (proc->status == 0)
    {
      printf("Running ");
      printf("\t \t \t%s\n", proc -> name);
    }
    /*** sinon on affiche son état et on passe au processus suivant ***/
    else
    {
      printf("Done ");
      printf("\t \t \t%s\n", proc -> name);
    }
    proc = proc->suiv;
  }

}
/********** affiche et retourne la nouvelle tête des processus ****************/
struct list_proc* modifier_processus(struct list_proc * proc)
{
  struct list_proc * cour = proc;
  struct list_proc * prev = cour;
  int status;
  while (proc != NULL)
  {
    int stat = waitpid(proc->pid, &(status), WNOHANG);
    /* si le processus dans la liste est done */
    if (proc->status != 0)
    {
      /* le processus reçoit son nouvel état*/
      proc->status = stat;
      /* si le dernier qui termine : */
      if (proc->done){
        /* si l'avant dernier est MINUS :*/
        if (prev->etat == MINUS)
        {
          if (prev->prev != NULL)
          /* donc l'avant l'avant dernier devient minus */
            (prev->prev)->etat = MINUS;
        }
        prev->done = true;
        /* reinitialisation du contour */
        cpt = (prev == proc)?1:prev -> no + 1;
      }
      if (proc -> no == cour->no)
      {
        cour = cour->suiv;
      }
      /**** la suppression */
      prev->etat = proc->etat;
      prev->suiv = (proc->suiv);
      proc = prev;
    }
    else
    {
      /* le processus reçoit son nouvel état*/
      proc->status = stat;
    }
    prev = proc;
    proc = proc->suiv;
  }
  return cour;
}
/**** Ajout d'un processus à la liste des processus en cours d'exécution ******/
struct list_proc* add_list(char *nom, int cpt, pid_t pid, int status, struct list_proc *proc){
    struct list_proc *cour = malloc(sizeof(struct list_proc));
    printf("[%d] %i\n", cpt, pid);
    /* la création de la structure processus */
    cour -> pid = pid;
    cour -> no = cpt;
		cour -> status = status;
		cour -> done = true;
		cour -> etat = PLUS;
    cour -> name = malloc(sizeof *cour->name *(strlen(nom) + 1 ));
		cour->suiv = NULL;
    strcpy(cour->name, nom);
    /** l'insertion de cette structure dans la liste des processus */
    if(proc == NULL){
			cour->prev = proc;
      return cour;
    }
    else{
        struct list_proc *tete = proc;
        while(tete-> suiv != NULL){
						tete->done = false;
						tete->etat = NOTHING;
            tete = tete->suiv;
        }
				cour->prev = tete;
				tete->done = false;
				tete->etat = MINUS;
        tete->suiv = cour;
				return proc;
    }
}
/******************************************************************************/

#if USE_GUILE == 1
#include <libguile.h>

int question6_executer(char *line)
{
  int status;
	struct cmdline *l = parsecmd(&line);
  char **cmd = l->seq[0];
  pid_t pid;
  pid = create_process();
  switch(pid){
    case -1:
      break;
    case 0:
       child_process(cmd[0], cmd);
       break;
    case 1:
       wait(&status);
       if(WIFEXITED(status)){
          break;
        }
      }
	// printf("Not implemented yet: can not execute %s\n", line);

	/* Remove this line when using parsecmd as it will free it */
	free(line);

	return 0;
}

SCM executer_wrapper(SCM x)
{
        return scm_from_int(question6_executer(scm_to_locale_stringn(x, 0)));
}
#endif


void terminate(char *line) {
#if USE_GNU_READLINE == 1
	/* rl_clear_history() does not exist yet in centOS 6 */
	clear_history();
#endif
	if (line)
	  free(line);
	printf("exit\n");
	exit(0);
}


int main() {
        printf("Variante %d: %s\n", VARIANTE, VARIANTE_STRING);

#if USE_GUILE == 1
        scm_init_guile();
        /* register "executer" function in scheme */
        scm_c_define_gsubr("executer", 1, 0, 0, executer_wrapper);
#endif

	while (1)
  {
		struct cmdline *l;
		char *line=0;
		char *prompt = "ensishell>";

		/* Readline use some internal memory structure that
		   can not be cleaned at the end of the program. Thus
		   one memory leak per command seems unavoidable yet */
		line = readline(prompt);
		if (line == 0 || ! strncmp(line,"exit", 4)) {
			terminate(line);
		}

#if USE_GNU_READLINE == 1
		add_history(line);
#endif

#if USE_GUILE == 1
   /* The line is a scheme command */
   if (line[0] == '(') {
     char catchligne[strlen(line) + 256];
     sprintf(catchligne, "(catch #t (lambda () %s) (lambda (key . parameters) (display \"mauvaise expression/bug en scheme\n\")))", line);
     scm_eval_string(scm_from_locale_string(catchligne));
     free(line);
      continue;
      }
#endif
		/* parsecmd free line and set it up to 0 */
		l = parsecmd( & line);
		/* If input stream closed, normal termination */
		if (!l) {
			terminate(0);
		}
		if (l->err) {
			/* Syntax error, read another command */
			printf("error: %s\n", l->err);
			continue;
		}


    /*************** le compte de nombre de pipes ****************************/
    int nbPipe = -1;
    for (int i=0; l->seq[i]!=0; i++)
    {
      nbPipe += 1;
    }
    /**************** tableaux des pid des processus ***************************/
    int pids[nbPipe + 1];
    char **** args = malloc((nbPipe + 1) * sizeof(char ***));
    int nb_len_cmd[nbPipe + 1];
    int **nb_words = malloc((nbPipe + 1) * sizeof(int *));
    /**************** la création des tuyaux entre les commandes **************/
    int pipes[2 * nbPipe];
    for ( int i=0; i< nbPipe; i++)
    {
      if(pipe(pipes + i*2) < 0)
      {
        perror("the process couldn't pipe\n");
        exit(EXIT_FAILURE);
      }
    }
    /*************** l'exécution de toutes les commandes *******************/
    int index_pipe = 0;
    int status = 0;
    for (int i=0; l->seq[i]!=0; i++)
    {
      /**************** i est l'indice de la i eme commande *******************/
      char **cmd = l->seq[i];
      /************ calcul du nombre d'arguments dans chaque commande *********/
      int len_cmd = 0;
      for (int j=0; cmd[j]!=0; j++) {
        len_cmd += 1;
      }
      /*********** on stocke ce nombre dans un tableau *********************/
      nb_len_cmd[i] = len_cmd;
      nb_words[i] = malloc(len_cmd * sizeof(int ));
      /******* calcul du nombre d'arguments dans le cas d'extension ***********/
      char **w;
      int k;
      int nb_cmd = 1;
      int nb_args[len_cmd];
      args[i] = malloc(len_cmd * sizeof(char**));
      /****** on calcule les extensions de chaque argument *******************/
      for (int j=1; cmd[j]!=0; j++)
      {
        wordexp_t p;
        wordexp(cmd[j], &p, 0);
        w = p.we_wordv;
        args[i][j] = malloc((p.we_wordc) * sizeof(char*));
        nb_args[j] = p.we_wordc;
        nb_words[i][j] = p.we_wordc;
        for (k=0; k<p.we_wordc; k++)
        {
          args[i][j][k] = malloc(sizeof *args[i][j][k] * (strlen(w[k]) + 1 ));
          strcpy(args[i][j][k], w[k]);
          nb_cmd += 1;
        }
        wordfree(&p);
      }
      /**************** on rassemble ces extensions ***************************/
      char *words[nb_cmd + 1];
      words[0] = cmd[0];
      int compteur=0;
      for (int l=1; l<len_cmd; l++)
      {
        for (int k=0; k<nb_args[l]; k++)
        {
          compteur += 1;
          words[compteur] = args[i][l][k];
        }
      }
      words[nb_cmd] = NULL;
      /************* le nouveau argument est words ***************************/

			/************************ Si la commande est jobs ***********************/
			bool jobs = false;
			if (!strncmp(cmd[0], "jobs", 4)) jobs = true;
			/********* création d'un processus qui exécute la commande j ***********/

			pid_t child_pid;
      if (jobs)
      {
        /* le cas de jobs on modifie liste des processus */
      proc = modifier_processus(proc);
      }
			child_pid = create_process();

			switch(child_pid)
			{
				/* Si on a une erreur irrémédiable (ENOMEM dans notre cas) */
				case -1:
					perror("fork");
					return EXIT_FAILURE;
					break;
				/* Si on est dans le fils */
				case 0:
            /*** si la commande est jobs, le processus fils n'a pas de rôle ***/

            /*** vérification des redirections *********/
            if(l->out){
                 int fd =1;
                 fd = open(l->out, O_WRONLY | O_CREAT, S_IRUSR|S_IRGRP|S_IROTH|S_IWUSR);
                 dup2(fd, 1);
                 close(fd);
             }
             if(l-> in != NULL){
                 int fd2 = open(l->in, O_RDWR, 0);
                 dup2(fd2, 0);
                 close(fd2);
             }
             /*********** si n'est pas la dernière commande ***************/
             if(l ->seq[i+1] != NULL){
                if (dup2(pipes[index_pipe + 1], 1) < 0)
                {
                  perror("last cmd : erreur de duplication\n");
                  exit(EXIT_FAILURE);
                }
              }
              /*********** si n'est pas la première commande ****************/
              if (index_pipe != 0)
              {
                if (dup2(pipes[index_pipe - 2], 0) < 0)
                {
                  perror("first cmd : erreur de duplication\n");
                  exit(EXIT_FAILURE);
                }
              }
              for (int k = 0; k < 2 * nbPipe; k++)
              {
                close(pipes[k]);
              }
              /**************** et finalement l'excécution de la commande ****/
              if (jobs){
                /* si la commande est jobs on affiche
                  les états des processus et on quitte */
                affichage_processus(proc);
                exit(EXIT_SUCCESS);
              }
              /* sinon on exécute l'autre commande */
              child_process(cmd[0], words);
              break;

				/* Si on est dans le père */
  				default:
            pids[i] = child_pid;

            /******** on enregistre le pid de chaque fils ***********/
            if ( l->bg )
            {
              /** si on a un & on doit éviter ce processus,
                  donc on le supprime de la liste des processus fils
              */
              pids[i] = -1;
              /* On ajoute à la liste chainée des processus ce programme ******/
              proc = add_list(cmd[0], cpt, child_pid, status, proc);
              /* on augmente le compteur des processus en cours d'exécution ***/
              cpt += 1;
              break;
          }
          /****** jobs est faite par le processus père **********/

			} // fin switch

    index_pipe += 2;
		} // fin for sur cmds
  /********** le père ferme tous les pipes ******************/
    for (int k = 0; k< 2 * nbPipe; k++)
    {
      close(pipes[k]);
    }
    /********* le père doit attendre ses fils *****************/
    for (int k = 0; k< nbPipe + 1; k++)
    {
      /**** le processus père doit attendre ces fils
            sauf ceux qui sont en background
      ****/
      if (pids[k] > 0){
        // printf("fils %i\n", pids[k]);
        bool finished = false;
        while (!finished)
        {
          finished = (waitpid(pids[k], &status, WNOHANG) == 0 ) ? false:true;
        }
      }
      /* Après la finition du fils k, on libère la mémoire allouée pour les arguments */
      for (int c=1; c<nb_len_cmd[k]; c++)
      {
        for (int w=0; w<nb_words[k][c]; w++)
        {
          free(args[k][c][w]);
        }
        free(args[k][c]);
      }
      free(args[k]);
      free(nb_words[k]);
    }
    /**et finalement on libère args qui joue le rôle d'une matrice des matrices*/
    free(args);
    free(nb_words);
	}// fin while
}
