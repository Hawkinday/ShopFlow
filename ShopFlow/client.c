/*
 * client.c - Gestion des clients
 * Projet ShopFlow - Module XTI101
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"
#include "commande.h"
#include "utils.h"
#include "historique.h"

/* Retourne l'indice du client d'identifiant idClient, ou -1 */
int rechercherIndiceClient(const Boutique *b, int idClient)
{
    int i;
    for (i = 0; i < b->nbClients; i++) {
        if (b->clients[i].idClient == idClient) {
            return i;
        }
    }
    return -1;
}

/*
 * Retourne 1 si l'email est deja utilise par un autre client que idIgnore.
 * Le controle porte sur tous les clients, meme supprimes.
 * La comparaison ne tient pas compte des majuscules.
 */
static int emailExiste(const Boutique *b, const char *email, int idIgnore)
{
    int i;
    char emailMin[TAILLE_EMAIL];
    char autreMin[TAILLE_EMAIL];

    mettreEnMinuscules(email, emailMin, TAILLE_EMAIL);
    for (i = 0; i < b->nbClients; i++) {
        if (b->clients[i].idClient == idIgnore) {
            continue;
        }
        mettreEnMinuscules(b->clients[i].email, autreMin, TAILLE_EMAIL);
        if (strcmp(emailMin, autreMin) == 0) {
            return 1;
        }
    }
    return 0;
}

/* Double la capacite du repertoire clients */
static int agrandirRepertoire(Boutique *b)
{
    int nouvelleCapacite;
    Client *nouveauTableau;

    if (b->capaciteClients == 0) {
        nouvelleCapacite = CAPACITE_INITIALE;
    } else {
        nouvelleCapacite = b->capaciteClients * 2;
    }
    nouveauTableau = (Client *)realloc(b->clients, nouvelleCapacite * sizeof(Client));
    if (nouveauTableau == NULL) {
        printf("Erreur : memoire insuffisante pour agrandir le repertoire.\n");
        return 0;
    }
    b->clients = nouveauTableau;
    b->capaciteClients = nouvelleCapacite;
    return 1;
}

/*
 * Ajoute un client au repertoire. L'identifiant est attribue
 * automatiquement par le compteur de la boutique (jamais saisi).
 * Retourne l'identifiant attribue, ou 0 en cas d'erreur.
 */
int ajouterClientAuRepertoire(Boutique *b, Client c)
{
    if (b->nbClients == b->capaciteClients) {
        if (!agrandirRepertoire(b)) {
            return 0;
        }
    }
    c.idClient = b->idClientSuivant;
    b->idClientSuivant++;
    b->clients[b->nbClients] = c;
    b->nbClients++;
    return c.idClient;
}

/*
 * Ajoute une commande a l'historique du client (copie de la structure).
 * Le tableau "lignes" de la commande appartient ensuite au client.
 */
int ajouterCommandeAuClient(Client *c, Commande cmd)
{
    int nouvelleCapacite;
    Commande *nouveauTableau;

    if (c->nbCommandes == c->capaciteCommandes) {
        if (c->capaciteCommandes == 0) {
            nouvelleCapacite = 2;
        } else {
            nouvelleCapacite = c->capaciteCommandes * 2;
        }
        nouveauTableau = (Commande *)realloc(c->commandes, nouvelleCapacite * sizeof(Commande));
        if (nouveauTableau == NULL) {
            printf("Erreur : memoire insuffisante pour enregistrer la commande.\n");
            return 0;
        }
        c->commandes = nouveauTableau;
        c->capaciteCommandes = nouvelleCapacite;
    }
    c->commandes[c->nbCommandes] = cmd;
    c->nbCommandes++;
    return 1;
}

int nbCommandesNonAnnulees(const Client *c)
{
    int i;
    int nb = 0;
    for (i = 0; i < c->nbCommandes; i++) {
        if (strcmp(c->commandes[i].statut, STATUT_ANNULEE) != 0) {
            nb++;
        }
    }
    return nb;
}

/* Chiffre d'affaires HT (apres remise) genere par le client, hors annulations */
float chiffreAffairesClientHT(const Client *c)
{
    int i;
    float total = 0;
    for (i = 0; i < c->nbCommandes; i++) {
        if (strcmp(c->commandes[i].statut, STATUT_ANNULEE) != 0) {
            total += c->commandes[i].montantHT;
        }
    }
    return total;
}

