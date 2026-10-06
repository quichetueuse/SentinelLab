// BenchMemoire.java : coût mémoire et temps de ArrayList<Double> contre double[].
// Lancement sans compilation préalable :  java BenchMemoire.java [n]
// Puis sous contrainte :                  java -Xmx32m BenchMemoire.java
import java.util.ArrayList;
import java.util.List;

public class BenchMemoire {
    static long utilisee() {
        Runtime r = Runtime.getRuntime();
        for (int i = 0; i < 3; i++) System.gc();
        return r.totalMemory() - r.freeMemory();
    }

    public static void main(String[] args) {
        int n = args.length > 0 ? Integer.parseInt(args[0]) : 1_000_000;
        System.out.printf("tas maximal : %d Mo, %,d mesures%n",
                Runtime.getRuntime().maxMemory() >> 20, n);

        // 1) Tableau primitif : 8 octets par valeur, contigus
        long avant = utilisee();
        double[] tableau;
        try {
            tableau = new double[n];
        } catch (OutOfMemoryError e) {
            System.out.println("double[]          : OutOfMemoryError, réduisez n");
            return;
        }
        for (int i = 0; i < n; i++) tableau[i] = 20.0 + (i % 100) / 10.0;
        long octetsTableau = utilisee() - avant;
        long t0 = System.nanoTime();
        double s1 = 0;
        for (double v : tableau) s1 += v;
        long nsTableau = System.nanoTime() - t0;
        System.out.printf("double[]          : %6.1f Mo  (%4.1f octets/mesure), somme en %5.1f ms%n",
                octetsTableau / 1e6, (double) octetsTableau / n, nsTableau / 1e6);
        tableau = null;

        // 2) Liste d'objets : un Double (en-tête + valeur) + une référence par valeur
        try {
            avant = utilisee();
            List<Double> liste = new ArrayList<>();
            for (int i = 0; i < n; i++) liste.add(20.0 + (i % 100) / 10.0);   // autoboxing
            long octetsListe = utilisee() - avant;
            t0 = System.nanoTime();
            double s2 = 0;
            for (Double v : liste) s2 += v;                                      // unboxing
            long nsListe = System.nanoTime() - t0;
            System.out.printf("ArrayList<Double> : %6.1f Mo  (%4.1f octets/mesure), somme en %5.1f ms%n",
                    octetsListe / 1e6, (double) octetsListe / n, nsListe / 1e6);
            System.out.printf("rapport mémoire   : x%.1f%n", (double) octetsListe / octetsTableau);
            if (Math.abs(s1 - s2) > 1e-6) System.out.println("sommes différentes !");
        } catch (OutOfMemoryError e) {
            System.out.println("ArrayList<Double> : OutOfMemoryError, le tas est trop petit pour cette structure");
        }
    }
}
