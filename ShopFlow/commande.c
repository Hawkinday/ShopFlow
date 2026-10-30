/*
 * commande.c - Gestion des commandes
 * Projet ShopFlow - Module XTI101
 *
 * Regle importante sur le stock :
 *  - la validation d'une commande retire les quantites du stock ;
 *  - l'ajout d'une ligne sur une commande EN_COURS retire aussi du stock ;
 *  - la suppression d'une ligne ou l'annulation remet les quantites en stock.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commande.h"
#include "client.h"
#include "produit.h"
#include "utils.h"
#include "historique.h"

/* ================== CALCULS ================== */

/* Palier de remise determine sur le montant HT brut */
float tauxRemisePourMontant(float montantBrutHT)
{
    if (montantBrutHT >= 600.0f) {
        return 15.0f;
    }
    if (montantBrutHT >= 300.0f) {
        return 10.0f;
    }
    if (montantBrutHT >= 100.0f) {
        return 5.0f;
    }
    return 0.0f;
}

/*
 * Calcul en cascade de tous les montants de la commande :
 * sous-totaux -> brut HT -> remise -> HT -> TVA -> port -> TTC.
 * Chaque montant est arrondi au centime.
 */
void calculCommande(Commande *c)
{
    int i;
    float brut = 0;

    for (i = 0; i < c->nbLignes; i++) {
        c->lignes[i].sousTotalHT = arrondir(c->lignes[i].quantite * c->lignes[i].prixUnitaireHT);
        brut += c->lignes[i].sousTotalHT;
    }

    c->montantBrutHT = arrondir(brut);
    c->tauxRemise = tauxRemisePourMontant(c->montantBrutHT);
    c->montantRemise = arrondir(c->montantBrutHT * c->tauxRemise / 100.0f);
    c->montantHT = arrondir(c->montantBrutHT - c->montantRemise);
    c->montantTVA = arrondir(c->montantHT * TAUX_TVA / 100.0f);

    /* Port offert a partir de 60 euros HT brut (pas de TVA sur le port) */
    if (c->montantBrutHT >= SEUIL_PORT_GRATUIT) {
        c->fraisPort = 0.0f;
    } else {
        c->fraisPort = FRAIS_PORT;
    }

    c->montantTTC = arrondir(c->montantHT + c->montantTVA + c->fraisPort);
}

/* 1 point par tranche complete de 10 euros payee TTC (port inclus) */
int calculPointsFidelite(const Commande *c)
{
    /* le petit 0.001 evite qu'un 30.00 stocke 29.9999 donne 2 points au lieu de 3 */
    return (int)((c->montantTTC + 0.001f) / EUROS_PAR_POINT);
}

/* Ajoute (ou retire si delta < 0) des points, sans descendre sous 0 */
static void ajusterPointsClient(Client *client, int delta)
{
    client->pointsFidelite += delta;
    if (client->pointsFidelite < 0) {
        client->pointsFidelite = 0;
    }
}

/* ================== LIGNES ================== */

/* Prepare une commande vide avec un tableau de lignes alloue dynamiquement */
int initialiserCommande(Commande *c, int capaciteLignes)
{
    memset(c, 0, sizeof(Commande));
    c->lignes = (LigneCommande *)malloc(capaciteLignes * sizeof(LigneCommande));
    if (c->lignes == NULL) {
        printf("Erreur : memoire insuffisante pour creer la commande.\n");
        return 0;
    }
    c->capaciteLignes = capaciteLignes;
    c->nbLignes = 0;
    strcpy(c->statut, STATUT_EN_COURS);
    return 1;
}

int rechercherIndiceLigne(const Commande *c, const char *refProduit)
{
    int i;
    for (i = 0; i < c->nbLignes; i++) {
        if (strcmp(c->lignes[i].refProduit, refProduit) == 0) {
            return i;
        }
    }
    return -1;
}

/*
 * Ajoute une ligne a la commande. Si le produit est deja present, la
 * quantite est cumulee sur la ligne existante (une seule ligne par produit).
 * Le controle du stock est fait par la fonction appelante.
 * Retourne 1 si OK, 0 en cas d'erreur memoire.
 */
