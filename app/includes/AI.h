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


typedef struct GameState{
    int last_pick;
    int total_sticks;
    int max_pick;
    int player_turn;
    int ai_difficulty;
    short type;
    short strat;
    graphe_t ai_graphe;
} GameState;

chainon_t * ajouter_chainon(int numero_sommet, liste_t *liste);
void detruire_liste(liste_t *liste);
void detruire_graphe(graphe_t *graphe);
graphe_t init_graphe(int nbRod, int maxPick);
liste_t noyau(graphe_t *g, GameState *state);
void write_graphviz(FILE *f, graphe_t *g, GameState *state);
int jouer_coup(graphe_t *g, int sommet_actuel, GameState *state);
void plan_de_jeu(graphe_t *g, GameState *state);
int AI_pick(graphe_t *g, int node, GameState *state);
int Strat_optimale(graphe_t *g, int node, GameState *state);
int Strat_copie(int last_pick);

#endif
