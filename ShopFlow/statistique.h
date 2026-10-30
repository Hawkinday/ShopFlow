/*
 * statistique.h - Statistiques et palmares
 * Projet ShopFlow - Module XTI101
 *
 * Toutes les statistiques excluent les commandes au statut ANNULEE.
 */

#ifndef STATISTIQUE_H
#define STATISTIQUE_H

#include "types.h"

void afficherChiffreAffaires(const Boutique *b);
void afficherChiffreAffairesPeriode(const Boutique *b);
void afficherPalmaresProduits(const Boutique *b);
void afficherPalmaresClients(const Boutique *b);

/* Fonctionnalite creative n 2 */
void afficherStatistiquesCategories(const Boutique *b);

#endif
