#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>
#include "../includes/AI.h"

chainon_t * ajouter_chainon(int numero_sommet, liste_t *liste) {
    chainon_t *c = (chainon_t*)malloc(sizeof(chainon_t));
    if (c==NULL) return NULL;
    c->numero_sommet = numero_sommet;
    c->next = *liste;
    *liste = c;
    return c;
}

void detruire_liste(liste_t *liste) {
    chainon_t *c = *liste;
    while (c != NULL) {
        chainon_t *n = c->next;
        free(c);
        c = n;
    }
    *liste = NULL;
}

void detruire_graphe(graphe_t *graphe) {
    for (int i=0; i < graphe->nbr_sommets; i++) {
        detruire_liste(&(graphe->listes[i]));
    }
    if (graphe->listes != NULL) free(graphe->listes);
    graphe->listes = NULL;
    graphe->nbr_sommets = 0;
}

graphe_t init_graphe(int nbRod, int maxPick) {
    graphe_t graphe;
    graphe.listes = malloc(nbRod * sizeof(liste_t));
    graphe.nbr_sommets = nbRod;

    for (int i = nbRod-1; i >= 0; i--) {
        for (int j = 1; j <= maxPick && i-j >= 0; j++) {
            ajouter_chainon(i, &graphe.listes[i-j]);
        }
    }

    return graphe;
}

liste_t noyau(graphe_t *g, GameState *state) {
    liste_t liste = NULL;
    int som = g->nbr_sommets;
    bool noyau[som];

    for (int i = 0; i < som; i++)
        noyau[i] = true;

    if (state->type == 1) noyau[som-1] = false;

    for (int i = som ; i >= 0; i--) {
        liste = g->listes[i];
        while (liste != NULL && noyau[i]) {
            if (noyau[liste->numero_sommet])
                noyau[i] = false;
            liste = liste->next;
        }
    }

    liste_t res = NULL;
    for (int i = som - 1; i >= 0; i--) {
        if (noyau[i])
            ajouter_chainon(i, &res);
    }

    return res;
}

void write_graphviz(FILE *f, graphe_t *g, GameState *state) {
    fprintf(f, "digraph G {\n\tlayout=dot\n\trankdir=LR\n\tnode [ shape=circle,\n\t\twidth=.5,\n\t\tfixedsize=true,\n\t\tstyle=filled,\n\t\tcolorscheme=paired12,\n\t\tcolor=2,\n\t\tfillcolor=2,\n\t\tfontcolor=11 ]\n\tedge [ width=.4,\n\t\tpenwidth=2,\n\t\tcolorscheme=paired12,\n\t\tcolor=2 ]\n");
    liste_t noyal = noyau(g, state);
    while (noyal != NULL) {
        fprintf(f, "    S%d [ color=12, fillcolor=12 ]\n", noyal->numero_sommet);
        noyal = noyal->next;
    }
    for(int i=0; i < g->nbr_sommets; i++) {
        liste_t liste = g->listes[i];
        while(liste != NULL) {
            fprintf(f, "    S%d -> S%d\n", i, liste->numero_sommet);
            liste = liste->next;
        }
    }
    detruire_liste(&noyal);
    fprintf(f, "}\n");
}

int jouer_coup(graphe_t *g, int sommet_actuel, GameState *state) {
    liste_t noyal = noyau(g, state);
    liste_t liste = g->listes[sommet_actuel];
    int res = g->nbr_sommets;

    while (liste != NULL && noyal != NULL) {
        while (noyal != NULL && noyal->numero_sommet < liste->numero_sommet) {
            noyal = noyal->next;
        }
        while (noyal != NULL && liste != NULL && liste->numero_sommet < noyal->numero_sommet) {
            liste = liste->next;
        }
        if (liste != NULL && noyal != NULL && liste->numero_sommet == noyal->numero_sommet) {
            res = liste->numero_sommet;
            liste = liste->next;
        }
    }
    detruire_liste(&noyal);
    return res;
}

int Strat_optimale(graphe_t *g, int node, GameState *state) {
    static int seeded = 0;
    if (!seeded) {
        srand(time(NULL));
        seeded = 1;
    }
    int r = rand() % 101;
    if (r < state->ai_difficulty) {
        int coup = jouer_coup(g, node, state);
        if (coup < g->nbr_sommets)
            return (node - coup);
    }
    return (rand() % 3 + 1);
}

int Strat_copie(int last_pick) {
    if (last_pick == -1) return 1;
    else return last_pick;
}

int AI_pick(graphe_t *g, int node, GameState *state) {
    switch (state->strat) {
        case 0:
            return Strat_optimale(g, node, state);
            break;
        case 1:
            return Strat_copie(state->last_pick);
            break;
    }
}

void plan_de_jeu(graphe_t *g, GameState *state) {
    for (int i = 0; i < g->nbr_sommets - 1; i++) {
        printf("De S%d, je joue en S%d\n", i, jouer_coup(g, i, state));
    }
}
