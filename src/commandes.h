/* 
Fonctionnalité de message privé
Prends en paramètre l'utilisateur dest à qui envoyer le mp,l'utilisateur expe qui envoie le mp ainsi que le message msg à envoyer
Envoie un message que seule la personne en paramètre et la personne qui a envoyé le message peut voir
*/
void* messagePrive(int expe, int dest, char* msg);
//envoie la liste des fonctions disponibles à l'utilisateur d'id user
void help(int user);
//lit le string str, detecte si une fonction est demandé, lance la fonction si elle exite, un msg d'erreurs à l'utilisateur sinon
void detectFonc(char* str);
//deconnecte l'utilisateur d'id user
void deco(int user);