// Passerelle.java : lit les lignes du nœud sur l'entrée standard, calcule une
// moyenne glissante sans allocation par mesure, enregistre en CSV et s'arrête
// proprement sur SIGTERM ou Ctrl+C grâce à un shutdown hook.
// usage : ./build/noeud | java java/Passerelle.java [fichier.csv]
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Passerelle {
    private static final Pattern LIGNE = Pattern.compile("t=\\s*(\\d+) ms\\s+T=\\s*(-?\\d+\\.\\d+)");
    private static final int FENETRE = 10;

    private final double[] fenetre = new double[FENETRE];   // tableau circulaire de taille fixe
    private int indice = 0, remplies = 0;
    private double somme = 0;                                // somme glissante : O(1) par mesure
    private long nbMesures = 0;
    private final BufferedWriter csv;
    private boolean ferme = false;

    Passerelle(String fichier) throws IOException {
        csv = new BufferedWriter(new FileWriter(fichier), 64 * 1024);   // tampon de 64 Ko
        csv.write("t_ms,temperature,moyenne\n");
    }

    double ajouter(double t) {
        somme += t - fenetre[indice];        // retire la valeur sortante, ajoute la nouvelle
        fenetre[indice] = t;
        indice = (indice + 1) % FENETRE;
        if (remplies < FENETRE) remplies++;
        return somme / remplies;
    }

    synchronized void enregistrer(long tMs, double t, double moyenne) throws IOException {
        if (ferme) return;
        csv.write(tMs + "," + t + "," + String.format(java.util.Locale.ROOT, "%.2f", moyenne) + "\n");
        nbMesures++;
    }

    synchronized void fermer() {
        if (ferme) return;
        try {
            csv.flush();
            csv.close();
            System.err.println("Résumé: " + nbMesures + " mesures enregistrées dans le csv");
        } catch(IOException e) {
            System.err.println("Une erreur est survenue lors de la fermeture du fichier csv: " + e.getMessage());
        }
    }

    static String vmRss() {
        try {
            for (String l : Files.readAllLines(Path.of("/proc/self/status")))
                if (l.startsWith("VmRSS:")) return l.substring(6).trim();
        } catch (IOException e) { /* hors Linux */ }
        return "inconnue";
    }

    public static void main(String[] args) throws IOException {
        String fichier = args.length > 0 ? args[0] : "mesures.csv";
        Passerelle p = new Passerelle(fichier);
        Runtime.getRuntime().addShutdownHook(new Thread(p::fermer));

        System.err.println("[passerelle] écriture dans " + fichier + ", pid " + ProcessHandle.current().pid());

        BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
        String ligne;
        while ((ligne = in.readLine()) != null) {
            Matcher m = LIGNE.matcher(ligne);
            if (!m.find()) continue;                         // lignes de statistiques ignorées
            long tMs = Long.parseLong(m.group(1));
            double t = Double.parseDouble(m.group(2));
            double moyenne = p.ajouter(t);
            p.enregistrer(tMs, t, moyenne);
            if (p.nbMesures % 10 == 0)
                System.err.printf("[passerelle] %d mesures, moyenne glissante %.2f °C, VmRSS %s%n",
                        p.nbMesures, moyenne, vmRss());
        }
        // fin de l'entrée (le nœud s'est arrêté) : le hook s'exécute à la sortie de la JVM
    }
}
