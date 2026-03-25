#include "../includes/AI.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ================================================================
 *  GRAPHE
 * ================================================================ */

chainon_t *ajouter_chainon(int numero_sommet, liste_t *liste) {
  chainon_t *c = (chainon_t *)malloc(sizeof(chainon_t));
  if (c == NULL)
    return NULL;
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
  for (int i = 0; i < graphe->nbr_sommets; i++)
    detruire_liste(&(graphe->listes[i]));
  if (graphe->listes != NULL)
    free(graphe->listes);
  graphe->listes = NULL;
  graphe->nbr_sommets = 0;
}

/*
 * Construit le graphe de jeu avec nbRod+1 nœuds (positions 0..nbRod).
 * Nœud i = i allumettes restantes.  Arête i → i-j pour j dans [1,maxPick].
 * Corrections par rapport à la version originale :
 *   - nbRod+1 nœuds alloués → le nœud initial (total_sticks) est valide
 *   - Toutes les listes NULL-initialisées (évite UB)
 *   - Arêtes dans le bon sens (successeurs = positions avec moins d'allumettes)
 */
graphe_t init_graphe(int nbRod, int maxPick) {
  graphe_t graphe;
  graphe.listes = (liste_t *)malloc((nbRod + 1) * sizeof(liste_t));
  graphe.nbr_sommets = nbRod + 1;
  for (int i = 0; i <= nbRod; i++)
    graphe.listes[i] = NULL;
  for (int i = nbRod; i >= 0; i--)
    for (int j = 1; j <= maxPick && i - j >= 0; j++)
      ajouter_chainon(i - j, &graphe.listes[i]);
  return graphe;
}

/*
 * Calcule le noyau du graphe (positions P = perdantes pour le joueur à jouer).
 * Itère de BAS en HAUT : quand on évalue i, tous ses successeurs (< i) sont
 * traités. ker[i]=true  → P-position (perdant) ker[i]=false → N-position
 * (gagnant : peut atteindre une P-position)
 */
liste_t noyau(graphe_t *g) {
  int som = g->nbr_sommets;
  bool *ker = (bool *)calloc(som, sizeof(bool));
  for (int i = 0; i < som; i++)
    ker[i] = true;

  for (int i = 0; i < som; i++) {
    liste_t liste = g->listes[i];
    while (liste != NULL && ker[i]) {
      if (ker[liste->numero_sommet])
        ker[i] = false;
      liste = liste->next;
    }
  }

  liste_t res = NULL;
  for (int i = som - 1; i >= 0; i--)
    if (ker[i])
      ajouter_chainon(i, &res);
  free(ker);
  return res;
}

void write_graphviz(FILE *f, graphe_t *g) {
  fprintf(f,
          "digraph G {\n\tlayout=dot\n\trankdir=LR\n"
          "\tnode [ shape=circle, width=.5, fixedsize=true, style=filled,\n"
          "\t\tcolorscheme=paired12, color=2, fillcolor=2, fontcolor=11 ]\n"
          "\tedge [ width=.4, penwidth=2, colorscheme=paired12, color=2 ]\n");
  liste_t noyal = noyau(g);
  liste_t tmp = noyal;
  while (tmp != NULL) {
    fprintf(f, "    S%d [ color=12, fillcolor=12 ]\n", tmp->numero_sommet);
    tmp = tmp->next;
  }
  detruire_liste(&noyal);
  for (int i = 0; i < g->nbr_sommets; i++) {
    liste_t liste = g->listes[i];
    while (liste != NULL) {
      fprintf(f, "    S%d -> S%d\n", i, liste->numero_sommet);
      liste = liste->next;
    }
  }
  fprintf(f, "}\n");
}

/*
 * Retourne le nœud cible optimal depuis sommet_actuel (une P-position
 * accessible), ou g->nbr_sommets si aucun coup optimal n'existe (on est
 * soi-même en P-position).
 */
int jouer_coup(graphe_t *g, int sommet_actuel) {
  liste_t noyal = noyau(g);
  liste_t liste = g->listes[sommet_actuel];
  int res = g->nbr_sommets;

  while (liste != NULL && noyal != NULL) {
    while (noyal != NULL && noyal->numero_sommet < liste->numero_sommet)
      noyal = noyal->next;
    while (noyal != NULL && liste != NULL &&
           liste->numero_sommet < noyal->numero_sommet)
      liste = liste->next;
    if (liste != NULL && noyal != NULL &&
        liste->numero_sommet == noyal->numero_sommet) {
      res = liste->numero_sommet;
      liste = liste->next;
    }
  }
  detruire_liste(&noyal);
  return res;
}

