# TP Jour 2 : un nœud qui tient son budget

Nœud SentinelLab simulé (C++20) et passerelle (Java 21), entièrement en console.

## Construire et tester

    cmake -S . -B build && cmake --build build
    ctest --test-dir build --output-on-failure
    ./build/noeud --help

## Contenu

    src/ring_buffer.hpp   A1  file circulaire sans allocation
    src/alloc_guard.cpp   A2  garde d'allocation (operator new global)
    src/demo_bugs.cpp     A3  bugs volontaires pour ASan et Valgrind
    java/BenchMemoire.java A4 ArrayList<Double> contre double[]
    src/noeud.cpp         A5, B2 à B5  nœud simulé
    src/gpio.hpp          B1  registres et opérations sur les bits
    java/Passerelle.java  B6  passerelle avec arrêt propre
    outils/superviseur.sh, outils/rafale.sh
    tests/test_unitaires.cpp

Les zones à compléter sont marquées `// TODO <étape>`. Le programme compile
dès le départ ; les tests échouent tant que A1 et B1 ne sont pas faits.
