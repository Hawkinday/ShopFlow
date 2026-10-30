/*
 * produit.c - Gestion du catalogue produits
 * Projet ShopFlow - Module XTI101
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "produit.h"
#include "utils.h"
#include "historique.h"

/*
 * Retourne l'indice du produit dans le catalogue, ou -1 s'il n'existe pas.
 * La recherche porte sur TOUS les produits (meme supprimes) pour garantir
 * l'unicite des references.
 */
int rechercherIndiceProduit(const Boutique *b, const char *reference)
{
    int i;
    for (i = 0; i < b->nbProduits; i++) {
        if (strcmp(b->produits[i].reference, reference) == 0) {
            return i;
        }
    }
    return -1;
}

/* Double la capacite du catalogue. Retourne 1 si OK, 0 si erreur memoire */
static int agrandirCatalogue(Boutique *b)
{
    int nouvelleCapacite;
    Produit *nouveauTableau;

    if (b->capaciteProduits == 0) {
        nouvelleCapacite = CAPACITE_INITIALE;
    } else {
        nouvelleCapacite = b->capaciteProduits * 2;
    }

    /* On passe par un pointeur temporaire : si realloc echoue,
       l'ancien tableau n'est pas perdu */
    nouveauTableau = (Produit *)realloc(b->produits, nouvelleCapacite * sizeof(Produit));
    if (nouveauTableau == NULL) {
        printf("Erreur : memoire insuffisante pour agrandir le catalogue.\n");
        return 0;
    }
    b->produits = nouveauTableau;
    b->capaciteProduits = nouvelleCapacite;
    return 1;
}

/* Ajoute un produit (deja rempli) a la fin du catalogue */
int ajouterProduitAuCatalogue(Boutique *b, Produit p)
{
    if (b->nbProduits == b->capaciteProduits) {
        if (!agrandirCatalogue(b)) {
            return 0;
        }
    }
    b->produits[b->nbProduits] = p;
    b->nbProduits++;
    return 1;
}

