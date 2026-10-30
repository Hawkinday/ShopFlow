/*
 * historique.c - Journal des operations (fonctionnalite creative n 1)
 * Projet ShopFlow - Module XTI101
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "historique.h"
#include "utils.h"

/* Ajoute une operation a la fin du journal (le tableau grandit si besoin) */
void ajouterHistorique(Boutique *b, const char *texte)
{
    Operation *nouveauTableau;
    Operation *operation;
    int nouvelleCapacite;
    time_t maintenant;
    struct tm *heure;

    /* Tableau plein : on double sa capacite */
    if (b->nbOperations == b->capaciteOperations) {
        if (b->capaciteOperations == 0) {
            nouvelleCapacite = 8;
        } else {
            nouvelleCapacite = b->capaciteOperations * 2;
        }
        nouveauTableau = (Operation *)realloc(b->historique, nouvelleCapacite * sizeof(Operation));
        if (nouveauTableau == NULL) {
            /* L'ancien tableau reste valide : on perd juste cette operation */
            printf("  Attention : memoire insuffisante, operation non historisee.\n");
            return;
        }
        b->historique = nouveauTableau;
        b->capaciteOperations = nouvelleCapacite;
    }

    operation = &b->historique[b->nbOperations];
    operation->numero = b->nbOperations + 1;

    maintenant = time(NULL);
    heure = localtime(&maintenant);
    if (heure != NULL) {
        strftime(operation->heure, sizeof(operation->heure), "%H:%M:%S", heure);
    } else {
        strcpy(operation->heure, "--:--:--");
    }

    /* snprintf coupe proprement le texte s'il est trop long */
    snprintf(operation->texte, sizeof(operation->texte), "%s", texte);
    b->nbOperations++;
}

void afficherHistorique(const Boutique *b)
{
    int i;

    afficherTitre("HISTORIQUE DES OPERATIONS");
    if (b->nbOperations == 0) {
        printf("Aucune operation enregistree.\n");
        return;
    }
    printf("%-5s %-9s %s\n", "N", "Heure", "Operation");
    afficherSeparateur();
    for (i = 0; i < b->nbOperations; i++) {
        printf("%-5d %-9s %s\n", b->historique[i].numero,
               b->historique[i].heure, b->historique[i].texte);
    }
    printf("\n%d operation(s) enregistree(s).\n", b->nbOperations);
}
