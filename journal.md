#Problèmes rencontrés

##TP 1:

- CMAKE n'était pas installé (je pensais que ca venais avec cl et g++)

##TP 2:

- Bien comprendre ce qui était attendu

##TP JOUR 2

- Le code fourni est spécifique à LINUX et ne fonctionne donc pas sur windows, impossible de build

## Globaux

Certains TP étaient prévus pour être réalisés sous LINUX, j'ai donc du mettre en place du WSL pour pouvoir les réalisés.

Voir plus en détails la manière dont bien se servir de git (gestion des branches, gestion des tags, cherrypick, rebase, merge).
Mettre en pratique avec des éxemples concret la méthode solide, avec une comparaison sans/avec pour voir les différents avantages de celle-ci plutot que des exemples abstraits.

## Commandes utiles

- **Demarrer mosquito**: `mosquitto_sub -t 'eidl/#' -v`
- **Compiler le noeud en C++**: `g++ -std=c++20 main.cpp -o noeud -lmosquitto`
- **Compiler et demarrer la passerelle en java**: `mvn clean compile exec:java`
- **Creer des certificat openssl**: `openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out server.crt -days 365`
- **Push sur github avec les tags**: `git push origin main --tags`
- **Commit sur git avec un message**: `git commit -m "message"`
- **Lire le contenu des trames MQTT non sécurisé**: `printf '\xAA\xFF%0255d' 0 | ./parse_vuln`
- **Verifier que le noeud C++ ne contient pas de fuite de mémoire**: `cppcheck --enable=warning,style parse_vuln.cpp main.cpp`
- **Réaliser un stress test**: `stress-ng --cpu 4 --timeout 10s &`
- **Installer maven et java**: `sudo apt install default-jdk maven`
