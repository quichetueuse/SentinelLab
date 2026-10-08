# TP MODBUS

## Pourquoi l’esclave reste-t-il muet au lieu de renvoyer une exception quand le CRC est faux ?

Pour éviter de causer des problemes aux autre équipements peut être concernés par le CRC.

## Le maître ne peut pas savoir combien d’octets lire avant d’avoir reçu l’en-tête. Comment lireReponse() déduit-elle la longueur ? Que se passerait-il si le bruit touchait l’octet de fonction ?

Pour connaitre la structure et la longueur d'une trame, on peut regarder son entête puisqu'il contient des détails. Dans le cas ou le bruits viendrait toucher l'entête, cela rendrait les données invalides et causerai des erreurs dans la trâme.

## Quelle différence concrète, dans le binaire produit, entre consteval et constexpr pour tableCrc() ?

consteval force tableCRC à être évaluer à la compilation tandis que constexpr autorise tableCrc à être évalué dynamiquement lors de la compilation, mais également à l'éxécution.

## Réécrire traiter() avec une classe de base IEquipement à méthodes virtuelles. Qu’y gagne-t-on, qu’y perd-on ?

Dans le cas d'une réecriture, on gagnerai une meilleure flexibilité du code via du polymorphisme. Mais on perdrais en sécurité et en performance car cela introduit des tables supplémentaire à la compilation.

## Pourquoi la lecture de la sortie de l’esclave est-elle faite dans un thread séparé, plutôt que directement avec InputStream.read() dans la transaction ?

L'esclave envoit des messages de manière asynchrone, et si la partie qui gère la communication vient lire au moment ou un message est envoyé, cela peut bloquer tout le programme.

# TP EMBOUTEILLAGE

## Pourquoi faut-il deux sémaphores et un mutex dans FileBornee ? Que se passe-t-il avec un seul mutex et une attente active ?

Les 2 sémaphores permettent aux machines d'attendre lorsqu'elles sont vides ou que la machine suivante est pleine (ligne saturé). Le mutex vient protéger la tete et la queue lors d'une modification

## Quelle différence entre std::latch et std::barrier ? Pourquoi a-t-on besoin des deux dans la ligne ?

La latch reste ouverte une fois que le compteur à atteint 0. la barrier en revanche vient se refermer après que le compteur à atteint 0 et qu'une fonction ai été éxecutée. Dans le code, la latch permet d'attendre que tous les postes soient pret afin de lancer les machines. Et la barrier permet la validation d'un lot puis se relance au prochain lot

## Avec une alimentation à 20 ms et un remplissage à 30 ms, combien de bouteilles attendent sur le premier convoyeur en régime établi ? Que changerait un convoyeur de 100 places ?

4 bouteilles attendent sur le premier convoyeur. Le convoyeur avec 100 places ne changerai rien car au bout d'un certains temps, le convoyeur ne change pas le fait que l'alimentation envoi plus de bouteilles que ce que le remplissage ne peut tenir, il y a donc un goulot d'étranglement sur le remplissage (66% d'éfficacité).

## Pourquoi la fonction de fin de lot peut-elle modifier debutLot sans verrou ?

Seul le thread arrivé en dernier à la barrière intéragit avec debutlot, tous les autre attendent le redémarrage de la ligne.

## Dans quel cas un thread virtuel n’apporte-t-il rien par rapport à un thread de plateforme ?

Dans le cas ou les 2 threads essayent d'intéragir avec une ressource partagé et que c'est cette même ressources le goulot d'étranglement. Il y a également le cas ou les 2 threads sont a 100% avec des calculs en permanence.