int ajouterLigneDansCommande(Commande *c, const Produit *p, int quantite)
{
    int indice;
    int nouvelleCapacite;
    LigneCommande *nouveauTableau;
    LigneCommande *ligne;

    indice = rechercherIndiceLigne(c, p->reference);
    if (indice != -1) {
        c->lignes[indice].quantite += quantite;
    } else {
        /* Tableau plein : on double sa capacite */
        if (c->nbLignes == c->capaciteLignes) {
            if (c->capaciteLignes == 0) {
                nouvelleCapacite = CAPACITE_INITIALE;
            } else {
                nouvelleCapacite = c->capaciteLignes * 2;
            }
            nouveauTableau = (LigneCommande *)realloc(c->lignes,
                                                     nouvelleCapacite * sizeof(LigneCommande));
            if (nouveauTableau == NULL) {
                printf("Erreur : memoire insuffisante pour ajouter la ligne.\n");
                return 0;
            }
            c->lignes = nouveauTableau;
            c->capaciteLignes = nouvelleCapacite;
        }

        /* Designation et prix sont figes au moment de la vente */
        ligne = &c->lignes[c->nbLignes];
        strcpy(ligne->refProduit, p->reference);
        strcpy(ligne->designation, p->designation);
        ligne->quantite = quantite;
        ligne->prixUnitaireHT = p->prixUnitaireHT;
        c->nbLignes++;
    }

    calculCommande(c);
    return 1;
}

/* Retire la ligne d'indice donne en decalant les suivantes */
static void retirerLigneDeCommande(Commande *c, int indiceLigne)
{
    int i;
    for (i = indiceLigne; i < c->nbLignes - 1; i++) {
        c->lignes[i] = c->lignes[i + 1];
    }
    c->nbLignes--;
    calculCommande(c);
}

/* Remet en stock les quantites de toutes les lignes de la commande */
static void reintegrerStock(Boutique *b, const Commande *c)
{
    int i, indiceProduit;
    for (i = 0; i < c->nbLignes; i++) {
        indiceProduit = rechercherIndiceProduit(b, c->lignes[i].refProduit);
        if (indiceProduit != -1) {
            b->produits[indiceProduit].quantiteStock += c->lignes[i].quantite;
        }
    }
}

/* ================== RECHERCHE ================== */

/*
 * Cherche la commande de numero donne chez tous les clients.
 * Retourne un pointeur vers la commande (ou NULL) et l'indice du client.
 */
Commande *rechercherCommande(Boutique *b, int numero, int *indiceClient)
{
    int i, j;
    for (i = 0; i < b->nbClients; i++) {
        for (j = 0; j < b->clients[i].nbCommandes; j++) {
            if (b->clients[i].commandes[j].numero == numero) {
                *indiceClient = i;
                return &b->clients[i].commandes[j];
            }
        }
    }
    return NULL;
}

/* Demande un numero et retourne la commande correspondante (ou NULL) */
static Commande *demanderCommande(Boutique *b, int *indiceClient)
{
    int numero;
    Commande *c;

    numero = saisirEntier("Numero de la commande : ", 1, 1000000);
    c = rechercherCommande(b, numero, indiceClient);
    if (c == NULL) {
        printf("Erreur : la commande %d n'existe pas.\n", numero);
    }
    return c;
}

/* Meme chose mais la commande doit etre EN_COURS */
static Commande *demanderCommandeEnCours(Boutique *b, int *indiceClient)
{
    Commande *c = demanderCommande(b, indiceClient);
    if (c != NULL && strcmp(c->statut, STATUT_EN_COURS) != 0) {
        printf("Erreur : operation impossible, la commande %d est au statut %s.\n",
               c->numero, c->statut);
        return NULL;
    }
    return c;
}

/* ================== AFFICHAGES ================== */

void afficherCommande(const Commande *c)
{
    int i;

    printf("\n");
    afficherSeparateur();
    if (c->numero > 0) {
        printf("COMMANDE N %d", c->numero);
    } else {
        printf("COMMANDE (numero attribue a la validation)");
    }
    printf("   |   Client %d   |   Date : ", c->idClient);
    afficherDate(c->dateCommande);
    printf("   |   Statut : %s\n", c->statut);
    printf("Livraison : ");
    afficherAdresse(&c->adresseLivraison);
    printf("\n");
    afficherSeparateur();

    printf("%-10s %-26s %6s %12s %12s\n", "Reference", "Designation", "Qte", "Prix U. HT", "Sous-total");
    for (i = 0; i < c->nbLignes; i++) {
        printf("%-10s %-26.26s %6d %12.2f %12.2f\n",
               c->lignes[i].refProduit, c->lignes[i].designation, c->lignes[i].quantite,
               c->lignes[i].prixUnitaireHT, c->lignes[i].sousTotalHT);
    }
    afficherSeparateur();
    printf("%58s %12.2f\n", "Montant brut HT :", c->montantBrutHT);
    if (c->montantRemise > 0) {
        printf("%50s %4.0f %% : %12.2f\n", "Remise", c->tauxRemise, -c->montantRemise);
    } else {
        printf("%58s %12s\n", "Remise :", "aucune");
    }
    printf("%58s %12.2f\n", "Montant HT apres remise :", c->montantHT);
    printf("%58s %12.2f\n", "TVA 20 % :", c->montantTVA);
    if (c->fraisPort > 0) {
        printf("%58s %12.2f\n", "Frais de port :", c->fraisPort);
    } else {
        printf("%58s %12s\n", "Frais de port :", "offerts");
    }
    printf("%58s %12.2f\n", "TOTAL TTC :", c->montantTTC);
    if (strcmp(c->statut, STATUT_ANNULEE) != 0) {
        printf("%58s %12d\n", "Points fidelite :", calculPointsFidelite(c));
    }
    afficherSeparateur();
}

