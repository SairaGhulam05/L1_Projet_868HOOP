# Cahier de suivi du projet de Saïra Ghulam
### Séance 1
Lors de cette première séance, j'ai pris en main la carte UCA et le capteur ultrason HC-SR04. J'avais également à disposition le code émetteur que j'avais fait au préalable. Le code semblait correct mais rien ne fonctionnait : le capteur renvoyait systématiquement la valeur zéro dans le moniteur série et plusieurs messages d'erreur apparaissaient lors des téléversements.

Pendant les deux heures de la séance, le professeur et moi avons cherché une solution. Nous avons vérifié le code ligne par ligne, modifié les paramètres de la carte dans l'IDE Arduino, changé les pins utilisés mais rien n'a changé.

Finalement, en toute fin de séance, nous avons découvert que tous les fils utilisés pour relier le capteur à la carte UCA étaient défectueux. Après avoir remplacé les câbles par des nouveaux, le capteur s'est mis à fonctionner correctement et les distances se sont affichées normalement dans le moniteur série
### Séance 2
Cette séance a été particulièrement difficile. J'ai rencontré de nombreux messages d'erreur lors des téléversements, alors que le code semblait pourtant correct. J'ai passé l'intégralité de la séance à essayer de comprendre et de résoudre ces erreurs, sans pouvoir avancer sur le projet lui-même.

Face à ces difficultés et au temps perdu, j'ai pris la décision de simplifier le projet. L'architecture initiale prévoyait deux cartes UCA communiquant entre elles via LoRa, mais cette configuration s'est avérée trop complexe et trop longue à mettre en place dans les délais impartis. J'ai donc choisi de repartir sur une base plus simple : une seule carte UCA qui gère à la fois la détection des paniers, l'affichage du score, et l'envoi des résultats par email en fin de partie.

J'ai dû recommencer le code depuis le début, en fusionnant et modifiant les anciens codes émetteur et récepteur en un seul programme.
### Séance 3
Durant cette séance, j'ai ajouté deux nouveaux composants au projet : un buzzer pour signaler chaque panier marqué par un signal sonore, et un écran OLED pour afficher le score en temps réel.

Le branchement physique de ces composants n'a pas été simple. J'ai eu du mal à identifier les bons pins sur la carte UCA et à comprendre où connecter chaque fil. Après plusieurs essais et corrections, j'ai finalement réussi à tout brancher correctement.

Le code a également nécessité de nombreuses modifications. J'ai dû intégrer les bibliothèques adaptées pour l'écran OLED, gérer l'affichage du score qui augmente de deux points à chaque panier, et synchroniser le buzzer pour qu'il émette un son à chaque détection. Après beaucoup d'ajustements, les deux composants fonctionnent désormais correctement.
### Séance 4
Depuis les séances précédentes et également à la maison, j'essaie de faire fonctionner la partie connexion sans fil du projet. L'objectif est d'utiliser The Things Network pour récupérer les données envoyées par la carte UCA via LoRaWAN, puis de les transmettre à Tago.io qui se chargera d'envoyer un email contenant le score final à la fin de chaque partie.

J'ai continué à travailler sur cette partie pendant la séance. J'ai suivi le tutoriel fourni par le professeur. Le code compile et la carte émet bien des messages, mais je n'arrivais pas encore à recevoir les données sur TTN. Avec de l'aide, j'ai fini par réussir à recevoir les données sur TTN mais je n'arrive pas à faire en sorte que ça marche avec Tago.io.
