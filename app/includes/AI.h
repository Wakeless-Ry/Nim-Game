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

typedef enum {
    AI_STRATEGY_MIXED   = 0,  /* Probabiliste : optimal avec proba difficulty%  */
    AI_STRATEGY_COPIE   = 1,  /* Copie le dernier coup du joueur                */
    AI_STRATEGY_MINIMAX = 2,  /* Negamax à profondeur bornée                    */
    AI_STRATEGY_MCTS    = 3,  /* Monte Carlo Tree Search                        */
    AI_STRATEGY_SA      = 4,  /* Recuit simulé (Simulated Annealing)            */
} AIStrategyType;

typedef struct GameState {
    int last_pick;
    int total_sticks;
    int max_pick;
    int player_turn;
    int ai_difficulty;
    AIStrategyType ai_strategy;
    graphe_t ai_graphe;
} GameState;

/* ---- Construction et destruction du graphe ---- */
chainon_t * ajouter_chainon(int numero_sommet, liste_t *liste);
void detruire_liste(liste_t *liste);
void detruire_graphe(graphe_t *graphe);
graphe_t init_graphe(int nbRod, int maxPick);

/* ---- Théorie des jeux ---- */
liste_t noyau(graphe_t *g);
void write_graphviz(FILE *f, graphe_t *g);
int jouer_coup(graphe_t *g, int sommet_actuel);
void plan_de_jeu(graphe_t *g);

/* ---- Stratégies IA ---- */
int AI_pick(graphe_t *g, int node, GameState *state);
int Strat_optimale(graphe_t *g, int node, GameState *state);
int Strat_copie(int last_pick);
int AI_pick_minimax(graphe_t *g, int node, GameState *state);
int AI_pick_mcts(graphe_t *g, int node, GameState *state);
int AI_pick_recuit(graphe_t *g, int node, GameState *state);

#endif