void afficherEnteteResumeCommande(void)
{
    printf("  %-7s %-11s %-10s %6s %12s %12s\n", "Numero", "Date", "Statut", "Lignes", "Total HT", "Total TTC");
}

void afficherResumeCommande(const Commande *c)
{
    printf("  %-7d %02d/%02d/%04d  %-10s %6d %12.2f %12.2f\n",
           c->numero, c->dateCommande.jour, c->dateCommande.mois, c->dateCommande.annee,
           c->statut, c->nbLignes, c->montantHT, c->montantTTC);
}

/* ================== FONCTIONNALITES DU MENU ================== */

/* Creation d'une commande : client, N lignes, controle du stock, validation */
void creerCommande(Boutique *b)
{
    Commande cmd;
    Client *client;
    Produit *p;
    char reference[TAILLE_REF];
    char texte[200];
    int indiceClient, indiceProduit, indiceLigne;
    int idClient, dejaDansCommande, disponible, quantite, i, points;

    afficherTitre("CREATION D'UNE COMMANDE");
    idClient = saisirEntier("Identifiant du client : ", 1, 1000000);
    indiceClient = rechercherIndiceClient(b, idClient);
    if (indiceClient == -1 || b->clients[indiceClient].actif == 0) {
        printf("Erreur : aucun client actif avec l'identifiant %d.\n", idClient);
        return;
    }
    printf("Client : %s %s\n", b->clients[indiceClient].prenom, b->clients[indiceClient].nom);

    if (!initialiserCommande(&cmd, CAPACITE_INITIALE)) {
        return;
    }
    cmd.idClient = idClient;
    cmd.adresseLivraison = b->clients[indiceClient].adresse;   /* instantane de l'adresse */

    if (saisirOuiNon("Utiliser la date du jour")) {
        cmd.dateCommande = dateDuJour();
    } else {
        cmd.dateCommande = saisirDate("Date de la commande :");
    }

    afficherProduits(b);
    printf("\nSaisie des lignes de la commande.\n");

    /* Saisie de N lignes : on s'arrete quand l'utilisateur tape FIN */
    while (1) {
        saisirChaine("\nReference du produit (FIN pour terminer) : ", reference, TAILLE_REF);
        if (strcmp(reference, "FIN") == 0 || strcmp(reference, "fin") == 0) {
            break;
        }

        indiceProduit = rechercherIndiceProduit(b, reference);
        if (indiceProduit == -1 || b->produits[indiceProduit].actif == 0) {
            printf("  Erreur : produit %s inconnu ou supprime.\n", reference);
            continue;
        }
        p = &b->produits[indiceProduit];

        /* Controle du stock sur la quantite cumulee */
        dejaDansCommande = 0;
        indiceLigne = rechercherIndiceLigne(&cmd, reference);
        if (indiceLigne != -1) {
            dejaDansCommande = cmd.lignes[indiceLigne].quantite;
            printf("  Ce produit est deja dans la commande (%d), la quantite sera cumulee.\n",
                   dejaDansCommande);
        }
        disponible = p->quantiteStock - dejaDansCommande;
        if (disponible <= 0) {
            printf("  Erreur : stock insuffisant pour %s.\n", p->designation);
            continue;
        }

        printf("  %s - %.2f euros HT - stock disponible : %d\n",
               p->designation, p->prixUnitaireHT, disponible);
        quantite = saisirEntier("  Quantite : ", 1, disponible);

        if (!ajouterLigneDansCommande(&cmd, p, quantite)) {
            break;
        }
        printf("  Ligne ajoutee. Montant brut HT actuel : %.2f euros\n", cmd.montantBrutHT);
    }

    if (cmd.nbLignes == 0) {
        printf("Commande abandonnee : aucune ligne saisie.\n");
        free(cmd.lignes);
        return;
    }

    afficherCommande(&cmd);
    if (!saisirOuiNon("Valider cette commande")) {
        printf("Commande abandonnee.\n");
        free(cmd.lignes);
        return;
    }

    /* Numero attribue par le compteur de la boutique, jamais saisi */
    cmd.numero = b->numeroSuivant;
    client = &b->clients[indiceClient];
    if (!ajouterCommandeAuClient(client, cmd)) {
        free(cmd.lignes);
        return;
    }
    b->numeroSuivant++;

    /* Le stock est decremente et les points sont credites */
    for (i = 0; i < cmd.nbLignes; i++) {
        indiceProduit = rechercherIndiceProduit(b, cmd.lignes[i].refProduit);
        b->produits[indiceProduit].quantiteStock -= cmd.lignes[i].quantite;
    }
    points = calculPointsFidelite(&cmd);
    ajusterPointsClient(client, points);

    printf("Commande %d validee : %.2f euros TTC, %d point(s) fidelite credite(s).\n",
           cmd.numero, cmd.montantTTC, points);
    snprintf(texte, sizeof(texte), "Creation de la commande %d (client %d, %.2f euros TTC)",
             cmd.numero, idClient, cmd.montantTTC);
    ajouterHistorique(b, texte);
}

