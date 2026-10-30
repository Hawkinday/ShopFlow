/*
 * types.h - Structures de donnees et constantes partagees
 * Projet ShopFlow - Module XTI101 - Efrei Paris
 *
 * Ce fichier est inclus par tous les modules. Il regroupe toutes les
 * structures pour eviter les inclusions circulaires entre client.h
 * et commande.h.
 */

#ifndef TYPES_H
#define TYPES_H

/* ---------- Tailles des chaines de caracteres ---------- */
#define TAILLE_REF          12
#define TAILLE_DESIGNATION  60
#define TAILLE_CATEGORIE    30
#define TAILLE_NOM          50
#define TAILLE_EMAIL        80
#define TAILLE_TEL          15
#define TAILLE_RUE          80
#define TAILLE_VILLE        50
#define TAILLE_CP           6
#define TAILLE_PAYS         50
#define TAILLE_STATUT       20
#define TAILLE_TEXTE_HISTO  120

/* ---------- Statuts possibles d'une commande ---------- */
#define STATUT_EN_COURS  "EN_COURS"
#define STATUT_EXPEDIEE  "EXPEDIEE"
#define STATUT_LIVREE    "LIVREE"
#define STATUT_ANNULEE   "ANNULEE"

/* ---------- Regles de facturation ---------- */
#define TAUX_TVA            20.0f   /* en pourcentage                     */
#define FRAIS_PORT          5.90f   /* en euros                           */
#define SEUIL_PORT_GRATUIT  60.0f   /* HT brut a partir duquel port offert */
#define EUROS_PAR_POINT     10.0f   /* 1 point par tranche de 10 euros TTC */

/* Capacite de depart des tableaux dynamiques (doublee si plein) */
#define CAPACITE_INITIALE   4

/* ---------- 3.1.1 Date ---------- */
typedef struct {
    int jour;
    int mois;
    int annee;
} Date;

/* ---------- 3.1.2 Adresse ---------- */
typedef struct {
    char rue[TAILLE_RUE];
    char ville[TAILLE_VILLE];
    char codePostal[TAILLE_CP];
    char pays[TAILLE_PAYS];
} Adresse;

/* ---------- 3.1.3 Produit ---------- */
typedef struct {
    char reference[TAILLE_REF];
    char designation[TAILLE_DESIGNATION];
    char categorie[TAILLE_CATEGORIE];
    float prixUnitaireHT;
    int quantiteStock;
    int seuilAlerte;
    int actif;                 /* 1 = actif, 0 = supprime (logique) */
} Produit;

/* ---------- 3.1.4 LigneCommande ---------- */
typedef struct {
    char refProduit[TAILLE_REF];
    char designation[TAILLE_DESIGNATION];   /* figee au moment de la vente */
    int quantite;
    float prixUnitaireHT;                   /* fige au moment de la vente  */
    float sousTotalHT;                      /* quantite x prixUnitaireHT   */
} LigneCommande;

/* ---------- 3.1.5 Commande ---------- */
typedef struct {
    int numero;
    int idClient;
    Date dateCommande;
    char statut[TAILLE_STATUT];
    int nbLignes;
    int capaciteLignes;
    LigneCommande *lignes;      /* tableau dynamique */
    float montantBrutHT;
    float tauxRemise;
    float montantRemise;
    float montantHT;
    float montantTVA;
    float montantTTC;
    float fraisPort;
    Adresse adresseLivraison;   /* copie de l'adresse du client */
} Commande;

/* ---------- 3.1.6 Client ---------- */
typedef struct {
    int idClient;
    char nom[TAILLE_NOM];
    char prenom[TAILLE_NOM];
    char email[TAILLE_EMAIL];
    char telephone[TAILLE_TEL];
    Adresse adresse;
    int pointsFidelite;
    int nbCommandes;
    int capaciteCommandes;
    Commande *commandes;        /* tableau dynamique */
    int actif;                  /* 1 = actif, 0 = supprime (logique) */
} Client;

/* ---------- Operation (fonctionnalite creative : historique) ---------- */
typedef struct {
    int numero;
    char heure[9];                     /* format HH:MM:SS */
    char texte[TAILLE_TEXTE_HISTO];
} Operation;

/* ---------- 3.1.7 Boutique (structure racine) ---------- */
typedef struct {
    char nom[TAILLE_NOM];
    Produit *produits;          /* catalogue (tableau dynamique) */
    int nbProduits;
    int capaciteProduits;
    Client *clients;            /* repertoire (tableau dynamique) */
    int nbClients;
    int capaciteClients;
    int numeroSuivant;          /* prochain numero de commande */
    int idClientSuivant;        /* prochain identifiant client */
    Operation *historique;      /* journal des operations (ajout creatif) */
    int nbOperations;
    int capaciteOperations;
} Boutique;

#endif
