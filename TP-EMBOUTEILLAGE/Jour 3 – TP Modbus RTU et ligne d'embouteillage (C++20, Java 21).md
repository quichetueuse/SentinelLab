# Jour 3 – Deux TP pour pratiquer C++20 et Java 21

Oct 7, 2026 · @Denis

## Présentation

Deux TP indépendants du projet SentinelLab, pensés pour pratiquer les nouveautés de C++20 et de Java 21 sur les thèmes du jour 3. Chacun se fait en binôme, en console, sur un créneau de 1 h 45 : le TP 1 à 10 h 45 après le cours sur les interfaces et communications, le TP 2 à 15 h 15 après le cours sur la concurrence et le temps réel.

|  | TP 1 – matin | TP 2 – après-midi |
| --- | --- | --- |
| Thème du syllabus | Interfaces et communications embarquées | Développement concurrent et temps réel |
| Sujet | Un variateur de moteur piloté en Modbus RTU | Une ligne d’embouteillage à quatre postes |
| Côté C++20 | L’esclave Modbus : trames, CRC-16 calculé à la compilation, carte des registres | La ligne : postes en threads, files bornées, arrêt d’urgence |
| Côté Java 21 | Le maître Modbus : lance l’esclave et lui parle par un tube | La même ligne, puis 10 000 commandes sur threads virtuels |
| Notions C++20 | `std::span`, `consteval`, `concept`, `std::format`, `std::optional`, initialiseurs désignés, `operator== = default` | `std::jthread`, `std::stop_token`, `std::latch`, `std::barrier`, `std::counting_semaphore`, `std::atomic::wait` |
| Notions Java 21 | `record`, `sealed`, `switch` avec motifs de record, `ByteBuffer`, `HexFormat`, `ProcessBuilder` | Threads virtuels, `ExecutorService` en try-with-resources, `BlockingQueue`, `CountDownLatch`, `CyclicBarrier`, `Semaphore` |

**Organisation** : l'archive contient une version de départ, qui compile, avec des zones `// TODO` numérotées, et des tests qui échouent tant qu’elles ne sont pas remplies.&#32;

**Déroulé type d’un TP de 1 h 45** : \
10 min de lecture de l’énoncé et de la carte des fichiers, \
55 min sur la partie C++, \
30 min sur la partie Java, \
10 min pour les questions et le commit.

## TP 1  – Piloter un variateur de moteur en Modbus RTU

### Contexte

Modbus RTU est le protocole série le plus répandu dans l’industrie : automates, variateurs, compteurs d’énergie, capteurs sur bus RS-485. Un **maître** interroge, des **esclaves** répondent ; chaque esclave expose une table de registres de 16 bits. On va écrire les deux côtés : l’esclave, un variateur de vitesse pour moteur, en C++20, et le maître, en Java 21. Le maître lance l’esclave comme processus et lui parle par un tube, qui joue le rôle de la liaison série.

```
   console ──► Maitre.java ──── requête (8 octets) ────► variateur (C++20)
   (commandes)   Java 21   ◄──── réponse + CRC ──────    stdin / stdout
                                                         trace sur stderr
```

### Le protocole en une page

Deux fonctions suffisent : 0x03 lit N registres consécutifs, 0x06 écrit un registre. Les valeurs sont en big-endian (poids fort d’abord) ; le CRC final est en little-endian (poids faible d’abord), piège classique. Un esclave qui reçoit une trame qui ne lui est pas adressée, ou dont le CRC est faux, **ne répond pas** : le maître constate un silence.

| Trame | Octets |
| --- | --- |
| Requête 0x03 | `[esclave] [03] [adresse hi lo] [quantité hi lo] [CRC lo hi]` |
| Réponse 0x03 | `[esclave] [03] [2 × quantité] [registre 1 hi lo] … [CRC lo hi]` |
| Requête 0x06 | `[esclave] [06] [registre hi lo] [valeur hi lo] [CRC lo hi]` |
| Réponse 0x06 | écho exact de la requête |
| Exception | `[esclave] [fonction + 0x80] [code] [CRC lo hi]` ; codes 01 fonction illégale, 02 adresse illégale, 03 valeur illégale |

