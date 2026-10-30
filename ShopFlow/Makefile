2CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
EXEC = shopflow
OBJ = main.o utils.o historique.o produit.o client.o commande.o statistique.o initialisation.o

# Edition des liens
all: $(EXEC)
$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJ)

# Compilation de chaque module
%.o: %.c types.h
	$(CC) $(CFLAGS) -c $< -o $@

main.o: main.c utils.h produit.h client.h commande.h statistique.h initialisation.h historique.h
utils.o: utils.c utils.h
historique.o: historique.c historique.h utils.h
produit.o: produit.c produit.h utils.h historique.h
client.o: client.c client.h commande.h utils.h historique.h
commande.o: commande.c commande.h client.h produit.h utils.h historique.h
statistique.o: statistique.c statistique.h client.h produit.h utils.h
initialisation.o: initialisation.c initialisation.h produit.h client.h commande.h historique.h

# Lancer le programme
run: $(EXEC)
	./$(EXEC)

# Verifier les fuites memoire
valgrind: $(EXEC)
	valgrind --leak-check=full ./$(EXEC)

clean:
	rm -f $(OBJ) $(EXEC)

.PHONY: all run valgrind clean