/* Retourne 1 si le produit apparait dans au moins une commande non annulee */
int produitUtiliseDansCommande(const Boutique *b, const char *reference)
{
    int i, j, k;
    const Commande *cmd;

    for (i = 0; i < b->nbClients; i++) {
        for (j = 0; j < b->clients[i].nbCommandes; j++) {
            cmd = &b->clients[i].commandes[j];
            if (strcmp(cmd->statut, STATUT_ANNULEE) == 0) {
                continue;
            }
            for (k = 0; k < cmd->nbLignes; k++) {
                if (strcmp(cmd->lignes[k].refProduit, reference) == 0) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

/* Affichage d'un produit sous forme de ligne de tableau */
static void afficherEnteteProduits(void)
{
    printf("%-10s %-22s %-14s %9s %6s %6s %11s\n",
           "Reference", "Designation", "Categorie", "Prix HT", "Stock", "Seuil", "Valeur HT");
    afficherSeparateur();
}

static void afficherLigneProduit(const Produit *p)
{
    printf("%-10s %-22.22s %-14.14s %9.2f %6d %6d %11.2f\n",
           p->reference, p->designation, p->categorie, p->prixUnitaireHT,
           p->quantiteStock, p->seuilAlerte, p->prixUnitaireHT * p->quantiteStock);
}

/* Demande une reference et retourne l'indice d'un produit ACTIF, ou -1 */
static int demanderProduitActif(const Boutique *b)
{
    char reference[TAILLE_REF];
    int indice;

    saisirChaine("Reference du produit : ", reference, TAILLE_REF);
    indice = rechercherIndiceProduit(b, reference);
    if (indice == -1 || b->produits[indice].actif == 0) {
        printf("Erreur : aucun produit actif avec la reference %s.\n", reference);
        return -1;
    }
    return indice;
}

void ajouterProduit(Boutique *b)
{
    Produit p;
    char texte[200];

    afficherTitre("AJOUT D'UN PRODUIT");
    saisirChaine("Reference (ex : PRD-0009) : ", p.reference, TAILLE_REF);

    /* Controle d'unicite, y compris sur les produits supprimes */
    if (rechercherIndiceProduit(b, p.reference) != -1) {
        printf("Erreur : la reference %s existe deja dans le catalogue.\n", p.reference);
        return;
    }

    saisirChaine("Designation : ", p.designation, TAILLE_DESIGNATION);
    saisirChaine("Categorie : ", p.categorie, TAILLE_CATEGORIE);
    p.prixUnitaireHT = arrondir(saisirFloat("Prix unitaire HT : ", 0.01f, 100000.0f));
    p.quantiteStock = saisirEntier("Quantite en stock : ", 0, 1000000);
    p.seuilAlerte = saisirEntier("Seuil d'alerte : ", 0, 1000000);
    p.actif = 1;

    if (ajouterProduitAuCatalogue(b, p)) {
        printf("Produit %s ajoute au catalogue (%d produits).\n", p.reference, b->nbProduits);
        snprintf(texte, sizeof(texte), "Ajout du produit %s (%s)", p.reference, p.designation);
        ajouterHistorique(b, texte);
    }
}

void modifierProduit(Boutique *b)
{
    int indice, choix;
    Produit *p;
    char texte[200];

    afficherTitre("MODIFICATION D'UN PRODUIT");
    indice = demanderProduitActif(b);
    if (indice == -1) {
        return;
    }
    p = &b->produits[indice];

    do {
        printf("\n");
        afficherEnteteProduits();
        afficherLigneProduit(p);
        printf("\n1. Modifier le prix unitaire HT\n");
        printf("2. Modifier la quantite en stock\n");
        printf("3. Modifier le seuil d'alerte\n");
        printf("0. Terminer\n");
        choix = saisirEntier("Votre choix : ", 0, 3);

        switch (choix) {
            case 1:
                p->prixUnitaireHT = arrondir(saisirFloat("Nouveau prix HT : ", 0.01f, 100000.0f));
                printf("Prix modifie (les commandes deja passees gardent l'ancien prix).\n");
                break;
            case 2:
                p->quantiteStock = saisirEntier("Nouvelle quantite en stock : ", 0, 1000000);
                printf("Stock modifie.\n");
                break;
            case 3:
                p->seuilAlerte = saisirEntier("Nouveau seuil d'alerte : ", 0, 1000000);
                printf("Seuil modifie.\n");
                break;
            default:
                break;
        }
    } while (choix != 0);

    snprintf(texte, sizeof(texte), "Modification du produit %s", p->reference);
    ajouterHistorique(b, texte);
}

/* Suppression logique : le produit reste en memoire avec actif = 0 */
void supprimerProduit(Boutique *b)
{
    int indice;
    Produit *p;
    char texte[200];

    afficherTitre("SUPPRESSION D'UN PRODUIT");
    indice = demanderProduitActif(b);
    if (indice == -1) {
        return;
    }
    p = &b->produits[indice];

    if (produitUtiliseDansCommande(b, p->reference)) {
        printf("Suppression refusee : %s figure dans au moins une commande non annulee.\n",
               p->reference);
        return;
    }

    printf("Produit : %s - %s\n", p->reference, p->designation);
    if (saisirOuiNon("Confirmer la suppression")) {
        p->actif = 0;
        printf("Produit %s supprime.\n", p->reference);
        snprintf(texte, sizeof(texte), "Suppression du produit %s", p->reference);
        ajouterHistorique(b, texte);
    } else {
        printf("Suppression abandonnee.\n");
    }
}

/* Affiche le catalogue des produits actifs avec la valorisation du stock */
void afficherProduits(const Boutique *b)
{
    int i;
    int nbAffiches = 0;
    float valeurTotale = 0;

    afficherTitre("CATALOGUE DES PRODUITS");
    afficherEnteteProduits();
    for (i = 0; i < b->nbProduits; i++) {
        if (b->produits[i].actif) {
            afficherLigneProduit(&b->produits[i]);
            valeurTotale += b->produits[i].prixUnitaireHT * b->produits[i].quantiteStock;
            nbAffiches++;
        }
    }
    afficherSeparateur();
    printf("%d produit(s) actif(s) - Valorisation totale du stock : %.2f euros HT\n",
           nbAffiches, valeurTotale);
    if (nbAffiches < b->nbProduits) {
        printf("(%d produit(s) supprime(s) non affiche(s))\n", b->nbProduits - nbAffiches);
    }
}

/* Recherche par reference exacte ou par mot-cle dans la designation */
void rechercherProduit(const Boutique *b)
{
    int choix, i, indice;
    int nbTrouves = 0;
    char motCle[TAILLE_DESIGNATION];
    char motCleMin[TAILLE_DESIGNATION];
    char designationMin[TAILLE_DESIGNATION];

    afficherTitre("RECHERCHE D'UN PRODUIT");
    printf("1. Par reference exacte\n");
    printf("2. Par mot-cle dans la designation\n");
    choix = saisirEntier("Votre choix : ", 1, 2);

    if (choix == 1) {
        indice = demanderProduitActif(b);
        if (indice != -1) {
            afficherEnteteProduits();
            afficherLigneProduit(&b->produits[indice]);
        }
        return;
    }

    /* Recherche sans tenir compte des majuscules/minuscules */
    saisirChaine("Mot-cle : ", motCle, TAILLE_DESIGNATION);
    mettreEnMinuscules(motCle, motCleMin, TAILLE_DESIGNATION);

    afficherEnteteProduits();
    for (i = 0; i < b->nbProduits; i++) {
        if (!b->produits[i].actif) {
            continue;
        }
        mettreEnMinuscules(b->produits[i].designation, designationMin, TAILLE_DESIGNATION);
        if (strstr(designationMin, motCleMin) != NULL) {
            afficherLigneProduit(&b->produits[i]);
            nbTrouves++;
        }
    }
    printf("%d produit(s) trouve(s) pour \"%s\".\n", nbTrouves, motCle);
}

/* Produits actifs dont le stock est inferieur ou egal au seuil d'alerte */
void afficherAlertesStock(const Boutique *b)
{
    int i;
    int nbAlertes = 0;
    const Produit *p;

    afficherTitre("ALERTES DE STOCK");
    printf("%-10s %-22s %6s %6s %s\n", "Reference", "Designation", "Stock", "Seuil", "A commander");
    afficherSeparateur();
    for (i = 0; i < b->nbProduits; i++) {
        p = &b->produits[i];
        if (p->actif && p->quantiteStock <= p->seuilAlerte) {
            /* On propose de recommander de quoi revenir au double du seuil */
            printf("%-10s %-22.22s %6d %6d %d\n", p->reference, p->designation,
                   p->quantiteStock, p->seuilAlerte, 2 * p->seuilAlerte - p->quantiteStock);
            nbAlertes++;
        }
    }
    if (nbAlertes == 0) {
        printf("Aucun produit en alerte.\n");
    } else {
        printf("\n%d produit(s) a reapprovisionner.\n", nbAlertes);
    }
}
