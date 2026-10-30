/* ---------- Sous-menu Produits ---------- */
#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "utils.h"
#include "produit.h"
#include "client.h"
#include "commande.h"
#include "statistique.h"
#include "initialisation.h"
#include "historique.h"

static void menuProduits(Boutique *b)
{
    int choix;
    do {
        afficherTitre("GESTION DU CATALOGUE");
        printf("1. Ajouter un produit\n");
        printf("2. Modifier un produit (prix, stock, seuil)\n");
        printf("3. Supprimer un produit\n");
        printf("4. Afficher le catalogue\n");
        printf("5. Rechercher un produit\n");
        printf("6. Alertes de stock\n");
        printf("0. Retour\n");
        choix = saisirEntier("Votre choix : ", 0, 6);

        switch (choix) {
            case 1: ajouterProduit(b);       break;
            case 2: modifierProduit(b);      break;
            case 3: supprimerProduit(b);     break;
            case 4: afficherProduits(b);     break;
            case 5: rechercherProduit(b);    break;
            case 6: afficherAlertesStock(b); break;
            default: break;
        }
        if (choix != 0) {
            pause();
        }
    } while (choix != 0);
}

/* ---------- Sous-menu Clients ---------- */
static void menuClients(Boutique *b)
{
    int choix;
    do {
        afficherTitre("GESTION DES CLIENTS");
        printf("1. Ajouter un client\n");
        printf("2. Modifier un client\n");
        printf("3. Supprimer un client\n");
        printf("4. Afficher la liste des clients\n");
        printf("5. Afficher la fiche d'un client\n");
        printf("0. Retour\n");
        choix = saisirEntier("Votre choix : ", 0, 5);

        switch (choix) {
            case 1: ajouterClient(b);       break;
            case 2: modifierClient(b);      break;
            case 3: supprimerClient(b);     break;
            case 4: afficherClients(b);     break;
            case 5: afficherFicheClient(b); break;
            default: break;
        }
        if (choix != 0) {
            pause();
        }
    } while (choix != 0);
}

/* ---------- Sous-menu Commandes ---------- */
static void menuCommandes(Boutique *b)
{
    int choix;
    do {
        afficherTitre("GESTION DES COMMANDES");
        printf("1. Creer une commande\n");
        printf("2. Ajouter une ligne (commande EN_COURS)\n");
        printf("3. Supprimer une ligne (commande EN_COURS)\n");
        printf("4. Faire avancer le statut d'une commande\n");
        printf("5. Annuler une commande\n");
        printf("6. Afficher le detail d'une commande\n");
        printf("7. Lister toutes les commandes\n");
        printf("0. Retour\n");
        choix = saisirEntier("Votre choix : ", 0, 7);

        switch (choix) {
            case 1: creerCommande(b);              break;
            case 2: ajouterLigneCommande(b);       break;
            case 3: supprimerLigneCommande(b);     break;
            case 4: changerStatutCommande(b);      break;
            case 5: annulerCommande(b);            break;
            case 6: afficherCommandeParNumero(b);  break;
            case 7: afficherToutesLesCommandes(b); break;
            default: break;
        }
        if (choix != 0) {
            pause();
        }
    } while (choix != 0);
}

/* ---------- Sous-menu Statistiques ---------- */
static void menuStatistiques(Boutique *b)
{
    int choix;
    do {
        afficherTitre("STATISTIQUES");
        printf("1. Chiffre d'affaires global\n");
        printf("2. Chiffre d'affaires par periode (mois / annee)\n");
        printf("3. Palmares produits (top 5)\n");
        printf("4. Palmares clients\n");
        printf("5. Alertes de stock\n");
        printf("6. Statistiques par categorie\n");
        printf("0. Retour\n");
        choix = saisirEntier("Votre choix : ", 0, 6);

        switch (choix) {
            case 1: afficherChiffreAffaires(b);        break;
            case 2: afficherChiffreAffairesPeriode(b); break;
            case 3: afficherPalmaresProduits(b);       break;
            case 4: afficherPalmaresClients(b);        break;
            case 5: afficherAlertesStock(b);           break;
            case 6: afficherStatistiquesCategories(b); break;
            default: break;
        }
        if (choix != 0) {
            pause();
        }
    } while (choix != 0);
}

int main()
{
    Boutique boutique;
    int choix;

    initialiserBoutique(&boutique);
    if (!initialiserDonneesTest(&boutique)) {
        printf("Erreur lors du chargement des donnees de demonstration.\n");
        detruireBoutique(&boutique);
        return EXIT_FAILURE;
    }

    printf("\n******************************************\n");
    printf("*      Bienvenue dans %-18s *\n", boutique.nom);
    printf("******************************************\n");
    printf("Donnees chargees : %d produits, %d clients.\n", boutique.nbProduits, boutique.nbClients);

    do {
        afficherTitre("MENU PRINCIPAL");
        printf("1. Catalogue produits\n");
        printf("2. Clients\n");
        printf("3. Commandes\n");
        printf("4. Statistiques\n");
        printf("5. Historique des operations\n");
        printf("0. Quitter\n");
        choix = saisirEntier("Votre choix : ", 0, 5);

        switch (choix) {
            case 1: menuProduits(&boutique);     break;
            case 2: menuClients(&boutique);      break;
            case 3: menuCommandes(&boutique);    break;
            case 4: menuStatistiques(&boutique); break;
            case 5:
                afficherHistorique(&boutique);
                pause();
                break;
            default: break;
        }
    } while (choix != 0);

    /* Liberation de toute la memoire allouee avant de quitter */
    detruireBoutique(&boutique);
    printf("Memoire liberee. Au revoir !\n");
    return EXIT_SUCCESS;
}
