# ShopFlow — Gestion des commandes d'une boutique en ligne

Mini-projet de programmation C — module XTI101 — Efrei Paris

## Compilation et lancement

```bash
make            # compile tous les modules (gcc -Wall -Wextra, 0 avertissement)
./shopflow      # lance le programme
make valgrind   # lance le programme sous valgrind --leak-check=full
make clean      # supprime les fichiers .o et l'executable
```

Sans `make` : `gcc -Wall -Wextra -std=c99 *.c -o shopflow`

## Organisation des fichiers

| Module | Fichiers | Role |
|---|---|---|
| Types | `types.h` | Les 7 structures + constantes, inclus par tous les modules |
| Catalogue | `produit.h / produit.c` | Ajout, modification, suppression logique, recherche, alertes de stock |
| Clients | `client.h / client.c` | Ajout, modification, suppression logique, fiche client, points fidelite |
| Commandes | `commande.h / commande.c` | Creation, lignes, calcul des montants, statuts, annulation |
| Statistiques | `statistique.h / statistique.c` | Chiffre d'affaires, palmares produits et clients (tri a bulles) |
| Saisie | `utils.h / utils.c` | Saisies securisees (fgets), validations (date, email, telephone, CP) |
| Initialisation | `initialisation.h / initialisation.c` | Donnees de demonstration et liberation de la memoire |
| Historique | `historique.h / historique.c` | Journal des operations (fonctionnalite creative) |
| Programme | `main.c` | Menus et boucle principale |

## Allocation dynamique

Les cinq collections sont des tableaux dynamiques (`pointeur + nb + capacite`) :
produits, clients, commandes de chaque client, lignes de chaque commande et historique.

- Quand un tableau est plein, sa capacite est doublee avec `realloc`, en passant
  par un pointeur temporaire pour ne pas perdre l'ancien tableau si `realloc` echoue.
- Chaque retour de `malloc` / `realloc` est verifie.
- `detruireBoutique()` libere tout, du niveau le plus profond (lignes) au plus haut.
- Resultat valgrind : `All heap blocks were freed -- no leaks are possible`.

## Regles metier appliquees

- Remise sur le HT brut : 0 % (< 100), 5 % (100–299,99), 10 % (300–599,99), 15 % (≥ 600).
- TVA 20 % sur le HT apres remise ; port 5,90 € offert des 60 € HT brut, sans TVA.
- Points : 1 point par tranche de 10 € TTC, credites a la creation, retires a l'annulation
  (et recalcules si on ajoute ou supprime une ligne d'une commande EN_COURS).
- Statuts : EN_COURS → EXPEDIEE → LIVREE ; annulation seulement depuis EN_COURS.
- Stock : retire a la validation et a l'ajout d'une ligne ; remis a l'annulation et
  a la suppression d'une ligne.
- Une seule ligne par produit dans une commande : un produit ressaisi est cumule
  et le stock est controle sur la quantite totale.
- Numeros de commande et identifiants clients donnes par les compteurs de la boutique.
- Unicite de la reference produit et de l'email verifiee aussi sur les elements supprimes.
- Une commande garde toujours au moins une ligne (pour tout retirer : annulation).

**Remarque sur le sujet** : dans le tableau de la structure Commande, l'exemple
(brut 285,96 €) compte 5,90 € de frais de port alors que la regle dit que le port
est offert des 60 € HT brut. Nous avons applique la regle.

## Fonctionnalites creatives

1. **Historique des operations** (menu principal, choix 5) : chaque action
   (ajout, modification, suppression, commande, changement de statut, annulation)
   est enregistree avec l'heure dans un tableau dynamique supplementaire.
2. **Statistiques par categorie** (menu Statistiques, choix 6) : nombre de produits,
   quantite vendue, CA HT et part du CA par categorie. Le tableau des categories
   grandit avec `realloc` a chaque nouvelle categorie trouvee.

## Donnees de demonstration

- 8 produits dans 5 categories ; PRD-0002 et PRD-0007 sont en alerte de stock.
- 5 clients (identifiants 1 a 5).
- 6 commandes (1001 a 1006) :

| N | Client | Statut | Lignes | Brut HT | Remise | TTC |
|---|---|---|---|---|---|---|
| 1001 | Julie Bernard | LIVREE | 3 | 214,00 | 5 % | 243,96 |
| 1002 | Lucas Martin | LIVREE | 4 | 715,80 | 15 % | 730,12 |
| 1003 | Emma Petit | EXPEDIEE | 1 | 59,90 | 0 % + port | 77,78 |
| 1004 | Julie Bernard | EXPEDIEE | 2 | 229,70 | 5 % | 261,85 |
| 1005 | Hugo Durand | ANNULEE | 2 | 188,90 | 5 % | 215,34 |
| 1006 | Chloe Leroy | EN_COURS | 4 | 178,60 | 5 % | 203,60 |

## Demo conseillee pour la soutenance

1. Catalogue → Afficher (valorisation du stock), puis Alertes de stock.
2. Commandes → Afficher la commande 1002 (palier 15 %) et 1003 (frais de port).
3. Commandes → Creer une commande pour le client 3 avec 5 produits differents
   (le tableau de lignes passe de 4 a 8 places : `realloc`), en ressaisissant
   un produit pour montrer le cumul et le controle du stock.
4. Commandes → Annuler cette commande, puis reafficher le catalogue (stock reintegre).
5. Catalogue → Supprimer PRD-0001 (refuse : produit utilise).
6. Clients → Ajouter un client avec un email deja pris (refuse), puis valide.
7. Statistiques → CA global, CA par periode (2026), palmares, categories.
8. Historique, puis Quitter et montrer le resultat de `make valgrind`.
