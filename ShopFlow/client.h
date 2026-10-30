/*
 * client.h - Gestion des clients
 * Projet ShopFlow - Module XTI101
 *
 * CRUD des clients, adresses de livraison, points de fidelite.
 */

#ifndef CLIENT_H
#define CLIENT_H

#include "types.h"

/* Fonctions de base (utilisees aussi par les autres modules) */
int   rechercherIndiceClient(const Boutique *b, int idClient);
int   ajouterClientAuRepertoire(Boutique *b, Client c);
int   ajouterCommandeAuClient(Client *c, Commande cmd);
int   nbCommandesNonAnnulees(const Client *c);
float chiffreAffairesClientHT(const Client *c);
float chiffreAffairesClientTTC(const Client *c);
void  afficherAdresse(const Adresse *a);

/* Fonctionnalites du menu */
void ajouterClient(Boutique *b);
void modifierClient(Boutique *b);
void supprimerClient(Boutique *b);
void afficherClients(const Boutique *b);
void afficherFicheClient(const Boutique *b);

#endif