float chiffreAffairesClientTTC(const Client *c)
{
    int i;
    float total = 0;
    for (i = 0; i < c->nbCommandes; i++) {
        if (strcmp(c->commandes[i].statut, STATUT_ANNULEE) != 0) {
            total += c->commandes[i].montantTTC;
        }
    }
    return total;
}

void afficherAdresse(const Adresse *a)
{
    printf("%s, %s %s, %s", a->rue, a->codePostal, a->ville, a->pays);
}

/* ---------- Saisies specifiques aux clients ---------- */

static void saisirAdresse(Adresse *a)
{
    saisirChaine("  Rue (numero et voie) : ", a->rue, TAILLE_RUE);
    saisirChaine("  Ville : ", a->ville, TAILLE_VILLE);
    do {
        saisirChaine("  Code postal (5 chiffres) : ", a->codePostal, TAILLE_CP);
        if (!codePostalValide(a->codePostal)) {
            printf("  Erreur : le code postal doit contenir 5 chiffres.\n");
        }
    } while (!codePostalValide(a->codePostal));
    saisirChaine("  Pays : ", a->pays, TAILLE_PAYS);
}

/* Saisie d'un email valide et unique (idIgnore = client en cours de modification) */
static void saisirEmail(const Boutique *b, char *email, int idIgnore)
{
    while (1) {
        saisirChaine("Email : ", email, TAILLE_EMAIL);
        if (!emailValide(email)) {
            printf("  Erreur : email invalide (exemple : prenom.nom@mail.fr).\n");
        } else if (emailExiste(b, email, idIgnore)) {
            printf("  Erreur : cet email est deja utilise par un autre client.\n");
        } else {
            return;
        }
    }
}

static void saisirTelephone(char *telephone)
{
    while (1) {
        saisirChaine("Telephone (10 chiffres) : ", telephone, TAILLE_TEL);
        if (telephoneValide(telephone)) {
            return;
        }
        printf("  Erreur : le telephone doit contenir exactement 10 chiffres.\n");
    }
}

/* Demande un identifiant et retourne l'indice d'un client ACTIF, ou -1 */
static int demanderClientActif(const Boutique *b)
{
    int id, indice;

    id = saisirEntier("Identifiant du client : ", 1, 1000000);
    indice = rechercherIndiceClient(b, id);
    if (indice == -1 || b->clients[indice].actif == 0) {
        printf("Erreur : aucun client actif avec l'identifiant %d.\n", id);
        return -1;
    }
    return indice;
}

/* ---------- Fonctionnalites du menu ---------- */

void ajouterClient(Boutique *b)
{
    Client c;
    int id;
    char texte[200];

    afficherTitre("AJOUT D'UN CLIENT");
    saisirChaine("Nom : ", c.nom, TAILLE_NOM);
    saisirChaine("Prenom : ", c.prenom, TAILLE_NOM);
    saisirEmail(b, c.email, 0);
    saisirTelephone(c.telephone);
    printf("Adresse de livraison :\n");
    saisirAdresse(&c.adresse);

    /* Un nouveau client n'a ni commande ni point */
    c.pointsFidelite = 0;
    c.nbCommandes = 0;
    c.capaciteCommandes = 0;
    c.commandes = NULL;
    c.actif = 1;

    id = ajouterClientAuRepertoire(b, c);
    if (id != 0) {
        printf("Client %s %s ajoute avec l'identifiant %d.\n", c.prenom, c.nom, id);
        snprintf(texte, sizeof(texte), "Ajout du client %d (%s %s)", id, c.prenom, c.nom);
        ajouterHistorique(b, texte);
    }
}

void modifierClient(Boutique *b)
{
    int indice, choix;
    Client *c;
    char texte[200];

    afficherTitre("MODIFICATION D'UN CLIENT");
    indice = demanderClientActif(b);
    if (indice == -1) {
        return;
    }
    c = &b->clients[indice];

    do {
        printf("\nClient %d : %s %s\n", c->idClient, c->prenom, c->nom);
        printf("  Email     : %s\n", c->email);
        printf("  Telephone : %s\n", c->telephone);
        printf("  Adresse   : ");
        afficherAdresse(&c->adresse);
        printf("\n\n1. Modifier le nom et le prenom\n");
        printf("2. Modifier l'email\n");
        printf("3. Modifier le telephone\n");
        printf("4. Modifier l'adresse de livraison\n");
        printf("0. Terminer\n");
        choix = saisirEntier("Votre choix : ", 0, 4);

        switch (choix) {
            case 1:
                saisirChaine("Nouveau nom : ", c->nom, TAILLE_NOM);
                saisirChaine("Nouveau prenom : ", c->prenom, TAILLE_NOM);
                break;
            case 2:
                saisirEmail(b, c->email, c->idClient);
                break;
            case 3:
                saisirTelephone(c->telephone);
                break;
            case 4:
                printf("Nouvelle adresse (les commandes passees gardent l'ancienne) :\n");
                saisirAdresse(&c->adresse);
                break;
            default:
                break;
        }
    } while (choix != 0);

    snprintf(texte, sizeof(texte), "Modification du client %d", c->idClient);
    ajouterHistorique(b, texte);
}

