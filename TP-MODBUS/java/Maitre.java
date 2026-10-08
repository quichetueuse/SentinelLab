// Maitre.java : maître Modbus RTU en Java 21. Lance l'esclave C++ et lui parle par
// un tube, comme sur une liaison série. Commandes tapées en console (ou en tube).
//   java java/Maitre.java ./build/variateur [--bruit 0.2]
//   java java/Maitre.java --tests
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.HexFormat;
import java.util.List;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.TimeUnit;

public class Maitre {
    static final HexFormat HEX = HexFormat.ofDelimiter(" ").withUpperCase();
    static final int LIRE = 0x03, ECRIRE = 0x06;
    static final String[] NOMS = {"consigne", "vitesse", "courant", "température", "état", "commande"};

    // ------------------------------------------------------------ CRC (J1)
    static int crc16(byte[] d, int n) {
        for (int i = 0; i < n; i++) {
            crc ^= d[i] & 0xFF;
            for (int bit = 0; bit < 8; bit++) {
                crc = (crc & 1) != 0 ? (crc >>> 1) ^ 0xA001 : crc >>> 1;
            }
        }
        return crc;
    }

    // ------------------------------------------------- construction (J2)
    static byte[] requete(int esclave, int fonction, int adresse, int valeur) {
        ByteBuffer b = ByteBuffer.allocate(8);
        b.put((byte) esclave).put((byte) fonction).putShort((short) adresse).putShort((short) valeur);
        int crc = crc16(b.array(), 6);
        b.put((byte) crc).put((byte) (crc >>> 8));
        return b.array();
    }

    // ------------------------------------------- réponses : type scellé (J3)
    sealed interface Reponse permits Lecture, Ecriture, Refus, Corrompue {}
    record Lecture(int adresse, int[] valeurs) implements Reponse {}
    record Ecriture(int registre, int valeur) implements Reponse {}
    record Refus(int fonction, int code) implements Reponse {}
    record Corrompue(String raison) implements Reponse {}

    static Reponse decoder(byte[] t, int adresseDemandee) {
        if (t.length < 5) return new Corrompue("trame trop courte");
        if (crc16(t, t.length) != 0) return new Corrompue("CRC invalide");
        int fonction = t[1] & 0xFF;
        if ((fonction & 0x80) != 0)
            return new Refus(fonction & 0x7F, t[2] & 0xFF);
        switch (fonction) {
            case LIRE -> {
                int octets = t[2] & 0xFF;
                if (octets % 2 != 0 || t.length != 3 + octets + 2)
                    return new Corrompue("longueur incohérente");
                int[] valeurs = new int[octets / 2];
                for (int i = 0; i < valeurs.length; i++)
                    valeurs[i] = ((t[3 + 2 * i] & 0xFF) << 8) | (t[4 + 2 * i] & 0xFF);
                return new Lecture(adresseDemandee, valeurs);
            }
            case ECRIRE -> {
                if (t.length != 8) return new Corrompue("longueur incohérente");
                return new Ecriture(((t[2] & 0xFF) << 8) | (t[3] & 0xFF),
                                    ((t[4] & 0xFF) << 8) | (t[5] & 0xFF));
            }
            default -> { return new Corrompue(String.format("fonction 0x%02X inattendue", fonction)); }
        }
    }

    static String nomException(int code) {
        return switch (code) {
            case 1 -> "fonction illégale";
            case 2 -> "adresse illégale";
            case 3 -> "valeur illégale";
            default -> "code " + code;
        };
    }

    static String decrireEtat(int e) {
        List<String> s = new ArrayList<>();
        if ((e & 1) != 0) s.add("MARCHE"); else s.add("ARRÊT");
        if ((e & 2) != 0) s.add("DÉFAUT");
        if ((e & 4) != 0) s.add("SURCHAUFFE");
        if ((e & 8) != 0) s.add("À LA CONSIGNE");
        return String.join(" | ", s);
    }

