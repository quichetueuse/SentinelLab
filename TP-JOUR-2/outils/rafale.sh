#!/usr/bin/env bash
# rafale.sh : envoie N appuis bouton (SIGUSR1) espacés de 50 ms au processus PID.
# usage : ./outils/rafale.sh PID [N]
pid=$1; n=${2:-10}
for _ in $(seq 1 "$n"); do kill -USR1 "$pid"; sleep 0.05; done
echo "[rafale] $n appuis envoyés à $pid" >&2
