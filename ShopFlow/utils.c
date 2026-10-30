/*
 * utils.c - Saisie securisee, validation des donnees, affichage formate
 * Projet ShopFlow - Module XTI101
 *
 * Toutes les saisies passent par fgets (jamais de scanf direct) pour
 * eviter les problemes de buffer clavier et de saisies invalides.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "utils.h"

#define TAILLE_BUFFER 256

/* Verifie si une chaine ne contient que des espaces (ou est vide) */
static int estVide(const char *texte)
{
    int i;
    for (i = 0; texte[i] != '\0'; i++) {
        if (!isspace((unsigned char)texte[i])) {
            return 0;
        }
    }
    return 1;
}

/* Verifie qu'une chaine fait exactement "longueur" caracteres, tous des chiffres */
static int estUnNombreDeLongueur(const char *texte, int longueur)
{
    int i;
    if ((int)strlen(texte) != longueur) {
        return 0;
    }
    for (i = 0; i < longueur; i++) {
        if (!isdigit((unsigned char)texte[i])) {
            return 0;
        }
    }
    return 1;
}

/*
 * Lit une ligne au clavier dans buffer (sans le '\n').
 * Retourne 1 si la ligne tenait dans le buffer, 0 si elle etait trop longue
 * (dans ce cas le reste de la ligne est vide du buffer clavier).
 */
int lireLigne(char *buffer, int taille)
{
    int c;
    int tropLong = 0;
    size_t longueur;

    if (fgets(buffer, taille, stdin) == NULL) {
        /* Fin de fichier (Ctrl+D / Ctrl+Z) : on ne peut plus rien saisir */
        printf("\nFin de l'entree standard : arret du programme.\n");
        exit(EXIT_FAILURE);
    }

    longueur = strlen(buffer);
    if (longueur > 0 && buffer[longueur - 1] == '\n') {
        buffer[longueur - 1] = '\0';
        longueur--;
    } else {
        /* La ligne etait trop longue : on vide le reste */
        while ((c = getchar()) != '\n' && c != EOF) {
            tropLong = 1;
        }
    }

    /* On enleve le '\r' eventuel (fichiers Windows) */
    if (longueur > 0 && buffer[longueur - 1] == '\r') {
        buffer[longueur - 1] = '\0';
    }

    return !tropLong;
}

/* Saisie d'un entier compris entre min et max (inclus) */
int saisirEntier(const char *message, int min, int max)
{
    char buffer[TAILLE_BUFFER];
    int valeur;
    char reste;

    while (1) {
        printf("%s", message);
        if (lireLigne(buffer, TAILLE_BUFFER) &&
            sscanf(buffer, "%d %c", &valeur, &reste) == 1) {
            if (valeur >= min && valeur <= max) {
                return valeur;
            }
            printf("  Erreur : la valeur doit etre comprise entre %d et %d.\n", min, max);
        } else {
            printf("  Erreur : veuillez saisir un nombre entier.\n");
        }
    }
}

/* Saisie d'un reel compris entre min et max (la virgule est acceptee) */
float saisirFloat(const char *message, float min, float max)
{
    char buffer[TAILLE_BUFFER];
    float valeur;
    char reste;
    int i;

    while (1) {
        printf("%s", message);
        if (lireLigne(buffer, TAILLE_BUFFER)) {
            /* on accepte 12,50 comme 12.50 */
            for (i = 0; buffer[i] != '\0'; i++) {
                if (buffer[i] == ',') {
                    buffer[i] = '.';
                }
            }
            if (sscanf(buffer, "%f %c", &valeur, &reste) == 1) {
                if (valeur >= min && valeur <= max) {
                    return valeur;
                }
                printf("  Erreur : la valeur doit etre comprise entre %.2f et %.2f.\n", min, max);
                continue;
            }
        }
        printf("  Erreur : veuillez saisir un nombre (ex : 19.90).\n");
    }
}

/* Saisie d'une chaine non vide de (taille - 1) caracteres maximum */
void saisirChaine(const char *message, char *destination, int taille)
{
    char buffer[TAILLE_BUFFER];

    while (1) {
        printf("%s", message);
        if (!lireLigne(buffer, TAILLE_BUFFER) || (int)strlen(buffer) >= taille) {
            printf("  Erreur : %d caracteres maximum.\n", taille - 1);
        } else if (estVide(buffer)) {
            printf("  Erreur : ce champ ne peut pas etre vide.\n");
        } else {
            strcpy(destination, buffer);
            return;
        }
    }
}

