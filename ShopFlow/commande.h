/*
 * commande.h - Gestion des commandes
 * Projet ShopFlow - Module XTI101
 *
 * Creation des commandes, gestion des lignes, calcul des montants,
 * changement de statut et annulation.
 */

#ifndef COMMANDE_H
#define COMMANDE_H

#include "types.h"

/* Calculs (regles metier de la section 2) */
float tauxRemisePourMontant(float montantBrutHT);
void  calculCommande(Commande *c);
int   calculPointsFidelite(const Commande *c);

/* Manipulation des lignes (utilisee aussi par initialisation.c) */
int  initialiserCommande(Commande *c, int capaciteLignes);
int  rechercherIndiceLigne(const Commande *c, const char *refProduit);
int  ajouterLigneDansCommande(Commande *c, const Produit *p, int quantite);

/* Recherche d'une commande dans toute la boutique */
Commande *rechercherCommande(Boutique *b, int numero, int *indiceClient);

/* Affichages */
void afficherCommande(const Commande *c);
void afficherEnteteResumeCommande(void);
void afficherResumeCommande(const Commande *c);

/* Fonctionnalites du menu */
void creerCommande(Boutique *b);
void ajouterLigneCommande(Boutique *b);
void supprimerLigneCommande(Boutique *b);
void changerStatutCommande(Boutique *b);
void annulerCommande(Boutique *b);
void afficherCommandeParNumero(Boutique *b);
void afficherToutesLesCommandes(const Boutique *b);

#endif
