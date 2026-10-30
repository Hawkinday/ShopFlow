/*
 * utils.h - Saisie securisee, validation des donnees, affichage formate
 * Projet ShopFlow - Module XTI101
 */

#ifndef UTILS_H
#define UTILS_H

#include "types.h"

/* ----- Saisies securisees (redemandent tant que la saisie est invalide) ----- */
int   lireLigne(char *buffer, int taille);
int   saisirEntier(const char *message, int min, int max);
float saisirFloat(const char *message, float min, float max);
void  saisirChaine(const char *message, char *destination, int taille);
int   saisirOuiNon(const char *message);
Date  saisirDate(const char *message);

/* ----- Validations ----- */
int estBissextile(int annee);
int dateValide(Date d);
int emailValide(const char *email);
int telephoneValide(const char *telephone);
int codePostalValide(const char *codePostal);

/* ----- Outils divers ----- */
Date  dateDuJour(void);
void  mettreEnMinuscules(const char *source, char *destination, int taille);
float arrondir(float valeur);

/* ----- Affichage ----- */
void afficherDate(Date d);
void afficherTitre(const char *titre);
void afficherSeparateur(void);
void pause(void);

#endif