    // ------------------------------------------------------ affichage (J4)
    static void afficher(Reponse rep) {
        switch (rep) {
            case Lecture(int adr, int[] v) when adr == 0 && v.length == 6 -> {
                System.out.printf("  consigne     %5d tr/min%n", v[0]);
                System.out.printf("  vitesse      %5d tr/min%n", v[1]);
                System.out.printf("  courant      %5.1f A%n", v[2] / 10.0);
                System.out.printf("  température  %5.1f °C%n", v[3] / 10.0);
                System.out.printf("  état         %s%n", decrireEtat(v[4]));
                System.out.printf("  commande     %5d%n", v[5]);
            }
            case Lecture(int adr, int[] v) -> {
                for (int i = 0; i < v.length; i++)
                    System.out.printf("  registre %d = %d%n", adr + i, v[i]);
            }
            case Ecriture(int reg, int val) -> {
                String nom = reg >= 0 && reg < NOMS.length ? NOMS[reg] : "registre " + reg;
                System.out.printf("  %s <- %d : accepté%n", nom, val);
            }
            case Refus(int f, int code) ->
                System.out.printf("  refus de l'esclave (fonction 0x%02X) : %s%n", f, nomException(code));
            case Corrompue(String raison) ->
                System.out.println("  réponse inexploitable : " + raison);
        }

        System.out.println(rep);
    }

    // ----------------------------------------------------------- transport
    private final OutputStream versEsclave;
    private final BlockingQueue<Integer> octetsRecus = new LinkedBlockingQueue<>();
    private int esclave = 0x11;
    private int transactions, reprises, silences;

    Maitre(Process p) {
        versEsclave = p.getOutputStream();
        InputStream depuisEsclave = p.getInputStream();
        // Un thread virtuel lit en continu : la lecture avec délai se fait sur la file.
        Thread.ofVirtual().name("lecteur-rs485").start(() -> {
            try {
                int o;
                while ((o = depuisEsclave.read()) >= 0) octetsRecus.add(o);
            } catch (IOException e) { /* esclave arrêté */ }
        });
    }

    private Integer octet(long delaiMs) throws InterruptedException {
        return octetsRecus.poll(delaiMs, TimeUnit.MILLISECONDS);
    }

    // Lit une réponse dont la longueur se déduit de l'en-tête. null si silence.
    private byte[] lireReponse() throws InterruptedException {
        Integer a = octet(500), f = octet(50);
        if (a == null || f == null) return null;
        int reste;
        if ((f & 0x80) != 0) reste = 3;                     // code + CRC
        else if (f == LIRE) {
            Integer n = octet(50);
            if (n == null) return null;
            byte[] t = new byte[3 + n + 2];
            t[0] = a.byteValue(); t[1] = f.byteValue(); t[2] = n.byteValue();
            for (int i = 3; i < t.length; i++) { Integer o = octet(50); if (o == null) return null; t[i] = o.byteValue(); }
            return t;
        } else reste = 6;                                    // écho 0x06 : 4 octets + CRC
        byte[] t = new byte[2 + reste];
        t[0] = a.byteValue(); t[1] = f.byteValue();
        for (int i = 2; i < t.length; i++) { Integer o = octet(50); if (o == null) return null; t[i] = o.byteValue(); }
        return t;
    }

    Reponse transaction(int fonction, int adresse, int valeur) throws IOException, InterruptedException {
        transactions++;
        byte[] req = requete(esclave, fonction, adresse, valeur);
        for (int essai = 1; essai <= 3; essai++) {
            versEsclave.write(req);
            versEsclave.flush();
            byte[] rep = lireReponse();
            if (rep == null) {
                silences++;
                System.out.printf("  [essai %d] pas de réponse de l'esclave 0x%02X%n", essai, esclave);
                continue;
            }
            Reponse r = decoder(rep, adresse);
            if (r instanceof Corrompue(String raison)) {
                reprises++;
                System.out.printf("  [essai %d] %s : %s%n", essai, raison, HEX.formatHex(rep));
                continue;
            }
            return r;
        }
        return new Corrompue("échec après 3 essais");
    }

