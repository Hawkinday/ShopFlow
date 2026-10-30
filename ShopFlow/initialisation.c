/*
 * initialisation.c - Jeu de donnees de demonstration et liberation memoire
 * Projet ShopFlow - Module XTI101
 *
 * Contenu du jeu de test :
 *  - 8 produits dans 5 categories, dont 2 en alerte apres les commandes
 *    (PRD-0002 : stock 5 / seuil 15 et PRD-0007 : stock 2 / seuil 10) ;
 *  - 5 clients avec adresses completes ;
 *  - 6 commandes couvrant les 4 statuts, dont 3 de 3 lignes ou plus et
 *    une qui depasse 600 euros HT (commande 1002, remise de 15 %).
 *
 * Les stocks ci-dessous sont les stocks AVANT les commandes : la creation
 * des commandes non annulees les decremente, comme dans le vrai programme.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "initialisation.h"
#include "produit.h"
#include "client.h"
#include "commande.h"
#include "historique.h"

/* Met la boutique dans un etat vide et coherent */
void initialiserBoutique(Boutique *b)
{
    strcpy(b->nom, "ShopFlow");
    b->produits = NULL;
    b->nbProduits = 0;
    b->capaciteProduits = 0;
    b->clients = NULL;
    b->nbClients = 0;
    b->capaciteClients = 0;
    b->numeroSuivant = 1001;
    b->idClientSuivant = 1;
    b->historique = NULL;
    b->nbOperations = 0;
    b->capaciteOperations = 0;
}

/* Prepare un client de test (sans commande) */
static Client creerClientTest(const char *nom, const char *prenom, const char *email,
                              const char *telephone, const char *rue, const char *ville,
                              const char *codePostal, const char *pays)
{
    Client c;

    memset(&c, 0, sizeof(Client));
    strcpy(c.nom, nom);
    strcpy(c.prenom, prenom);
    strcpy(c.email, email);
    strcpy(c.telephone, telephone);
    strcpy(c.adresse.rue, rue);
    strcpy(c.adresse.ville, ville);
    strcpy(c.adresse.codePostal, codePostal);
    strcpy(c.adresse.pays, pays);
    c.pointsFidelite = 0;
    c.nbCommandes = 0;
    c.capaciteCommandes = 0;
    c.commandes = NULL;
    c.actif = 1;
    return c;
}

/*
 * Cree une commande de test et la rattache a son client.
 * Les lignes sont donnees par deux tableaux paralleles (references, quantites).
 * Retourne 1 si OK, 0 en cas d'erreur.
 */
static int creerCommandeTest(Boutique *b, int idClient, Date date, const char *statut,
                             const char *references[], const int quantites[], int nbLignes)
{
    Commande c;
    int i, indiceClient, indiceProduit;

    indiceClient = rechercherIndiceClient(b, idClient);
    if (indiceClient == -1) {
        printf("Erreur d'initialisation : client %d introuvable.\n", idClient);
        return 0;
    }

    /* Tableau de lignes alloue dynamiquement, a la taille exacte */
    if (!initialiserCommande(&c, nbLignes)) {
        return 0;
    }
    c.numero = b->numeroSuivant;
    c.idClient = idClient;
    c.dateCommande = date;
    strcpy(c.statut, statut);
    c.adresseLivraison = b->clients[indiceClient].adresse;

    for (i = 0; i < nbLignes; i++) {
        indiceProduit = rechercherIndiceProduit(b, references[i]);
        if (indiceProduit == -1 ||
            !ajouterLigneDansCommande(&c, &b->produits[indiceProduit], quantites[i])) {
            printf("Erreur d'initialisation : produit %s.\n", references[i]);
            free(c.lignes);
            return 0;
        }
    }
    calculCommande(&c);

    if (!ajouterCommandeAuClient(&b->clients[indiceClient], c)) {
        free(c.lignes);
        return 0;
    }
    b->numeroSuivant++;

    /* Une commande non annulee a retire du stock et rapporte des points */
    if (strcmp(statut, STATUT_ANNULEE) != 0) {
        for (i = 0; i < c.nbLignes; i++) {
            indiceProduit = rechercherIndiceProduit(b, c.lignes[i].refProduit);
            b->produits[indiceProduit].quantiteStock -= c.lignes[i].quantite;
        }
        b->clients[indiceClient].pointsFidelite += calculPointsFidelite(&c);
    }
    return 1;
}

