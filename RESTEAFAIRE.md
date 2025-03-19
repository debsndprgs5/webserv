# Alias 
        Rajouter la map alias dans server/methods
        Faire une function search replace 

## Trunk
        Trouver la variable Content Length ou chunked dans parsing http
        Finir la function isFull()-> elle retourne true h24

### Reponse 
        Formatter la reponse en HTTP/1.1
        Faire une boucle for pour envoyer la reponse,(on deecoupe string tout les 1024)

### CGI 
        Faire un execve , balancer le tout dans un fd tempo
        Formatter la reponse avec le content de fd tempo 

les trucs qu'on a pas encore : 
    • Votre programme doit prendre un fichier de configuration en argument *** ou utiliser un chemin par défaut ***
    Définir un répertoire ou un fichier à partir duquel le fichier doit être recherché 
    /kapouet est rootée sur /tmp/www  -> l’url /kapouet/pouic/toto/pouet -> /tmp/www/pouic/toto/pouet.
    *** pas trop compris *** , perso j'ai root + uri dans l'exemple ca ferais /tmp/www/kapouet/pouic/toto/pouet
    *** Set un fichier par défaut comme réponse si la requête est un répertoire *** 

    Aucun Alias mentionner dans sujet ? 
    