Le CRC est le CRC-16/MODBUS : valeur initiale 0xFFFF, polynôme réfléchi 0xA001. Propriété utile : le CRC d’une trame complète, CRC inclus, vaut 0.

Sur une vraie liaison, une trame se termine par un silence de 3,5 caractères. Sur un tube, l’esclave considère qu’une trame est un bloc reçu par un seul `read()`, ce qui est vrai tant que le maître l’envoie en une écriture.

### Carte des registres du variateur (esclave 0x11)

| Registre | Nom | Accès | Unité ou valeurs |
| --- | --- | --- | --- |
| 0 | Consigne | lecture, écriture | tr/min, 0 à 3 000 |
| 1 | Vitesse | lecture | tr/min, rampe de 1 000 tr/min par seconde |
| 2 | Courant | lecture | dixièmes d’ampère |
| 3 | Température | lecture | dixièmes de °C ; défaut si elle dépasse 80 °C |
| 4 | État | lecture | bit 0 marche, bit 1 défaut, bit 2 surchauffe, bit 3 consigne atteinte |
| 5 | Commande | lecture, écriture | 0 arrêt, 1 marche (refusée en défaut), 2 acquitter (refusé au-dessus de 60 °C) |

### Fichiers

```
tp1-modbus/
├── src/crc16.hpp        C1, C2  table CRC calculée à la compilation
├── src/modbus.hpp       C3, C4  trames, concept Equipement, traitement
├── src/variateur.hpp    C5      modèle du moteur et règles d’écriture
├── src/esclave.cpp              boucle de l’esclave (fourni)
├── tests/test_modbus.cpp        20 vérifications
└── java/Maitre.java     J1 à J4 le maître et ses tests (--tests)
```

### Partie C++20 : l’esclave (55 min)

**C1. Table CRC calculée par le compilateur** – `crc16.hpp`. Remplir la fonction `consteval tableCrc()`. Une fonction `consteval` **doit** être évaluée à la compilation : la table de 256 mots est rangée dans le binaire, aucun calcul au démarrage, ce qui compte sur un microcontrôleur.

**C2. Calcul du CRC et vérification à la compilation.** Écrire la boucle de `crc16(std::span<const uint8_t>)`, puis ajouter un `static_assert` sur l’exemple de la spécification : `01 03 00 00 00 0A` donne `0xCDC5`. Si le CRC est faux, **le programme ne compile plus** : c’est un test qui s’exécute avant même les tests. `std::span` désigne une suite d’octets contigus sans la copier, qu’elle vienne d’un `std::array`, d’un `std::vector` ou d’un tableau C.

**C3. Lecture de registres** – `traiter()` dans `modbus.hpp`, cas 0x03. Refuser une quantité de 0 ou supérieure à 125 (exception 03), refuser une adresse inexistante (exception 02), sinon construire la réponse avec `Trame::ajouter`, `ajouter16` et `terminer`.

`traiter` est un modèle de fonction contraint par le concept `Equipement` : n’importe quel type qui offre `lire()` et `ecrire()` avec les bonnes signatures convient. Les tests s’en servent avec un `FauxEquipement` à base de `std::map`. Comparer avec une classe de base abstraite : même souplesse, mais vérifiée à la compilation et sans vtable.

**C4. Écriture d’un registre**, cas 0x06. Si `eq.ecrire()` renvoie un code d’exception, renvoyer la trame d’exception ; sinon renvoyer l’écho des 6 premiers octets suivis d’un CRC recalculé. Noter la forme `if (const auto refus = eq.ecrire(...))` : déclaration dans la condition, sur un `std::optional`.

**C5. Règles du variateur** – `Variateur::ecrire()`. Appliquer la carte des registres. La ligne `using enum modbus::CodeException;` (C++20) permet d’écrire `ValeurIllegale` sans préfixe.

Vérification :

