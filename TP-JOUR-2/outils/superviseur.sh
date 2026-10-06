#!/usr/bin/env bash
# superviseur.sh : relance le programme donné en argument tant qu'il ne
# s'arrête pas proprement (code 0), comme le ferait un reset matériel.
# usage : MAX_REDEMARRAGES=3 ./outils/superviseur.sh ./build/noeud --bloquer-a 5
max=${MAX_REDEMARRAGES:-5}
n=0
while true; do
  REDEMARRAGES=$n "$@"
  code=$?
  if [ "$code" -eq 0 ]; then
    echo "[superviseur] arrêt propre (code 0), fin de la supervision" >&2
    exit 0
  fi
  n=$((n + 1))
  echo "[superviseur] arrêt anormal (code $code) : redémarrage n°$n" >&2
  if [ "$n" -ge "$max" ]; then
    echo "[superviseur] $max redémarrages : abandon, intervention nécessaire" >&2
    exit 1
  fi
  sleep 1
done