/* Ajout d'une ligne sur une commande EN_COURS */
void ajouterLigneCommande(Boutique *b)
{
    Commande *c;
    Produit *p;
    char reference[TAILLE_REF];
    char texte[200];
    int indiceClient, indiceProduit, quantite, anciensPoints;

    afficherTitre("AJOUT D'UNE LIGNE");
    c = demanderCommandeEnCours(b, &indiceClient);
    if (c == NULL) {
        return;
    }
    afficherCommande(c);

    saisirChaine("Reference du produit a ajouter : ", reference, TAILLE_REF);
    indiceProduit = rechercherIndiceProduit(b, reference);
    if (indiceProduit == -1 || b->produits[indiceProduit].actif == 0) {
        printf("Erreur : produit %s inconnu ou supprime.\n", reference);
        return;
    }
    p = &b->produits[indiceProduit];

    /* Le stock des lignes deja presentes a deja ete retire : on compare au stock actuel */
    if (p->quantiteStock <= 0) {
        printf("Erreur : %s est en rupture de stock.\n", p->designation);
        return;
    }
    if (rechercherIndiceLigne(c, reference) != -1) {
        printf("Ce produit est deja dans la commande : la quantite sera cumulee.\n");
    }
    printf("Stock disponible : %d\n", p->quantiteStock);
    quantite = saisirEntier("Quantite : ", 1, p->quantiteStock);

    anciensPoints = calculPointsFidelite(c);
    if (!ajouterLigneDansCommande(c, p, quantite)) {
        return;
    }
    p->quantiteStock -= quantite;

    /* Le montant a change : on met a jour les points du client */
    ajusterPointsClient(&b->clients[indiceClient], calculPointsFidelite(c) - anciensPoints);

    printf("Ligne ajoutee. Nouveau total : %.2f euros TTC.\n", c->montantTTC);
    snprintf(texte, sizeof(texte), "Ajout de %d x %s dans la commande %d", quantite, reference, c->numero);
    ajouterHistorique(b, texte);
}

/* Suppression d'une ligne sur une commande EN_COURS (stock reintegre) */
void supprimerLigneCommande(Boutique *b)
{
    Commande *c;
    char texte[200];
    char reference[TAILLE_REF];
    int indiceClient, numLigne, indiceProduit, quantite, anciensPoints, i;

    afficherTitre("SUPPRESSION D'UNE LIGNE");
    c = demanderCommandeEnCours(b, &indiceClient);
    if (c == NULL) {
        return;
    }
    if (c->nbLignes == 1) {
        printf("Impossible : une commande doit garder au moins une ligne.\n");
        printf("Utilisez plutot l'annulation de la commande.\n");
        return;
    }

    printf("Lignes de la commande %d :\n", c->numero);
    for (i = 0; i < c->nbLignes; i++) {
        printf("  %d. %-10s %-26.26s x %d\n", i + 1, c->lignes[i].refProduit,
               c->lignes[i].designation, c->lignes[i].quantite);
    }
    numLigne = saisirEntier("Numero de la ligne a supprimer : ", 1, c->nbLignes);

    if (!saisirOuiNon("Confirmer la suppression de la ligne")) {
        printf("Suppression abandonnee.\n");
        return;
    }

    /* On garde les infos avant de supprimer la ligne */
    strcpy(reference, c->lignes[numLigne - 1].refProduit);
    quantite = c->lignes[numLigne - 1].quantite;
    anciensPoints = calculPointsFidelite(c);

    retirerLigneDeCommande(c, numLigne - 1);

    indiceProduit = rechercherIndiceProduit(b, reference);
    if (indiceProduit != -1) {
        b->produits[indiceProduit].quantiteStock += quantite;
    }
    ajusterPointsClient(&b->clients[indiceClient], calculPointsFidelite(c) - anciensPoints);

    printf("Ligne supprimee, %d x %s remis en stock. Nouveau total : %.2f euros TTC.\n",
           quantite, reference, c->montantTTC);
    snprintf(texte, sizeof(texte), "Suppression de %s dans la commande %d", reference, c->numero);
    ajouterHistorique(b, texte);
}

