/*
 * produit.h - Gestion du catalogue produits
 * Projet ShopFlow - Module XTI101
 *
 * CRUD des produits, gestion du stock et alertes de reapprovisionnement.
 */

#ifndef PRODUIT_H
#define PRODUIT_H

#include "types.h"

/* Fonctions de base (utilisees aussi par les autres modules) */
int  rechercherIndiceProduit(const Boutique *b, const char *reference);
int  ajouterProduitAuCatalogue(Boutique *b, Produit p);
int  produitUtiliseDansCommande(const Boutique *b, const char *reference);

/* Fonctionnalites du menu */
void ajouterProduit(Boutique *b);
void modifierProduit(Boutique *b);
void supprimerProduit(Boutique *b);
void afficherProduits(const Boutique *b);
void rechercherProduit(const Boutique *b);
void afficherAlertesStock(const Boutique *b);

#endif
