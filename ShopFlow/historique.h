/*
 * historique.h - Journal des operations (fonctionnalite creative n 1)
 * Projet ShopFlow - Module XTI101
 *
 * Chaque action importante (ajout, suppression, commande, annulation...)
 * est enregistree dans un tableau dynamique de la Boutique.
 */

#ifndef HISTORIQUE_H
#define HISTORIQUE_H

#include "types.h"

void ajouterHistorique(Boutique *b, const char *texte);
void afficherHistorique(const Boutique *b);

#endif