/* Progression EN_COURS -> EXPEDIEE -> LIVREE, sans retour possible */
void changerStatutCommande(Boutique *b)
{
    Commande *c;
    char texte[200];
    char message[100];
    const char *nouveauStatut;
    int indiceClient;

    afficherTitre("CHANGEMENT DE STATUT");
    c = demanderCommande(b, &indiceClient);
    if (c == NULL) {
        return;
    }

    if (strcmp(c->statut, STATUT_EN_COURS) == 0) {
        nouveauStatut = STATUT_EXPEDIEE;
    } else if (strcmp(c->statut, STATUT_EXPEDIEE) == 0) {
        nouveauStatut = STATUT_LIVREE;
    } else {
        printf("Impossible : la commande %d est %s, son statut ne peut plus changer.\n",
               c->numero, c->statut);
        return;
    }

    snprintf(message, sizeof(message), "Passer la commande %d de %s a %s",
             c->numero, c->statut, nouveauStatut);
    if (saisirOuiNon(message)) {
        snprintf(texte, sizeof(texte), "Commande %d : %s -> %s", c->numero, c->statut, nouveauStatut);
        strcpy(c->statut, nouveauStatut);
        printf("Statut mis a jour : %s.\n", c->statut);
        ajouterHistorique(b, texte);
    } else {
        printf("Changement abandonne.\n");
    }
}

/* Annulation (seulement EN_COURS) : stock reintegre et points retires */
void annulerCommande(Boutique *b)
{
    Commande *c;
    char texte[200];
    int indiceClient, points;

    afficherTitre("ANNULATION D'UNE COMMANDE");
    c = demanderCommandeEnCours(b, &indiceClient);
    if (c == NULL) {
        return;
    }
    afficherCommande(c);

    if (!saisirOuiNon("Confirmer l'annulation de cette commande")) {
        printf("Annulation abandonnee.\n");
        return;
    }

    reintegrerStock(b, c);
    points = calculPointsFidelite(c);
    ajusterPointsClient(&b->clients[indiceClient], -points);
    strcpy(c->statut, STATUT_ANNULEE);

    printf("Commande %d annulee : stock reintegre, %d point(s) retire(s) au client.\n",
           c->numero, points);
    snprintf(texte, sizeof(texte), "Annulation de la commande %d", c->numero);
    ajouterHistorique(b, texte);
}

void afficherCommandeParNumero(Boutique *b)
{
    Commande *c;
    int indiceClient;

    afficherTitre("DETAIL D'UNE COMMANDE");
    c = demanderCommande(b, &indiceClient);
    if (c != NULL) {
        printf("Client : %s %s\n", b->clients[indiceClient].prenom, b->clients[indiceClient].nom);
        afficherCommande(c);
    }
}

/* Liste de toutes les commandes de la boutique, client par client */
void afficherToutesLesCommandes(const Boutique *b)
{
    int i, j;
    int total = 0;

    afficherTitre("TOUTES LES COMMANDES");
    for (i = 0; i < b->nbClients; i++) {
        if (b->clients[i].nbCommandes == 0) {
            continue;
        }
        printf("\nClient %d - %s %s :\n", b->clients[i].idClient,
               b->clients[i].prenom, b->clients[i].nom);
        afficherEnteteResumeCommande();
        for (j = 0; j < b->clients[i].nbCommandes; j++) {
            afficherResumeCommande(&b->clients[i].commandes[j]);
            total++;
        }
    }
    printf("\n%d commande(s) au total.\n", total);
}
