/*
 * statistique.c - Statistiques et palmares
 * Projet ShopFlow - Module XTI101
 *
 * Les classements utilisent des tableaux temporaires alloues
 * dynamiquement, tries par un tri a bulles, puis liberes.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "statistique.h"
#include "client.h"
#include "produit.h"
#include "utils.h"

#define TAILLE_TOP_PRODUITS 5

/* Element du palmares produits */
typedef struct {
    int indiceProduit;
    int quantiteVendue;
} StatProduit;

/* Element du palmares clients */
typedef struct {
    int indiceClient;
    float caHT;
    float caTTC;
} StatClient;

/* Element des statistiques par categorie */
typedef struct {
    char nom[TAILLE_CATEGORIE];
    int nbProduits;
    int quantiteVendue;
    float caHT;
} StatCategorie;

static int estAnnulee(const Commande *c)
{
    return strcmp(c->statut, STATUT_ANNULEE) == 0;
}

/*
 * Additionne le CA des commandes non annulees sur une periode.
 * mois = 0 : toute l'annee ; annee = 0 : toutes les annees.
 */
static void cumulerChiffreAffaires(const Boutique *b, int mois, int annee,
                                   float *totalHT, float *totalTTC, int *nbCommandes)
{
    int i, j;
    const Commande *c;

    *totalHT = 0;
    *totalTTC = 0;
    *nbCommandes = 0;
    for (i = 0; i < b->nbClients; i++) {
        for (j = 0; j < b->clients[i].nbCommandes; j++) {
            c = &b->clients[i].commandes[j];
            if (estAnnulee(c)) {
                continue;
            }
            if (annee != 0 && c->dateCommande.annee != annee) {
                continue;
            }
            if (mois != 0 && c->dateCommande.mois != mois) {
                continue;
            }
            *totalHT += c->montantHT;
            *totalTTC += c->montantTTC;
            (*nbCommandes)++;
        }
    }
}

/* CA global de la boutique avec le detail remise / TVA / port */
void afficherChiffreAffaires(const Boutique *b)
{
    int i, j;
    int nbCommandes = 0, nbAnnulees = 0;
    float brut = 0, remises = 0, ht = 0, tva = 0, port = 0, ttc = 0;
    const Commande *c;

    afficherTitre("CHIFFRE D'AFFAIRES GLOBAL");
    for (i = 0; i < b->nbClients; i++) {
        for (j = 0; j < b->clients[i].nbCommandes; j++) {
            c = &b->clients[i].commandes[j];
            if (estAnnulee(c)) {
                nbAnnulees++;
                continue;
            }
            brut += c->montantBrutHT;
            remises += c->montantRemise;
            ht += c->montantHT;
            tva += c->montantTVA;
            port += c->fraisPort;
            ttc += c->montantTTC;
            nbCommandes++;
        }
    }

    printf("Commandes prises en compte : %d (%d annulee(s) exclue(s))\n\n", nbCommandes, nbAnnulees);
    printf("  Montant brut HT      : %12.2f euros\n", brut);
    printf("  Remises accordees    : %12.2f euros\n", -remises);
    printf("  CHIFFRE D'AFFAIRES HT: %12.2f euros\n", ht);
    printf("  TVA collectee        : %12.2f euros\n", tva);
    printf("  Frais de port        : %12.2f euros\n", port);
    printf("  CHIFFRE D'AFFAIRES TTC: %11.2f euros\n", ttc);
    if (nbCommandes > 0) {
        printf("\n  Panier moyen         : %12.2f euros TTC\n", ttc / nbCommandes);
    }
}

/* CA sur un mois precis, ou sur une annee avec le detail mois par mois */
void afficherChiffreAffairesPeriode(const Boutique *b)
{
    int choix, mois, annee, nb;
    float ht, ttc;
    const char *nomsMois[12] = {"Janvier", "Fevrier", "Mars", "Avril", "Mai", "Juin", "Juillet",
                                "Aout", "Septembre", "Octobre", "Novembre", "Decembre"};

    afficherTitre("CHIFFRE D'AFFAIRES PAR PERIODE");
    printf("1. Pour un mois precis\n");
    printf("2. Pour une annee (detail mois par mois)\n");
    choix = saisirEntier("Votre choix : ", 1, 2);

    if (choix == 1) {
        mois = saisirEntier("Mois (1-12) : ", 1, 12);
        annee = saisirEntier("Annee (2000-2100) : ", 2000, 2100);
        cumulerChiffreAffaires(b, mois, annee, &ht, &ttc, &nb);
        printf("\n%s %d : %d commande(s), CA HT = %.2f euros, CA TTC = %.2f euros\n",
               nomsMois[mois - 1], annee, nb, ht, ttc);
        return;
    }

    annee = saisirEntier("Annee (2000-2100) : ", 2000, 2100);
    printf("\n%-12s %10s %14s %14s\n", "Mois", "Commandes", "CA HT", "CA TTC");
    afficherSeparateur();
    for (mois = 1; mois <= 12; mois++) {
        cumulerChiffreAffaires(b, mois, annee, &ht, &ttc, &nb);
        printf("%-12s %10d %14.2f %14.2f\n", nomsMois[mois - 1], nb, ht, ttc);
    }
    afficherSeparateur();
    cumulerChiffreAffaires(b, 0, annee, &ht, &ttc, &nb);
    printf("%-12s %10d %14.2f %14.2f\n", "TOTAL", nb, ht, ttc);
}