```
$ cmake -S . -B build && cmake --build build
$ ./build/test_modbus
20 vérification(s), 0 échec(s)
```

Au départ, le programme affiche `20 vérification(s), 16 échec(s)`. \
Les tests utilisent aussi les initialiseurs désignés (`Requete{.esclave = 0x11, .fonction = 0x03, ...}`) et l’opérateur `==` généré par `= default`, deux ajouts de C++20.

### Partie Java 21 : le maître (30 min)

**J1. CRC.** Même algorithme, sans table. Attention à `d[i] & 0xFF` : `byte` est signé en Java.

**J2. Construire une requête** avec `ByteBuffer` (big-endian par défaut, comme Modbus), puis ajouter le CRC poids faible d’abord.

**J3. Décoder la réponse** en un type scellé : `sealed interface Reponse permits Lecture, Ecriture, Refus, Corrompue`, chaque cas étant un `record`. Le compilateur connaît la liste complète des cas possibles.

**J4. Afficher** avec un `switch` sur `Reponse` utilisant les motifs de record : `case Lecture(int adr, int[] v) when adr == 0 && v.length == 6 -> …`. Pas de `default` : le `switch` est exhaustif parce que l’interface est scellée. Ajouter un cinquième type de réponse sans le traiter fait échouer la compilation : c’est voulu.

Le reste est fourni et mérite d’être lu : un **thread virtuel** lit en continu la sortie de l’esclave et pousse chaque octet dans une `BlockingQueue`, ce qui permet d’attendre une réponse avec un délai (`poll(500, MILLISECONDS)`) ; la transaction fait jusqu’à trois essais.

```
$ java java/Maitre.java --tests
6/6 tests réussis
```

### Essais en console

Lancer le maître : il démarre l’esclave, dont la trace s’affiche sur la même console.

```
$ java java/Maitre.java ./build/variateur
[esclave] variateur prêt à l'adresse 0x11, bruit 0 %
> consigne 1500
[esclave] <- 11 06 00 00 05 DC 89 93  -> 11 06 00 00 05 DC 89 93
  consigne <- 1500 : accepté
> marche
  commande <- 1 : accepté
> surveiller 4
  t+ 0.0 s  vitesse    2 tr/min  courant  1.5 A   25.0 °C  MARCHE
  t+ 0.5 s  vitesse  505 tr/min  courant  3.2 A   25.3 °C  MARCHE
  t+ 1.0 s  vitesse 1007 tr/min  courant  4.9 A   25.9 °C  MARCHE
  t+ 1.5 s  vitesse 1500 tr/min  courant  6.5 A   26.9 °C  MARCHE | À LA CONSIGNE
> consigne 3500
[esclave] <- 11 06 00 00 0D AC 8F B7  -> 11 86 03 03 A4
  refus de l'esclave (fonction 0x06) : valeur illégale
> ecrire 1 10
  refus de l'esclave (fonction 0x06) : adresse illégale
> esclave 5
> etat
[esclave] <- 05 03 00 00 00 06 C4 4C  (ignorée)
  [essai 1] pas de réponse de l'esclave 0x05
  …
  réponse inexploitable : échec après 3 essais
```

Les commandes peuvent aussi arriver par un tube, pour rejouer un scénario : `printf 'consigne 1500\nmarche\nsurveiller 4\nquitter\n' | java java/Maitre.java ./build/variateur`.

**Scénario 1 – ligne bruitée.** Relancer avec `--bruit 0.3` : l’esclave inverse un bit dans 30 % de ses réponses. Le maître détecte chaque erreur par le CRC et recommence :

```
  [essai 1] CRC invalide : 11 06 00 05 80 01 5A 9B
  commande <- 1 : accepté
> stats
  transactions 6, reprises sur CRC 5, silences 0
```

**Scénario 2 – surchauffe.** `consigne 3000`, `marche`, puis `surveiller 70`. À pleine vitesse, la température tend vers 100 °C ; la protection se déclenche vers 80 °C, le moteur s’arrête et le défaut reste mémorisé :