void plan_de_jeu(graphe_t *g) {
  for (int i = 0; i < g->nbr_sommets - 1; i++)
    printf("De S%d, je joue en S%d\n", i, jouer_coup(g, i));
}

/* ================================================================
 *  STRATÉGIES — utilitaires internes (static)
 * ================================================================ */

/* Déduit max_pick depuis le graphe (successeurs du nœud le plus haut). */
static int get_max_pick(graphe_t *g) {
  int count = 0;
  liste_t l = g->listes[g->nbr_sommets - 1];
  while (l != NULL) {
    count++;
    l = l->next;
  }
  return count;
}

/*
 * Negamax récursif à profondeur bornée.
 * Retourne +1 (gagne), -1 (perd), 0 (inconnu à l'horizon de recherche).
 */
static int negamax_helper(graphe_t *g, int node, int depth, int max_pick) {
  if (g->listes[node] == NULL)
    return -1;
  if (depth == 0)
    return 0;

  int best = -2;
  liste_t liste = g->listes[node];
  while (liste != NULL) {
    int val = -negamax_helper(g, liste->numero_sommet, depth - 1, max_pick);
    if (val > best)
      best = val;
    liste = liste->next;
  }
  return best;
}

/*
 * Simulation aléatoire complète depuis node (playout MCTS).
 * Retourne +1 si le joueur qui agit en premier depuis node gagne, -1 sinon.
 */
static int mcts_playout(graphe_t *g, int node) {
  int current = node;
  int sign = 1;

  while (g->listes[current] != NULL) {
    int count = 0;
    liste_t l = g->listes[current];
    while (l != NULL) {
      count++;
      l = l->next;
    }

    int r = rand() % count;
    l = g->listes[current];
    for (int i = 0; i < r; i++)
      l = l->next;

    current = l->numero_sommet;
    sign = -sign;
  }
  return -sign;
}

/* ================================================================
 *  STRATÉGIES PUBLIQUES
 * ================================================================ */

/*
 * MIXED : joue le coup optimal avec probabilité ai_difficulty%,
 * sinon joue un coup aléatoire (1..max_pick).
 */
int Strat_optimale(graphe_t *g, int node, GameState *state) {
  static int seeded = 0;
  if (!seeded) {
    srand(time(NULL));
    seeded = 1;
  }

  int r = rand() % 101;
  if (r < state->ai_difficulty) {
    int coup = jouer_coup(g, node);
    if (coup < g->nbr_sommets)
      return (node - coup);
  }
  return (rand() % state->max_pick + 1);
}

/* COPIE : rejoue le dernier coup du joueur (ou 1 si premier tour). */
int Strat_copie(int last_pick) {
  if (last_pick == -1)
    return 1;
  return last_pick;
}

/*
 * MINIMAX / Negamax à profondeur depth = ai_difficulty / 10.
 *   difficulty=0   → depth=0  → aléatoire pur
 *   difficulty=50  → depth=5  → stratégie partielle
 *   difficulty=100 → depth=10 → quasi-optimal
 * Réservoir sampling pour briser les ex-æquo uniformément.
 */
int AI_pick_minimax(graphe_t *g, int node, GameState *state) {
  if (g->listes[node] == NULL)
    return 1;

  int depth = state->ai_difficulty / 10;

  if (depth <= 0) {
    int nb_moves = 0;
    liste_t l = g->listes[node];
    while (l != NULL) {
      nb_moves++;
      l = l->next;
    }
    int r = rand() % nb_moves;
    l = g->listes[node];
    for (int i = 0; i < r; i++)
      l = l->next;
    return node - l->numero_sommet;
  }

  int max_pick = get_max_pick(g);
  int best_val = -2;
  int best_pick = 1;
  int tie_count = 0;

  liste_t liste = g->listes[node];
  while (liste != NULL) {
    int next_node = liste->numero_sommet;
    int val = -negamax_helper(g, next_node, depth - 1, max_pick);

    if (val > best_val) {
      best_val = val;
      best_pick = node - next_node;
      tie_count = 1;
    } else if (val == best_val) {
      tie_count++;
      if (rand() % tie_count == 0)
        best_pick = node - next_node;
    }
    liste = liste->next;
  }
  return best_pick;
}