/* Quantite totale vendue d'un produit (lignes des commandes non annulees) */
static int quantiteVendueProduit(const Boutique *b, const char *reference)
{
    int i, j, k;
    int total = 0;
    const Commande *c;

    for (i = 0; i < b->nbClients; i++) {
        for (j = 0; j < b->clients[i].nbCommandes; j++) {
            c = &b->clients[i].commandes[j];
            if (estAnnulee(c)) {
                continue;
            }
            for (k = 0; k < c->nbLignes; k++) {
                if (strcmp(c->lignes[k].refProduit, reference) == 0) {
                    total += c->lignes[k].quantite;
                }
            }
        }
    }
    return total;
}

/* Top 5 des produits par quantite vendue */
void afficherPalmaresProduits(const Boutique *b)
{
    StatProduit *stats;
    StatProduit temp;
    int i, j, rang;
    int nbAffiches = 0;
    const Produit *p;

    afficherTitre("PALMARES PRODUITS - TOP 5");
    if (b->nbProduits == 0) {
        printf("Le catalogue est vide.\n");
        return;
    }

    stats = (StatProduit *)malloc(b->nbProduits * sizeof(StatProduit));
    if (stats == NULL) {
        printf("Erreur : memoire insuffisante pour le classement.\n");
        return;
    }

    for (i = 0; i < b->nbProduits; i++) {
        stats[i].indiceProduit = i;
        stats[i].quantiteVendue = quantiteVendueProduit(b, b->produits[i].reference);
    }

    /* Tri a bulles par quantite decroissante */
    for (i = 0; i < b->nbProduits - 1; i++) {
        for (j = 0; j < b->nbProduits - 1 - i; j++) {
            if (stats[j].quantiteVendue < stats[j + 1].quantiteVendue) {
                temp = stats[j];
                stats[j] = stats[j + 1];
                stats[j + 1] = temp;
            }
        }
    }

    printf("%-5s %-10s %-26s %-14s %10s\n", "Rang", "Reference", "Designation", "Categorie", "Qte vendue");
    afficherSeparateur();
    for (i = 0; i < b->nbProduits && nbAffiches < TAILLE_TOP_PRODUITS; i++) {
        if (stats[i].quantiteVendue == 0) {
            break;   /* le tableau est trie : les suivants n'ont rien vendu non plus */
        }
        rang = i + 1;
        p = &b->produits[stats[i].indiceProduit];
        printf("%-5d %-10s %-26.26s %-14.14s %10d\n", rang, p->reference,
               p->designation, p->categorie, stats[i].quantiteVendue);
        nbAffiches++;
    }
    if (nbAffiches == 0) {
        printf("Aucune vente enregistree.\n");
    }

    free(stats);
}

/* Classement des clients par chiffre d'affaires genere */
void afficherPalmaresClients(const Boutique *b)
{
    StatClient *stats;
    StatClient temp;
    int i, j;
    const Client *c;
    char nomComplet[2 * TAILLE_NOM];

    afficherTitre("PALMARES CLIENTS");
    if (b->nbClients == 0) {
        printf("Aucun client enregistre.\n");
        return;
    }

    stats = (StatClient *)malloc(b->nbClients * sizeof(StatClient));
    if (stats == NULL) {
        printf("Erreur : memoire insuffisante pour le classement.\n");
        return;
    }

    for (i = 0; i < b->nbClients; i++) {
        stats[i].indiceClient = i;
        stats[i].caHT = chiffreAffairesClientHT(&b->clients[i]);
        stats[i].caTTC = chiffreAffairesClientTTC(&b->clients[i]);
    }

    /* Tri a bulles par CA HT decroissant */
    for (i = 0; i < b->nbClients - 1; i++) {
        for (j = 0; j < b->nbClients - 1 - i; j++) {
            if (stats[j].caHT < stats[j + 1].caHT) {
                temp = stats[j];
                stats[j] = stats[j + 1];
                stats[j + 1] = temp;
            }
        }
    }

    printf("%-5s %-4s %-24s %10s %12s %12s\n", "Rang", "ID", "Client", "Commandes", "CA HT", "CA TTC");
    afficherSeparateur();
    for (i = 0; i < b->nbClients; i++) {
        c = &b->clients[stats[i].indiceClient];
        snprintf(nomComplet, sizeof(nomComplet), "%s %s", c->prenom, c->nom);
        printf("%-5d %-4d %-24.24s %10d %12.2f %12.2f%s\n", i + 1, c->idClient, nomComplet,
               nbCommandesNonAnnulees(c), stats[i].caHT, stats[i].caTTC,
               c->actif ? "" : "  (supprime)");
    }

    free(stats);
}