```
  t+24.0 s  vitesse 3000 tr/min  courant 11.5 A   75.8 °C  MARCHE | À LA CONSIGNE
  t+28.0 s  vitesse 2849 tr/min  courant  9.5 A   80.1 °C  ARRÊT | DÉFAUT | SURCHAUFFE
  t+31.0 s  vitesse    0 tr/min  courant  0.0 A   77.0 °C  ARRÊT | DÉFAUT
```

`marche` et `acquitter` sont refusés tant que la température dépasse 60 °C ; une fois le moteur refroidi, `acquitter` puis `marche` relancent le variateur.

### Questions

1. Pourquoi l’esclave reste-t-il muet au lieu de renvoyer une exception quand le CRC est faux ?
2. Le maître ne peut pas savoir combien d’octets lire avant d’avoir reçu l’en-tête. Comment `lireReponse()` déduit-elle la longueur ? Que se passerait-il si le bruit touchait l’octet de fonction ?
3. Quelle différence concrète, dans le binaire produit, entre `consteval` et `constexpr` pour `tableCrc()` ?
4. Réécrire `traiter()` avec une classe de base `IEquipement` à méthodes virtuelles. Qu’y gagne-t-on, qu’y perd-on ?
5. Pourquoi la lecture de la sortie de l’esclave est-elle faite dans un thread séparé, plutôt que directement avec `InputStream.read()` dans la transaction ?

### Pour aller plus loin

Ajouter la fonction 0x10 (écriture de plusieurs registres). Brancher deux esclaves sur le même « bus » (un processus intermédiaire qui diffuse les requêtes). Remplacer le tube par une vraie liaison série virtuelle avec `socat -d -d pty,raw,echo=0 pty,raw,echo=0` et la bibliothèque jSerialComm.

## TP 2  – Une ligne d’embouteillage concurrente

### Contexte

Une ligne de production est un cas d’école de concurrence : des postes qui travaillent en parallèle, reliés par des convoyeurs de capacité limitée, avec des moments où tous doivent s’arrêter ensemble. On la modélise avec un thread par poste. Le poste le plus lent impose sa cadence à toute la ligne ; le but du TP est de le voir, de le mesurer, puis de manipuler les outils de synchronisation de C++20 et de Java 21.

&#91;embedded content: ligne d'embouteillage · 5 postes, 4 convoyeurs, 1 barrière\]

La ligne produit 3 lots de 40 bouteilles. Une bouteille sur 17 est rejetée au contrôle. La console accepte quatre commandes pendant l’exécution : `pause`, `reprise`, `stats`, `urgence`.

### Fichiers

```
tp2-embouteillage/
├── src/file_bornee.hpp   L1      convoyeur : deux sémaphores et un mutex
├── src/ligne.cpp         L2 à L5 postes, latch, barrière, pause, arrêt
├── src/course.cpp        R1, R2  course critique : sans protection, atomic_ref, mutex
├── tests/test_file.cpp           13 vérifications, sous ThreadSanitizer
└── java/Ligne.java       V1 à V3 la ligne et 10 000 commandes en Java 21
```

### Partie C++20 (55 min)

**R1, R2. Voir une course critique (10 min)** – `course.cpp`. Quatre threads incrémentent un compteur commun dix millions de fois chacun. La version sans protection est fournie ; écrire la version `std::atomic_ref` (C++20 : opérations atomiques sur un `int` ordinaire, sans changer son type) et la version `std::mutex` avec `std::scoped_lock`. Lancer plusieurs fois :

```
$ ./build/course
sans protection   27898970 / 40000000  (  16.9 ms)
atomic_ref        40000000 / 40000000  ( 276.2 ms)
mutex             40000000 / 40000000  ( 948.7 ms)
```

Le résultat non protégé change à chaque exécution et peut même tomber juste : un test qui passe ne prouve pas l’absence de course. Les temps donnent le prix de la correction ; ils varient beaucoup selon le nombre de cœurs.

