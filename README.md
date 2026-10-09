# SentinelLab

Projet "fil rouge" de la matière programmation C++ / Java embarqué.

Contient différents TPs comme:

- TP EMBOUTEILLAGE : tp qui vient simuler une ligne de production de mise en bouteilles.
- TP qui vient simuler des Capteurs.
- TP qui vient simuler une station Météo.
- TP qui vient intéragir avec un bus MODBUS et I2C.
- TP qui vient simuler une architecture avec le protocole MQTT.
- TP qui vient mettre en lumière les failles présentes dans le protocole MQTT

# Comment lancer le projet SentinelLab

Clonez le repo github:

```bash
git clone https://github.com/quichetueuse/SentinelLab.git
```

Installer les paquets necessaire

```bash
sudo apt install cmake build-essential valgrind openjdk-25-jre-headless stress-ng default-jdk maven cppcheck -y
```

Allez dans le dossier TP7/noeud puis lancer la compilation du noeud c++

```bash
Cd ~/SentinelLab/TP7/noeud
```

```bash
g++ -std=c++20 main.cpp -o noeud -lmosquitto
```

Allez dans le dossier TP7/passerelle puis lancer la compilation de la passerelle java

```bash
Cd ~/SentinelLab/TP7/passerelle
```

```bash
mvn clean compile exec:java
```

Ouvrez 3 terminaux différents.

Dans le premier terminal, allez dans le dossier TP7/passerelle puis lancer la compilation de la passerelle java

```bash
Cd ~/SentinelLab/TP7/passerelle
```

```bash
mvn clean compile exec:java
```

Dans le 2eme terminal, lancer le brocker mosquito (l'installer si besoin)

```bash
mosquitto_sub -t 'eidl/#' -v
```

Dans le troisième terminal, allez dans le dossier du noeud c++ et lancer le noeud

```bash
Cd ~/SentinelLab/TP7/noeud
```

```bash
./noeud
```