/* Pose une question fermee. Retourne 1 pour oui, 0 pour non */
int saisirOuiNon(const char *message)
{
    char buffer[TAILLE_BUFFER];

    while (1) {
        printf("%s (o/n) : ", message);
        lireLigne(buffer, TAILLE_BUFFER);
        if (strcmp(buffer, "o") == 0 || strcmp(buffer, "O") == 0) {
            return 1;
        }
        if (strcmp(buffer, "n") == 0 || strcmp(buffer, "N") == 0) {
            return 0;
        }
        printf("  Erreur : repondez par o ou n.\n");
    }
}

/* Saisie d'une date valide (jour verifie selon le mois et les annees bissextiles) */
Date saisirDate(const char *message)
{
    Date d;

    printf("%s\n", message);
    while (1) {
        d.jour = saisirEntier("  Jour (1-31)        : ", 1, 31);
        d.mois = saisirEntier("  Mois (1-12)        : ", 1, 12);
        d.annee = saisirEntier("  Annee (2000-2100)  : ", 2000, 2100);
        if (dateValide(d)) {
            return d;
        }
        printf("  Erreur : cette date n'existe pas, recommencez.\n");
    }
}

/* Une annee est bissextile si divisible par 4 mais pas par 100, ou par 400 */
int estBissextile(int annee)
{
    return (annee % 4 == 0 && annee % 100 != 0) || (annee % 400 == 0);
}

int dateValide(Date d)
{
    int joursParMois[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (d.annee < 2000 || d.annee > 2100) {
        return 0;
    }
    if (d.mois < 1 || d.mois > 12) {
        return 0;
    }
    if (estBissextile(d.annee)) {
        joursParMois[1] = 29;
    }
    return d.jour >= 1 && d.jour <= joursParMois[d.mois - 1];
}

/* Email valide : un seul @, du texte avant, un point dans le domaine, pas d'espace */
int emailValide(const char *email)
{
    const char *arobase = strchr(email, '@');
    int i;

    if (arobase == NULL || arobase == email) {
        return 0;
    }
    if (strchr(arobase + 1, '@') != NULL) {
        return 0;
    }
    if (arobase[1] == '.' || strchr(arobase + 1, '.') == NULL) {
        return 0;
    }
    if (email[strlen(email) - 1] == '.') {
        return 0;
    }
    for (i = 0; email[i] != '\0'; i++) {
        if (isspace((unsigned char)email[i])) {
            return 0;
        }
    }
    return 1;
}

int telephoneValide(const char *telephone)
{
    return estUnNombreDeLongueur(telephone, 10);
}

int codePostalValide(const char *codePostal)
{
    return estUnNombreDeLongueur(codePostal, 5);
}

/* Retourne la date systeme */
Date dateDuJour(void)
{
    Date d = {1, 1, 2026};
    time_t maintenant = time(NULL);
    struct tm *t = localtime(&maintenant);

    if (t != NULL) {
        d.jour = t->tm_mday;
        d.mois = t->tm_mon + 1;
        d.annee = t->tm_year + 1900;
    }
    return d;
}

/* Copie source dans destination en minuscules (pour les recherches) */
void mettreEnMinuscules(const char *source, char *destination, int taille)
{
    int i;
    for (i = 0; source[i] != '\0' && i < taille - 1; i++) {
        destination[i] = (char)tolower((unsigned char)source[i]);
    }
    destination[i] = '\0';
}

/*
 * Arrondi au centime le plus proche (valeurs positives).
 * Le petit 0.001 corrige l'imprecision des float : par exemple
 * 5 % de 229.70 = 11.485 est stocke 11.48499... et doit donner 11.49.
 */
float arrondir(float valeur)
{
    return (float)((long)((double)valeur * 100.0 + 0.5 + 0.001)) / 100.0f;
}

void afficherDate(Date d)
{
    printf("%02d/%02d/%04d", d.jour, d.mois, d.annee);
}

void afficherTitre(const char *titre)
{
    printf("\n========== %s ==========\n", titre);
}

void afficherSeparateur(void)
{
    printf("------------------------------------------------------------------------------------------\n");
}

void pause(void)
{
    char buffer[TAILLE_BUFFER];
    printf("\nAppuyez sur Entree pour continuer...");
    lireLigne(buffer, TAILLE_BUFFER);
}