**L1. Le convoyeur (15 min)** – `file_bornee.hpp`. Écrire `deposer` et `retirer` avec le schéma à deux sémaphores : `places_` compte les places libres, `presents_` les bouteilles présentes, et un `std::mutex` protège seulement l’accès au tableau. `std::counting_semaphore` n’est pas interruptible : la fonction fournie `acquerir` l’attend par tranches de 20 ms en vérifiant le `std::stop_token`, ce qui permet l’arrêt d’urgence.

```
$ ctest --test-dir build --output-on-failure
100% tests passed, 0 tests failed out of 1
```

Les tests sont compilés avec ThreadSanitizer (`-fsanitize=thread`), qui signale toute course de données qu’il observe. Au départ : `13 vérification(s), 11 échec(s)`.

**L3. Démarrage synchronisé** – `travailler()`. Chaque poste appelle `pret.arrive_and_wait()` sur un `std::latch` de 5 : aucun ne démarre avant que tous soient prêts. Un latch ne sert qu’une fois.

**L2. Fin de lot** – `FinDeLot::operator()`. Toutes les 40 bouteilles, chaque poste attend sur un `std::barrier`. Quand le cinquième arrive, la barrière exécute **une seule fois** la fonction de fin de lot, puis libère tout le monde : la barrière, elle, se réutilise à chaque lot. Écrire l’affichage du bilan et la demande d’arrêt après le dernier lot. La fonction doit être `noexcept`, imposé par `std::barrier`.

**L4. Pause sans attente active** – `l.enPause.wait(true)` bloque le thread tant que la valeur vaut `true`, sans consommer de CPU ; la console fait `notify_all()` à la reprise. C’est `std::atomic::wait`, nouveau en C++20.

**L5. Sortie propre** – un poste qui quitte la boucle (arrêt d’urgence) appelle `arrive_and_drop()` : il se retire de la barrière, sinon les postes qui y attendent resteraient bloqués pour toujours. Faire l’essai : supprimer cette ligne et lancer `--urgence-apres 1500`.

Le reste est fourni et mérite d’être lu : la `std::stop_source` commune à tous les postes ; le `std::stop_callback` qui, au moment de l’arrêt, débloque les postes en pause et réveille `main` ; les `std::jthread` qui se rejoignent automatiquement en sortie de bloc ; le thread console, qui utilise lui le `stop_token` propre de son `jthread`.

### Essais en console

```
$ ./build/ligne
ligne : 3 lots de 40 bouteilles, cadence 20 ms, remplissage 30 ms
[lot 1] 40 bouteilles en 1283 ms, cadence 31.2 b/s, rejets cumulés 2
         changement de format...
[lot 2] 40 bouteilles en 1283 ms, cadence 31.2 b/s, rejets cumulés 4
         changement de format...
[lot 3] 40 bouteilles en 1283 ms, cadence 31.2 b/s, rejets cumulés 7

poste         traitées  occupé  attente amont  attente aval  changement format
alimentation       120     63 %            0 %          21 %               16 %
remplissage        120     94 %            2 %           0 %                4 %
bouchage           120     63 %           34 %           0 %                3 %
étiquetage         120     78 %           21 %           0 %                1 %
contrôle           120     32 %           68 %           0 %                0 %
durée 3.85 s, 120 bouteilles contrôlées, 7 rejetées
```

Lire le tableau : le remplissage est occupé 94 % du temps, c’est le goulot. L’alimentation, plus rapide, attend que le convoyeur aval se libère (21 %) ; les postes suivants attendent des bouteilles en amont. La cadence de 31 bouteilles par seconde est celle du remplissage (une toutes les 30 ms), un peu réduite par les changements de format.

**Expérience 1 – déplacer le goulot.** `./build/ligne --remplissage 15` : c’est maintenant l’étiquetage (25 ms) qui est occupé 94 % du temps. Accélérer un poste qui n’est pas le goulot ne sert à rien.

**Expérience 2 – pause et urgence.** Lancer `./build/ligne`, taper `stats`, `pause`, attendre, `stats` à nouveau (les compteurs n’avancent presque plus), `reprise`, puis `urgence` :