/*
 * MCTS : distribue nb_simulations = ai_difficulty + 1 simulations
 * en round-robin sur les coups disponibles.
 *   difficulty=0   → 1   simulation  → quasi-aléatoire
 *   difficulty=50  → 51  simulations → stratégie partielle
 *   difficulty=100 → 101 simulations → quasi-optimal
 */
int AI_pick_mcts(graphe_t *g, int node, GameState *state) {
  if (g->listes[node] == NULL)
    return 1;

  int nb_simulations = state->ai_difficulty + 1;

  int nb_moves = 0;
  liste_t liste = g->listes[node];
  while (liste != NULL) {
    nb_moves++;
    liste = liste->next;
  }

  int *wins = (int *)calloc(nb_moves, sizeof(int));
  int *plays = (int *)calloc(nb_moves, sizeof(int));
  if (!wins || !plays) {
    free(wins);
    free(plays);
    return 1;
  }

  for (int i = 0; i < nb_simulations; i++) {
    int move_idx = i % nb_moves;
    liste_t l = g->listes[node];
    for (int j = 0; j < move_idx; j++)
      l = l->next;
    int next_node = l->numero_sommet;

    int result = -mcts_playout(g, next_node);
    plays[move_idx]++;
    if (result > 0)
      wins[move_idx]++;
  }

  int best_idx = 0;
  float best_rate = -1.0f;
  for (int i = 0; i < nb_moves; i++) {
    float rate = (plays[i] > 0) ? (float)wins[i] / (float)plays[i] : 0.0f;
    if (rate > best_rate) {
      best_rate = rate;
      best_idx = i;
    }
  }

  liste_t l = g->listes[node];
  for (int i = 0; i < best_idx; i++)
    l = l->next;
  int best_pick = node - l->numero_sommet;

  free(wins);
  free(plays);
  return best_pick;
}

/*
 * RECUIT SIMULÉ : compare le coup optimal (via noyau) à un coup aléatoire,
 * accepte le sous-optimal avec probabilité P = e^(-Δ/T).
 * Température T = (100 - ai_difficulty) / 20.0 + 0.01
 *   difficulty=0   → T≈5.01 → permissif (presque aléatoire)
 *   difficulty=100 → T≈0.01 → quasi-optimal
 */
int AI_pick_sa(graphe_t *g, int node, GameState *state) {
  if (g->listes[node] == NULL)
    return 1;

  float temperature = (100.0f - (float)state->ai_difficulty) / 20.0f + 0.01f;
  int max_pick = get_max_pick(g);

  int optimal_node = jouer_coup(g, node);
  bool has_optimal = (optimal_node < g->nbr_sommets);
  int optimal_val =
      has_optimal ? -negamax_helper(g, optimal_node, 2, max_pick) : -1;
  int optimal_pick = has_optimal ? (node - optimal_node) : 1;

  int nb_moves = 0;
  liste_t liste = g->listes[node];
  while (liste != NULL) {
    nb_moves++;
    liste = liste->next;
  }

  int r = rand() % nb_moves;
  liste = g->listes[node];
  for (int i = 0; i < r; i++)
    liste = liste->next;
  int candidate_node = liste->numero_sommet;
  int candidate_val = -negamax_helper(g, candidate_node, 2, max_pick);
  int candidate_pick = node - candidate_node;

  float delta = (float)(optimal_val - candidate_val);
  if (delta <= 0.0f)
    return candidate_pick;
  if (temperature < 0.001f)
    return optimal_pick;

  float prob = expf(-delta / temperature);
  float rand_unit = (float)rand() / (float)RAND_MAX;
  return (rand_unit < prob) ? candidate_pick : optimal_pick;
}

/*
 * Dispatcher principal : choisit la stratégie selon state->ai_strategy.
 */
int AI_pick(graphe_t *g, int node, GameState *state) {
  static int seeded = 0;
  if (!seeded) {
    srand(time(NULL));
    seeded = 1;
  }

  switch (state->ai_strategy) {
  case AI_STRATEGY_MIXED:
    return Strat_optimale(g, node, state);
  case AI_STRATEGY_COPIE:
    return Strat_copie(state->last_pick);
  case AI_STRATEGY_MINIMAX:
    return AI_pick_minimax(g, node, state);
  case AI_STRATEGY_MCTS:
    return AI_pick_mcts(g, node, state);
  case AI_STRATEGY_SA:
    return AI_pick_sa(g, node, state);
  default:
    return Strat_optimale(g, node, state);
  }
}