/* Charge le jeu de donnees. Retourne 1 si OK, 0 en cas d'erreur memoire */
int initialiserDonneesTest(Boutique *b)
{
    int i;
    int ok = 1;

    /* ---------- 1. Catalogue : 10 emplacements alloues ---------- */
    b->capaciteProduits = 10;
    b->produits = (Produit *)malloc(b->capaciteProduits * sizeof(Produit));
    if (b->produits == NULL) {
        printf("Erreur : allocation du catalogue impossible.\n");
        b->capaciteProduits = 0;
        return 0;
    }
    b->nbProduits = 0;

    /* reference, designation, categorie, prix HT, stock, seuil, actif */
    Produit modele[] = {
        {"PRD-0001", "Clavier mecanique",  "Informatique", 79.90f,  42, 10, 1},
        {"PRD-0002", "Souris sans fil",    "Informatique", 24.50f,   8, 15, 1},
        {"PRD-0003", "Ecran 27 pouces",    "Informatique", 199.00f,  8,  5, 1},
        {"PRD-0004", "Casque audio",       "Audio",        89.90f,  25, 10, 1},
        {"PRD-0005", "Enceinte Bluetooth", "Audio",        59.90f,  12,  8, 1},
        {"PRD-0006", "Cable HDMI 2 m",     "Accessoires",  9.90f,  120, 30, 1},
        {"PRD-0007", "Hub USB-C 7 ports",  "Accessoires",  49.90f,   4, 10, 1},
        {"PRD-0008", "Disque SSD 1 To",    "Stockage",     109.00f, 18,  6, 1}
    };
    for (i = 0; i < 8; i++) {
        ajouterProduitAuCatalogue(b, modele[i]);
    }

    /* ---------- 2. Clients : 8 emplacements alloues ---------- */
    b->capaciteClients = 8;
    b->clients = (Client *)malloc(b->capaciteClients * sizeof(Client));
    if (b->clients == NULL) {
        printf("Erreur : allocation du repertoire clients impossible.\n");
        b->capaciteClients = 0;
        return 0;
    }
    b->nbClients = 0;

    /* Les identifiants 1 a 5 sont attribues par le compteur */
    ajouterClientAuRepertoire(b, creerClientTest("Bernard", "Julie", "j.bernard@mail.fr", "0612345678",
                                                 "12 rue de la Paix", "Paris", "75002", "France"));
    ajouterClientAuRepertoire(b, creerClientTest("Martin", "Lucas", "lucas.martin@gmail.com", "0698765432",
                                                 "5 avenue Victor Hugo", "Lyon", "69003", "France"));
    ajouterClientAuRepertoire(b, creerClientTest("Petit", "Emma", "emma.petit@outlook.fr", "0711223344",
                                                 "8 boulevard Gambetta", "Bordeaux", "33000", "France"));
    ajouterClientAuRepertoire(b, creerClientTest("Durand", "Hugo", "hugo.durand@yahoo.fr", "0655443322",
                                                 "27 rue Nationale", "Lille", "59000", "France"));
    ajouterClientAuRepertoire(b, creerClientTest("Leroy", "Chloe", "chloe.leroy@mail.fr", "0633221100",
                                                 "3 place du Capitole", "Toulouse", "31000", "France"));

    /* ---------- 3. Commandes (numeros 1001 a 1006) ---------- */

    /* 1001 - Julie - LIVREE - 3 lignes - 214.00 HT brut -> remise 5 % */
    {
        const char *refs[] = {"PRD-0001", "PRD-0002", "PRD-0006"};
        const int qtes[] = {2, 1, 3};
        Date d = {12, 3, 2026};
        ok = ok && creerCommandeTest(b, 1, d, STATUT_LIVREE, refs, qtes, 3);
    }

    /* 1002 - Lucas - LIVREE - 4 lignes - 715.80 HT brut -> remise 15 % */
    {
        const char *refs[] = {"PRD-0003", "PRD-0004", "PRD-0008", "PRD-0006"};
        const int qtes[] = {2, 1, 2, 1};
        Date d = {20, 4, 2026};
        ok = ok && creerCommandeTest(b, 2, d, STATUT_LIVREE, refs, qtes, 4);
    }

    /* 1003 - Emma - EXPEDIEE - 1 ligne - 59.90 HT brut -> frais de port factures */
    {
        const char *refs[] = {"PRD-0005"};
        const int qtes[] = {1};
        Date d = {5, 6, 2026};
        ok = ok && creerCommandeTest(b, 3, d, STATUT_EXPEDIEE, refs, qtes, 1);
    }

    /* 1004 - Julie - EXPEDIEE - 2 lignes - 229.70 HT brut -> remise 5 % */
    {
        const char *refs[] = {"PRD-0004", "PRD-0007"};
        const int qtes[] = {2, 1};
        Date d = {18, 9, 2026};
        ok = ok && creerCommandeTest(b, 1, d, STATUT_EXPEDIEE, refs, qtes, 2);
    }

    /* 1005 - Hugo - ANNULEE - 2 lignes (pas d'impact sur le stock ni les points) */
    {
        const char *refs[] = {"PRD-0008", "PRD-0001"};
        const int qtes[] = {1, 1};
        Date d = {2, 10, 2026};
        ok = ok && creerCommandeTest(b, 4, d, STATUT_ANNULEE, refs, qtes, 2);
    }

    /* 1006 - Chloe - EN_COURS - 4 lignes - 178.60 HT brut -> remise 5 % */
    {
        const char *refs[] = {"PRD-0002", "PRD-0005", "PRD-0006", "PRD-0007"};
        const int qtes[] = {2, 1, 2, 1};
        Date d = {25, 10, 2026};
        ok = ok && creerCommandeTest(b, 5, d, STATUT_EN_COURS, refs, qtes, 4);
    }

    if (ok) {
        ajouterHistorique(b, "Chargement des donnees de demonstration");
    }
    return ok;
}

/*
 * Liberation complete de la memoire, du niveau le plus profond
 * (lignes) jusqu'au plus haut (clients, produits).
 */
void detruireBoutique(Boutique *b)
{
    int i, j;

    for (i = 0; i < b->nbClients; i++) {
        for (j = 0; j < b->clients[i].nbCommandes; j++) {
            free(b->clients[i].commandes[j].lignes);
        }
        free(b->clients[i].commandes);
    }
    free(b->clients);
    free(b->produits);
    free(b->historique);

    /* On remet les pointeurs a NULL pour eviter toute double liberation */
    b->clients = NULL;
    b->produits = NULL;
    b->historique = NULL;
    b->nbClients = 0;
    b->nbProduits = 0;
    b->nbOperations = 0;
    b->capaciteClients = 0;
    b->capaciteProduits = 0;
    b->capaciteOperations = 0;
}