```
[ligne] en pause
  alimentation    20 bouteilles
  remplissage     16 bouteilles
[ligne] reprise
[lot 1] interrompu après 31 bouteilles
```

**Expérience 3 – arrêt d’urgence programmé.** `./build/ligne --urgence-apres 1500` : la ligne s’arrête au milieu du deuxième lot, chaque poste sort proprement et le tableau final s’affiche.

### Partie Java 21 (30 min)

**V1. Le poste** – `travailler()` dans `Ligne.java`. Même logique avec les outils Java : `BlockingQueue.take()` et `put()` pour les convoyeurs, `CountDownLatch` pour le démarrage, `CyclicBarrier.await()` pour le changement de format. Chaque poste est un `record` et tourne sur un thread virtuel.

**V2. Fin de lot** : l’action passée au constructeur de `CyclicBarrier`, exécutée une fois par lot comme la fonction de `std::barrier`.

```
$ java java/Ligne.java ligne
[lot 1] 40 bouteilles en 1323 ms, cadence 30.2 b/s
[lot 2] 40 bouteilles en 1287 ms, cadence 31.1 b/s
[lot 3] 40 bouteilles en 1284 ms, cadence 31.2 b/s

3 lots, 120 bouteilles en 3936 ms
```

**V3. 10 000 commandes clients.** Chaque commande attend 10 ms une réponse d’un système de gestion (simulée par `sleep`). Écrire `chronometrer()` : soumettre les commandes puis fermer l’`ExecutorService` dans un try-with-resources, ce qui attend la fin de toutes les tâches (Java 19 et plus). Comparer trois exécuteurs :

```
$ java java/Ligne.java commandes
10000 commandes de 10 ms chacune
  100 threads de plateforme        :  1073 ms
  un thread virtuel par commande   :   378 ms
  virtuels, 50 accès simultanés    :  2114 ms
```

Avec 100 threads de plateforme, on traite au mieux 100 commandes toutes les 10 ms, soit environ 1 s. Un thread virtuel par commande lance les 10 000 attentes en même temps : le temps total ne dépend plus que du coût de création, quelques centaines de millisecondes au plus. Le `Semaphore` de 50 permis simule une ressource partagée limitée (connexions à une base) : on retrouve 10 000 / 50 × 10 ms = 2 s. Les threads virtuels suppriment le coût des threads, pas la limite des ressources.

### Questions

1. Pourquoi faut-il deux sémaphores **et** un mutex dans `FileBornee` ? Que se passe-t-il avec un seul mutex et une attente active ?
2. Quelle différence entre `std::latch` et `std::barrier` ? Pourquoi a-t-on besoin des deux dans la ligne ?
3. Avec une alimentation à 20 ms et un remplissage à 30 ms, combien de bouteilles attendent sur le premier convoyeur en régime établi ? Que changerait un convoyeur de 100 places ?
4. Pourquoi la fonction de fin de lot peut-elle modifier `debutLot` sans verrou ?
5. Dans quel cas un thread virtuel n’apporte-t-il rien par rapport à un thread de plateforme ?

### Pour aller plus loin

Doubler le poste goulot (deux threads de remplissage sur les mêmes convoyeurs) et adapter le comptage par lot. Remplacer la `FileBornee` à verrou par une file à un producteur et un consommateur sans verrou (`std::atomic` sur les indices). Mesurer la gigue de l’alimentation comme dans le cours de l’après-midi.

## Aide-mémoire, évaluation et ressources

### Ce que les deux TP font pratiquer