    // ---------------------------------------------------------- console
    void executer(String ligne) throws IOException, InterruptedException {
        String[] m = ligne.trim().split("\\s+");
        switch (m[0]) {
            case "etat" -> afficher(transaction(LIRE, 0, 6));
            case "lire" -> afficher(transaction(LIRE, Integer.parseInt(m[1]), Integer.parseInt(m[2])));
            case "ecrire" -> afficher(transaction(ECRIRE, Integer.parseInt(m[1]), Integer.parseInt(m[2])));
            case "consigne" -> afficher(transaction(ECRIRE, 0, Integer.parseInt(m[1])));
            case "marche" -> afficher(transaction(ECRIRE, 5, 1));
            case "arret" -> afficher(transaction(ECRIRE, 5, 0));
            case "acquitter" -> afficher(transaction(ECRIRE, 5, 2));
            case "esclave" -> { esclave = Integer.decode(m[1]); System.out.printf("  cible : esclave 0x%02X%n", esclave); }
            case "surveiller" -> {
                int n = m.length > 1 ? Integer.parseInt(m[1]) : 10;
                for (int i = 0; i < n; i++) {
                    if (transaction(LIRE, 0, 6) instanceof Lecture(int adr, int[] v))
                        System.out.printf("  t+%4.1f s  vitesse %4d tr/min  courant %4.1f A  %5.1f °C  %s%n",
                                i * 0.5, v[1], v[2] / 10.0, v[3] / 10.0, decrireEtat(v[4]));
                    Thread.sleep(500);
                }
            }
            case "stats" -> System.out.printf("  transactions %d, reprises sur CRC %d, silences %d%n",
                    transactions, reprises, silences);
            case "" -> { }
            default -> System.out.println("  commandes : etat, lire A N, ecrire R V, consigne V, marche, arret,"
                    + " acquitter, surveiller N, esclave A, stats, quitter");
        }
    }

    // ----------------------------------------------------------- tests
    static void tests() {
        int[] ok = {0}, total = {0};
        java.util.function.BiConsumer<Boolean, String> verifier = (c, nom) -> {
            total[0]++;
            if (c) ok[0]++; else System.out.println("ÉCHEC : " + nom);
        };
        verifier.accept(crc16(HEX.parseHex("01 03 00 00 00 0A"), 6) == 0xCDC5, "CRC exemple 1");
        verifier.accept(crc16(HEX.parseHex("11 03 00 6B 00 03"), 6) == 0x8776, "CRC exemple 2");
        verifier.accept(HEX.formatHex(requete(0x11, 3, 0x6B, 3)).equals("11 03 00 6B 00 03 76 87"), "requête");
        byte[] lecture = HEX.parseHex("11 03 04 05 DC 00 FA 00 00");
        int crc = crc16(lecture, 7);
        lecture[7] = (byte) crc; lecture[8] = (byte) (crc >>> 8);
        verifier.accept(decoder(lecture, 0) instanceof Lecture(int a, int[] v) && v[0] == 1500 && v[1] == 250, "lecture");
        lecture[4] ^= 1;
        verifier.accept(decoder(lecture, 0) instanceof Corrompue, "CRC faux détecté");
        byte[] refus = HEX.parseHex("11 86 03 00 00");
        crc = crc16(refus, 3);
        refus[3] = (byte) crc; refus[4] = (byte) (crc >>> 8);
        verifier.accept(decoder(refus, 0) instanceof Refus(int f, int code) && f == 6 && code == 3, "exception");
        System.out.printf("%d/%d tests réussis%n", ok[0], total[0]);
        System.exit(ok[0] == total[0] ? 0 : 1);
    }

    public static void main(String[] args) throws Exception {
        if (args.length > 0 && args[0].equals("--tests")) { tests(); return; }
        if (args.length == 0) {
            System.err.println("usage : java Maitre.java ./build/variateur [options de l'esclave] | --tests");
            System.exit(2);
        }
        Process p = new ProcessBuilder(args).redirectError(ProcessBuilder.Redirect.INHERIT).start();
        Maitre maitre = new Maitre(p);
        BufferedReader console = new BufferedReader(new InputStreamReader(System.in));
        System.out.println("maître Modbus prêt. Tapez une commande (aide : ?), quitter pour sortir.");
        String ligne;
        while ((ligne = console.readLine()) != null && !ligne.trim().equals("quitter")) {
            System.out.println("> " + ligne.trim());
            maitre.executer(ligne);
        }
        p.getOutputStream().close();           // l'esclave lit la fin du tube et s'arrête
        p.waitFor(2, TimeUnit.SECONDS);
        maitre.executerStatsFinales();
    }

    void executerStatsFinales() throws IOException, InterruptedException { executer("stats"); }
}