/* Suppression logique, refusee si le client a des commandes non annulees */
void supprimerClient(Boutique *b)
{
    int indice;
    Client *c;
    char texte[200];

    afficherTitre("SUPPRESSION D'UN CLIENT");
    indice = demanderClientActif(b);
    if (indice == -1) {
        return;
    }
    c = &b->clients[indice];

    if (nbCommandesNonAnnulees(c) > 0) {
        printf("Suppression refusee : le client a %d commande(s) non annulee(s).\n",
               nbCommandesNonAnnulees(c));
        return;
    }

    printf("Client : %d - %s %s\n", c->idClient, c->prenom, c->nom);
    if (saisirOuiNon("Confirmer la suppression")) {
        c->actif = 0;
        printf("Client %d supprime.\n", c->idClient);
        snprintf(texte, sizeof(texte), "Suppression du client %d", c->idClient);
        ajouterHistorique(b, texte);
    } else {
        printf("Suppression abandonnee.\n");
    }
}

/* Vue d'ensemble : nombre de commandes et chiffre d'affaires genere */
void afficherClients(const Boutique *b)
{
    int i;
    const Client *c;
    char nomComplet[2 * TAILLE_NOM];

    afficherTitre("LISTE DES CLIENTS");
    printf("%-4s %-20s %-24s %-12s %4s %10s %10s %6s\n",
           "ID", "Nom", "Email", "Ville", "Cmd", "CA HT", "CA TTC", "Points");
    afficherSeparateur();
    for (i = 0; i < b->nbClients; i++) {
        c = &b->clients[i];
        if (!c->actif) {
            continue;
        }
        snprintf(nomComplet, sizeof(nomComplet), "%s %s", c->prenom, c->nom);
        printf("%-4d %-20.20s %-24.24s %-12.12s %4d %10.2f %10.2f %6d\n",
               c->idClient, nomComplet, c->email, c->adresse.ville, c->nbCommandes,
               chiffreAffairesClientHT(c), chiffreAffairesClientTTC(c), c->pointsFidelite);
    }
}

/* Fiche detaillee avec l'historique complet des commandes */
void afficherFicheClient(const Boutique *b)
{
    int indice, i, numero;
    const Client *c;
    int trouve = 0;

    afficherTitre("FICHE CLIENT");
    indice = demanderClientActif(b);
    if (indice == -1) {
        return;
    }
    c = &b->clients[indice];

    printf("\nClient n %d : %s %s\n", c->idClient, c->prenom, c->nom);
    printf("  Email           : %s\n", c->email);
    printf("  Telephone       : %s\n", c->telephone);
    printf("  Adresse         : ");
    afficherAdresse(&c->adresse);
    printf("\n  Points fidelite : %d\n", c->pointsFidelite);
    printf("  CA genere       : %.2f euros HT / %.2f euros TTC\n",
           chiffreAffairesClientHT(c), chiffreAffairesClientTTC(c));

    printf("\nHistorique des commandes (%d) :\n", c->nbCommandes);
    if (c->nbCommandes == 0) {
        printf("  Aucune commande.\n");
        return;
    }
    afficherEnteteResumeCommande();
    for (i = 0; i < c->nbCommandes; i++) {
        afficherResumeCommande(&c->commandes[i]);
    }

    if (saisirOuiNon("\nVoir le detail d'une commande")) {
        numero = saisirEntier("Numero de la commande : ", 1, 1000000);
        for (i = 0; i < c->nbCommandes; i++) {
            if (c->commandes[i].numero == numero) {
                afficherCommande(&c->commandes[i]);
                trouve = 1;
            }
        }
        if (!trouve) {
            printf("Cette commande n'appartient pas a ce client.\n");
        }
    }
}