| Nouveauté | Version | À quoi elle sert | Où dans les TP |
| --- | --- | --- | --- |
| `std::span` | C++20 | Vue sur des octets contigus, sans copie ni pointeur + taille séparés | TP 1 : `crc16`, `decoderRequete`, `traiter` |
| `consteval` | C++20 | Fonction obligatoirement évaluée à la compilation | TP 1 : `tableCrc` |
| `concept`, `requires` | C++20 | Contraindre un modèle ; polymorphisme vérifié à la compilation | TP 1 : `Equipement` |
| Initialiseurs désignés | C++20 | `Requete{.esclave = 0x11, ...}` : champs nommés à l’initialisation | TP 1 : `decoderRequete`, tests |
| `operator== = default` | C++20 | Comparaison générée par le compilateur | TP 1 : `Requete` |
| `using enum` | C++20 | Énumérateurs sans préfixe dans une portée | TP 1 : `Variateur::ecrire` |
| `std::format` | C++20 | Formatage typé et sûr, remplace `printf` | TP 1 : `hex` ; TP 2 : affichages |
| `std::jthread`, `std::stop_token`, `std::stop_source`, `std::stop_callback` | C++20 | Thread qui se rejoint seul ; arrêt coopératif | TP 2 : postes, console, arrêt d’urgence |
| `std::latch`, `std::barrier` | C++20 | Rendez-vous unique ; rendez-vous réutilisable avec action | TP 2 : démarrage, changement de format |
| `std::counting_semaphore` | C++20 | Compter des ressources : places, éléments | TP 2 : `FileBornee` |
| `std::atomic::wait` / `notify_all` | C++20 | Attendre un changement de valeur sans attente active | TP 2 : pause, fin de `main` |
| `std::atomic_ref` | C++20 | Opérations atomiques sur une variable ordinaire | TP 2 : `course.cpp` |
| `record` | Java 16 | Classe de données immuable en une ligne | TP 1 : réponses ; TP 2 : `Bouteille`, `Poste` |
| `sealed interface` | Java 17 | Liste fermée des sous-types, connue du compilateur | TP 1 : `Reponse` |
| `switch` à motifs, motifs de record, `when` | Java 21 | Déstructurer et choisir selon le type, de façon exhaustive | TP 1 : `afficher`, `transaction` |
| `HexFormat` | Java 17 | Afficher et lire de l’hexadécimal | TP 1 |
| Threads virtuels | Java 21 | Des milliers de tâches bloquantes sans épuiser les threads | TP 1 : lecteur ; TP 2 : postes, commandes |
| `ExecutorService` en try-with-resources | Java 19 | Fermer et attendre toutes les tâches en sortie de bloc | TP 2 : `ligne`, `chronometrer` |
| `List.getLast()` | Java 21 | Collections ordonnées (`SequencedCollection`) | TP 2 : `ligne` |

### Évaluation

Chaque TP est noté sur 4, à partir du commit et d’une démonstration de deux minutes :

| Critère | TP 1 – Modbus | TP 2 – Embouteillage |
| --- | --- | --- |
| Tests C++ au vert (1 pt) | `test_modbus` : 20/20 | `test_file` : 13/13 sous ThreadSanitizer |
| Partie Java fonctionnelle (1 pt) | `--tests` : 6/6 et dialogue avec l’esclave | `ligne` et `commandes` s’exécutent |
| Démonstration (1 pt) | Refus 0x03 et 0x02, silence d’un esclave absent, reprise sur bruit | Goulot identifié dans le tableau, pause, arrêt d’urgence propre |
| Questions (1 pt) | Trois réponses sur cinq justes et justifiées | Trois réponses sur cinq justes et justifiées |

### Ressources

- [cppreference : nouveautés de C++20](https://en.cppreference.com/w/cpp/20), avec un exemple pour chaque ajout.
- [Modbus Application Protocol Specification](https://modbus.org/specs.php) et la spécification de la liaison série RTU, gratuites sur le site de la Modbus Organization.
- [C++ Core Guidelines, section CP](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#S-concurrency) : règles de concurrence.
- [JEP 444 : Virtual Threads](https://openjdk.org/jeps/444) et [JEP 441 : Pattern Matching for switch](https://openjdk.org/jeps/441).
- [dev.java](https://dev.java/learn/) : chapitres sur les records, les classes scellées et les threads virtuels.
- [Operating Systems: Three Easy Pieces](https://pages.cs.wisc.edu/~remzi/OSTEP/), chapitres 30 et 31 sur les variables de condition et les sémaphores.
