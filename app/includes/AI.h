#ifndef AI_H
#define AI_H

#include <stdio.h>

typedef struct chainon {
    int numero_sommet;
    struct chainon* next;
} chainon_t;

typedef chainon_t* liste_t;

typedef struct {
    int nbr_sommets;
    liste_t *listes;
} graphe_t;

typedef struct {
    chainon_t * debut;
    chainon_t * fin;
} file_t;


chainon_t * ajouter_chainon(int numero_sommet, liste_t *liste);
void detruire_liste(liste_t *liste);
void detruire_graphe(graphe_t *graphe);
graphe_t init_graphe(int nbRod, int maxPick);
liste_t noyau(graphe_t *g);
void write_graphviz(FILE *f, graphe_t *g);
int jouer_coup(graphe_t *g, int sommet_actuel);
void plan_de_jeu(graphe_t *g);
int AI_pick(graphe_t *g, int node, int chances);

#endif
