import com.google.gson.Gson;

public class Mesure {
    double t, h, p;          // lus depuis le JSON
    long ts;                 // heure d'envoi par le nœud (ms)
    transient String node;   // déduit du topic, pas dans le JSON
    transient long recu;     // heure de réception par la passerelle (ms)

    private static final Gson GSON = new Gson();

    static Mesure fromJson(String node, String json) {
        Mesure m = GSON.fromJson(json, Mesure.class);
        m.node = node;
        m.recu = System.currentTimeMillis();   // pris dès le callback
        return m;
    }
}
