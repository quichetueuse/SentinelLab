import org.eclipse.paho.client.mqttv3.*;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;

import java.nio.charset.StandardCharsets;
import java.sql.*;
import java.util.*;
import java.util.concurrent.*;

public class Passerelle {
    static final double SEUIL = 30.0;
    static final BlockingQueue<Mesure> file = new LinkedBlockingQueue<>();
    static final List<Long> latences = new ArrayList<>();

    public static void main(String[] args) throws Exception {
        // --- SQLite ---
        Connection db = DriverManager.getConnection("jdbc:sqlite:mesures.db");
        db.createStatement().execute(
            "CREATE TABLE IF NOT EXISTS mesures(" +
            "id INTEGER PRIMARY KEY, node TEXT, t REAL, h REAL, p REAL, ts INTEGER)");
        PreparedStatement ins = db.prepareStatement(
            "INSERT INTO mesures(node,t,h,p,ts) VALUES(?,?,?,?,?)");

        // --- MQTT ---
        var client = new MqttClient("tcp://localhost:1883", "passerelle-b01",
                                    new MemoryPersistence());
        var opts = new MqttConnectOptions();
        opts.setCleanSession(false);          // garde les messages QoS 1 en absence
        opts.setAutomaticReconnect(true);
	client.setCallback(new MqttCallback() {
    		public void connectionLost(Throwable c) { System.err.println("CONNEXION PERDUE : " + c); }
    		public void messageArrived(String t, MqttMessage m) {}
    		public void deliveryComplete(IMqttDeliveryToken t) {}
	});
        client.connect(opts);

        client.subscribe("eidl/b01/node/+/mesures", 1, (topic, msg) -> {
            // callback court : on met en file, on ne publie pas d'ici
	    System.out.println("RECU " + topic); 
            String node = topic.split("/")[3];
            String json = new String(msg.getPayload(), StandardCharsets.UTF_8);
            try {
                file.offer(Mesure.fromJson(node, json));
            } catch (Exception e) {
                System.err.println("JSON invalide : " + json);
            }
        });
        client.subscribe("eidl/b01/node/+/etat", 1, (t, m) ->
            System.out.println("ETAT " + t + " = "
                + new String(m.getPayload(), StandardCharsets.UTF_8)));

        System.out.println("Passerelle prête, en attente de mesures...");

        // --- Pipeline : thread principal = thread de travail ---
        int compteur = 0;
        while (true) {
            Mesure m = file.take();
            compteur++;

            // 1. stockage
            ins.setString(1, m.node);
            ins.setDouble(2, m.t);
            ins.setDouble(3, m.h);
            ins.setDouble(4, m.p);
            ins.setLong(5, m.ts);
            ins.executeUpdate();

            // 2. alerte au-delà du seuil -> commande LED
            String cmd = m.t > SEUIL ? "ON" : "OFF";
            MqttMessage out = new MqttMessage(cmd.getBytes(StandardCharsets.UTF_8));
            out.setQos(1);
            client.publish("eidl/b01/node/" + m.node + "/cmd/led", out);

            System.out.printf("MESURE #%d node=%s t=%.1f h=%.1f p=%.1f -> LED %s%n",
                compteur, m.node, m.t, m.h, m.p, cmd);

            // 3. latence
            latences.add(m.recu - m.ts);
            if (latences.size() == 100) {
                afficherLatences();
                latences.clear();
            }
        }
    }

    static void afficherLatences() {
        List<Long> s = new ArrayList<>(latences);
        Collections.sort(s);
        double moy = s.stream().mapToLong(Long::longValue).average().orElse(0);
        long med = (s.get(49) + s.get(50)) / 2;
        long p95 = s.get(94);
        System.out.printf("LATENCE sur 100 msgs : min=%d ms, moy=%.1f ms, med=%d ms, p95=%d ms, max=%d ms%n",
            s.get(0), moy, med, p95, s.get(99));
    }
}
