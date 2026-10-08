// Ligne.java : la ligne d'embouteillage en Java 21, puis 10 000 commandes clients
// traitées par des threads de plateforme ou des threads virtuels.
//   java java/Ligne.java ligne [lots] [taille]
//   java java/Ligne.java commandes
import java.time.Duration;
import java.time.Instant;
import java.util.List;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.BrokenBarrierException;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.CyclicBarrier;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Semaphore;
import java.util.concurrent.atomic.AtomicInteger;

public class Ligne {
    record Bouteille(int numero, int lot) {}

    // Un poste : un nom, une durée de travail, un convoyeur amont et aval (null aux extrémités).
    record Poste(String nom, long dureeMs, BlockingQueue<Bouteille> amont, BlockingQueue<Bouteille> aval,
                 AtomicInteger traitees) {
        Poste(String nom, long dureeMs, BlockingQueue<Bouteille> amont, BlockingQueue<Bouteille> aval) {
            this(nom, dureeMs, amont, aval, new AtomicInteger());
        }
    }

    static volatile boolean fini = false;
    static final AtomicInteger lotCourant = new AtomicInteger(1), dansLot = new AtomicInteger();
    static Instant debutLot = Instant.now();

    static void travailler(Poste p, int tailleLot, CountDownLatch pret, CyclicBarrier format)
                        throws InterruptedException, BrokenBarrierException {
        pret.countDown();
        pret.await();
        int numero = 0, n = 0;
        while (!fini) {
            for (int i = 0; i < tailleLot; i++) {
                // récupère la bouteille ou crée
                Bouteille bouteille = (p.amont() == null ? new Bouteille(++numero, lotCourant.get()) : p.amont().take());

                // temps de travail
                Thread.sleep(p.dureeMs());

                if (p.aval() == null) p.traitees().incrementAndGet();
                else p.aval().put(bouteille);

            }
            format.await();
        }
    }

    static void ligne(int lots, int tailleLot) throws Exception {
        List<BlockingQueue<Bouteille>> c = List.of(new ArrayBlockingQueue<>(4), new ArrayBlockingQueue<>(4),
                new ArrayBlockingQueue<>(4), new ArrayBlockingQueue<>(4));
        List<Poste> postes = List.of(
                new Poste("alimentation", 20, null, c.get(0)),
                new Poste("remplissage", 30, c.get(0), c.get(1)),
                new Poste("bouchage", 20, c.get(1), c.get(2)),
                new Poste("étiquetage", 25, c.get(2), c.get(3)),
                new Poste("contrôle", 10, c.get(3), null));
        CountDownLatch pret = new CountDownLatch(postes.size());

        Runnable finDeLot = () -> {
            long temps = Duration.between(debutLot, Instant.now()).toMillis();
            double cadence = (tailleLot * 1000.0) / (temps == 0 ? 1 : temps);
            // Afiche le bilan
            System.out.printf("[Lot %d] %d bouteilles en %d ms - cadence de %.1f b/s%n", lotCourant.get(), tailleLot, temps, cadence);

            if (lotCourant.incrementAndGet() > lots) {
                fini = true;
            } else {
                debutLot = Instant.now();
            }
        };
        CyclicBarrier format = new CyclicBarrier(postes.size(), finDeLot);

        debutLot = Instant.now();
        Instant debut = Instant.now();
        // Java 21 : l'ExecutorService se ferme (et attend ses tâches) en sortie de try.
        try (ExecutorService equipe = Executors.newVirtualThreadPerTaskExecutor()) {
            for (Poste p : postes)
                equipe.submit(() -> { travailler(p, tailleLot, pret, format); return null; });
        }
        System.out.printf("%n%d lots, %d bouteilles en %d ms%n", lots,
                postes.getLast().traitees().get(), Duration.between(debut, Instant.now()).toMillis());
    }

    // ------------------------------------------------- 10 000 commandes clients
    // Chaque commande attend 10 ms une réponse du système de gestion (simulé par sleep).
    static void commande(Semaphore acces) {
        try {
            if (acces != null) acces.acquire();
            try { Thread.sleep(10); } finally { if (acces != null) acces.release(); }
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
    }

    static long chronometrer(ExecutorService ex, int n, Semaphore acces) {
        // chronometre le temps d'une bouteille
        Instant t0 = Instant.now();
        try (ex) {
            for (int i = 0; i < n; i++) {
                ex.submit(() -> {
                    commande(acces);
                    return null;
                });

            }
        }
        return Duration.between(t0, Instant.now()).toMillis();
    }

    static void commandes() {
        int n = 10_000;
        System.out.printf("%d commandes de 10 ms chacune%n", n);
        System.out.printf("  100 threads de plateforme        : %5d ms%n",
                chronometrer(Executors.newFixedThreadPool(100), n, null));
        System.out.printf("  un thread virtuel par commande   : %5d ms%n",
                chronometrer(Executors.newVirtualThreadPerTaskExecutor(), n, null));
        System.out.printf("  virtuels, 50 accès simultanés    : %5d ms%n",
                chronometrer(Executors.newVirtualThreadPerTaskExecutor(), n, new Semaphore(50)));
    }

    public static void main(String[] args) throws Exception {
        String mode = args.length > 0 ? args[0] : "ligne";
        switch (mode) {
            case "ligne" -> ligne(args.length > 1 ? Integer.parseInt(args[1]) : 3,
                                  args.length > 2 ? Integer.parseInt(args[2]) : 40);
            case "commandes" -> commandes();
            default -> System.err.println("usage : java Ligne.java ligne [lots] [taille] | commandes");
        }
    }
}
