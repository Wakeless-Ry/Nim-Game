
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>

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

chainon_t * ajouter_chainon(int numero_sommet, liste_t *liste) {
    chainon_t *c = malloc(sizeof(chainon_t));
    if (c==NULL) return NULL;
    c -> numero_sommet = numero_sommet;
    c -> next = *liste;
    *liste = c;
    return c;
}

void detruire_liste(liste_t *liste) {
    chainon_t *c = *liste;
    while (c != NULL) {
        chainon_t *n = c -> next;
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

graphe_t init_graphe(int nbRod, int maxPick) {  // Créé le graphe correspondant au jeu (un noeud = un état du jeu (nombre d'allumettes restantes))
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

liste_t noyau(graphe_t *g) {    // Renvoie la liste des noyaux du graphe
    liste_t liste = NULL;
    int som = g->nbr_sommets;
    bool noyau[som];

    for (int i = 0; i < som; i++)
        noyau[i] = true;

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

void write_graphviz(FILE *f, graphe_t *g) { // Formate le graphe dans un fichier .dot
    fprintf(f, "digraph G {\n\tlayout=dot\n\trankdir=LR\n\tnode [ shape=circle,\n\t\twidth=.5,\n\t\tfixedsize=true,\n\t\tstyle=filled,\n\t\tcolorscheme=paired12,\n\t\tcolor=2,\n\t\tfillcolor=2,\n\t\tfontcolor=11 ]\n\tedge [ width=.4,\n\t\tpenwidth=2,\n\t\tcolorscheme=paired12,\n\t\tcolor=2 ]\n");
    liste_t noyal = noyau(g);
    while (noyal != NULL) {
        fprintf(f, "    S%d [ color=12, fillcolor=12 ]\n", noyal->numero_sommet);
        noyal = noyal->next;
    }
    for(int i=0; i < g->nbr_sommets; i++) {
        liste_t liste = g->listes[i];
        while(liste != NULL) {
            fprintf(f, "    S%d -> S%d\n", i, liste->numero_sommet);
            liste = liste -> next;
        }
    }
    detruire_liste(&noyal);
    fprintf(f, "}\n");
}

int jouer_coup(graphe_t *g, int sommet_actuel) {    // Renvoie le coup optimal (correspondant à un sommet du graphe)
    liste_t noyal = noyau(g);
    liste_t liste = g->listes[sommet_actuel];
    int res = g->nbr_sommets;

    while (liste != NULL && noyal != NULL) {
        while (noyal != NULL && noyal->numero_sommet < liste->numero_sommet){
            //printf("noyal %d < sommet %d\n", noyal->numero_sommet, liste->numero_sommet);
            noyal = noyal->next;
        }

        while (noyal != NULL && liste != NULL && liste->numero_sommet < noyal->numero_sommet){
            //printf("sommet %d < noyal %d\n", liste->numero_sommet, noyal->numero_sommet);
            liste = liste->next;
        }

        if (liste != NULL && noyal != NULL && liste->numero_sommet == noyal->numero_sommet) {
            //printf("Sommet optimal : %d\n", liste->numero_sommet);
            res = liste->numero_sommet;
            liste = liste->next;
        }
    }
    detruire_liste(&noyal);
    return res;
}

void plan_de_jeu(graphe_t *g) { // Sert à afficher le plan de jeu idéal de l'IA (optimal)
    int coup;

    for (int i = 0; i < g->nbr_sommets - 1; i++) {
        printf("De S%d, ", i);
        coup = jouer_coup(g, i);
        if (coup < g->nbr_sommets)
            printf("je joue en S%d\n", coup);
        else
            printf("j'abandonne !\n");
    }
}

int AI_pick(graphe_t *g, int node, int chances) {   // Renvoie le nombre d'allumette que l'IA retire
    int r = (int)rand()%101;

    if (r < chances) {
        int coup = jouer_coup(g, node);
        if (coup < g->nbr_sommets)
            return (node - coup);
        else 
            return ((int)rand()%3 + 1); // Si aucun coup n'est optimal, joue aléatoirement
    }
    else {
        return ((int)rand()%3 + 1);
    }
}

int main() {
    srand(time(NULL));
    graphe_t graphe = init_graphe(20, 3);

    FILE *f = fopen("graphe.dot", "w");
    write_graphviz(f, &graphe);

    plan_de_jeu(&graphe);

    printf("\n\n");

    fclose(f);
    detruire_graphe(&graphe);
}

/*
    Pour résumer, tu veux principalement utiliser la fonction AI_pick() pour avoir le coup de l'IA.
    La fonction nécessite le noeud actuel pour fonctionner, il faut y penser.
    Le int chances correspond à la difficulté, on peut partir sur : 
        Facile = 25
        Normal = 50
        Difficile = 75
        Impossible = 100
*/