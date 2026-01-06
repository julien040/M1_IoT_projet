import { connect } from "mqtt";
import 'dotenv/config'
import { getTravelTime } from "./googleMaps.ts";
import { getBusTravelTime } from "./idelis.ts";
import { pushBusMetrics, pushCarMetrics } from "./grafana.ts";
import type { getBusTravelTimeResult,etapeBus } from "./idelis.ts";

const client = connect("mqtt://test.mosquitto.org");

const origin = "29 Avenue Honoré Baradat, 64000 Pau, France";
const destination = "Avenue de l'université, 64000 Pau, France";

client.on("connect", main);

let alreadyRunning = false;
let i = 0;

const loop = () => {
    console.log("Calcul des temps de trajet...");
    /* const tempsVoiture = getTravelTime(
        origin,
        destination,
        process.env.MAPS_API_KEY!
    ); */
    const bus: etapeBus = {
        tempsVersArretMinute: 4,
        ligne: "F",
        arretDepart: "F-SARA_2",
        tempsTrajetMinute: 5,
    };

    const topic1 = "UPPA_M1_IOT/harvest/travelTime/car";
    const topic2 = "UPPA_M1_IOT/harvest/travelTime/bus";
    const topic3 = "UPPA_M1_IOT/harvest/nextBusDepartures";

    const tempsBus: Promise<getBusTravelTimeResult> = getBusTravelTime(bus);

    // Rafraichissement complet toutes les 10 itérations (10 minutes)
    if (i % 20 === 0) {
        console.log("Rafraichissement complet des données voiture...");
        const tempsVoiture = getTravelTime(
            origin,
            destination,
            process.env.MAPS_API_KEY!
        );
        
        tempsVoiture.then((voiture) => {
            if (!voiture) {
                console.error("Erreur lors de la récupération du temps de trajet voiture");
                return;
            }
            const messageVoiture = `${
            voiture ? (voiture.dureeSeconde / 60).toFixed(0) : "N/A"
        } min ${voiture ? voiture.dureeSeconde % 60 : "N/A"} sec`;

            client.publish(topic1, messageVoiture);
            console.log(`Published to ${topic1}: ${messageVoiture}`);
            pushCarMetrics(voiture.dureeSeconde);
        });        
    }

    
    tempsBus.then((bus) => {
        /* console.log(`Temps voiture: ${voiture?.dureeSeconde} secondes`);
            if (bus.tempsTotalMinute === Infinity) {
                console.log("Aucun bus disponible dans le futur proche");
            } else {
                console.log(`Temps bus: ${bus.tempsTotalMinute} minutes`);
                console.log(`Prochains départs bus (minutes): ${bus.prochainsDeparts.join(", ")}`);
            } */
        const messageBus =
            bus.tempsTotalMinute === Infinity
                ? "N/A"
                : `${bus.tempsTotalMinute} min`;
        const messageDeparts =
            bus.prochainsDeparts.length > 0
                ? bus.prochainsDeparts.splice(0, 5).join("min, ") + "min"
                : "N/A";

        client.publish(topic2, messageBus);
        console.log(`Published to ${topic2}: ${messageBus}`);

        client.publish(topic3, messageDeparts);
        console.log(`Published to ${topic3}: ${messageDeparts}`);

        // Push to Grafana
        pushBusMetrics(bus.tempsTotalMinute * 60);
    });
    i++;
};

function main() {
    if (alreadyRunning) {
        return;
    }
    alreadyRunning = true;
    console.log("Connecté avec succès au broker MQTT");
    loop(); // Exécute immédiatement une première fois, puis toutes les 30 secondes
    setInterval(loop, 30000); // Raffraichit toutes les 30 secondes
}