/*
 * Fonctionnalite creative n 2 : statistiques par categorie.
 * Le tableau des categories n'a pas de taille connue a l'avance :
 * il est agrandi avec realloc a chaque nouvelle categorie rencontree.
 * Le CA HT d'une ligne tient compte de la remise de sa commande.
 */
void afficherStatistiquesCategories(const Boutique *b)
{
    StatCategorie *categories = NULL;
    StatCategorie *nouveauTableau;
    StatCategorie temp;
    int nbCategories = 0;
    int i, j, k, n, indiceProduit, indiceCategorie;
    const Commande *c;
    const char *nomCategorie;
    float totalCA = 0;

    afficherTitre("STATISTIQUES PAR CATEGORIE");

    /* 1. Liste des categories presentes dans le catalogue */
    for (i = 0; i < b->nbProduits; i++) {
        indiceCategorie = -1;
        for (n = 0; n < nbCategories; n++) {
            if (strcmp(categories[n].nom, b->produits[i].categorie) == 0) {
                indiceCategorie = n;
            }
        }
        if (indiceCategorie == -1) {
            nouveauTableau = (StatCategorie *)realloc(categories, (nbCategories + 1) * sizeof(StatCategorie));
            if (nouveauTableau == NULL) {
                printf("Erreur : memoire insuffisante.\n");
                free(categories);
                return;
            }
            categories = nouveauTableau;
            strcpy(categories[nbCategories].nom, b->produits[i].categorie);
            categories[nbCategories].nbProduits = 0;
            categories[nbCategories].quantiteVendue = 0;
            categories[nbCategories].caHT = 0;
            indiceCategorie = nbCategories;
            nbCategories++;
        }
        if (b->produits[i].actif) {
            categories[indiceCategorie].nbProduits++;
        }
    }

    if (nbCategories == 0) {
        printf("Le catalogue est vide.\n");
        return;
    }

    /* 2. Ventes : on rattache chaque ligne a la categorie de son produit */
    for (i = 0; i < b->nbClients; i++) {
        for (j = 0; j < b->clients[i].nbCommandes; j++) {
            c = &b->clients[i].commandes[j];
            if (estAnnulee(c)) {
                continue;
            }
            for (k = 0; k < c->nbLignes; k++) {
                indiceProduit = rechercherIndiceProduit(b, c->lignes[k].refProduit);
                if (indiceProduit == -1) {
                    continue;
                }
                nomCategorie = b->produits[indiceProduit].categorie;
                for (n = 0; n < nbCategories; n++) {
                    if (strcmp(categories[n].nom, nomCategorie) == 0) {
                        categories[n].quantiteVendue += c->lignes[k].quantite;
                        categories[n].caHT += c->lignes[k].sousTotalHT * (1.0f - c->tauxRemise / 100.0f);
                    }
                }
            }
        }
    }

    /* 3. Tri a bulles par CA decroissant */
    for (i = 0; i < nbCategories - 1; i++) {
        for (j = 0; j < nbCategories - 1 - i; j++) {
            if (categories[j].caHT < categories[j + 1].caHT) {
                temp = categories[j];
                categories[j] = categories[j + 1];
                categories[j + 1] = temp;
            }
        }
    }

    for (i = 0; i < nbCategories; i++) {
        totalCA += categories[i].caHT;
    }

    printf("%-16s %9s %11s %12s %8s\n", "Categorie", "Produits", "Qte vendue", "CA HT", "Part");
    afficherSeparateur();
    for (i = 0; i < nbCategories; i++) {
        printf("%-16.16s %9d %11d %12.2f %7.1f%%\n", categories[i].nom, categories[i].nbProduits,
               categories[i].quantiteVendue, categories[i].caHT,
               totalCA > 0 ? categories[i].caHT * 100.0f / totalCA : 0.0f);
    }
    afficherSeparateur();
    printf("%-16s %9s %11s %12.2f\n", "TOTAL", "", "", totalCA);
    printf("(CA HT apres remise, reparti au prorata des lignes)\n");

    free(categories);
}
