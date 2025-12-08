import got from 'got';
type etapeBus = {
    // Temps pour se rendre à l'arrêt en minute
    tempsVersArretMinute: number;
    // Numéro de la ligne de bus
    ligne: string;
    // Code de l'arrêt de départ
    arretDepart: string;
    // Temps de trajet en bus en minute depuis l'arrêt jusqu'à la destination
    tempsTrajetMinute: number;
}


type getBusTravelTimeResult = {
    // Temps total en minute (attente + trajet)
    tempsTotalMinute: number;
    // Les 5 prochains départs en minute
    prochainsDeparts: number[];
}

function parseDuree(arrivee: string): number {
    /*
    "6 min" => 6
    "11:32" => calculer la différence entre l'heure actuelle et 11:32
    "Imminent" => 0
    */
    if (arrivee === 'Imminent') {
        return 0;
    } else if (arrivee.endsWith(' min')) {
        return parseInt(arrivee.split(' ')[0], 10);
    } else if (arrivee.includes(':')) {
        const now = new Date();
        const [hours, minutes] = arrivee.split(':').map(Number);
        const arriveeDate = new Date(now.getFullYear(), now.getMonth(), now.getDate(), hours, minutes);
        if (arriveeDate < now) {
            arriveeDate.setDate(arriveeDate.getDate() + 1);
        }
        const diffMs = arriveeDate.getTime() - now.getTime();
        return Math.round(diffMs / 60000);
    }
    return Infinity; // unknown format
}

/**
 * Retourne le temps de trajet en bus depuis un arrêt donné. Prend en compte le temps d'attente, et le temps pour aller à l'arrêt.
 * @param etape Le bus à prendre
 * @returns Le temps de trajet en minute
 */
async function getBusTravelTime(
    etape: etapeBus
): Promise<getBusTravelTimeResult> {
    const data: any = await got
        .get("https://api.idelis.fr/GetStopMonitoring", {
            method: "GET",
            headers: {
                "X-AUTH-TOKEN": process.env.IDELIS_API_KEY,
                "Content-Type": "application/json",
            },
            json: {
                next: 10,
                code: etape.arretDepart,
            },
            allowGetBody: true,
        })
        .json();
    /*
    {
  "{SIRI_IDELIS:Line::F:LOC_SIRI_IDELIS:StopPoint:BP:PGDPQE_E:LOC": {
    "ligne": "F",
    "destination": "PAU GARE DE PAU",
    "pmr": true,
    "passages": [
      {
        "arrivee": "6 min",
        "type": "reel",
        "premier": false,
        "dernier": false
      },
      {
        "arrivee": "11:32",
        "type": "theorique",
        "premier": false,
        "dernier": false
      },
      ...
    ]
    },
    ...
    }
    */

    const prochainsDeparts: number[] = [];
    let tempsTotalMinute: number | null = null;

    for (const key in data) {
        const busInfo = data[key];
        if (busInfo.ligne === etape.ligne) {
            for (const passage of busInfo.passages) {
                const tempsAttente = parseDuree(passage.arrivee);
                if (tempsAttente === Infinity) {
                    continue;
                }
                if (prochainsDeparts.length < 5) {
                    prochainsDeparts.push(tempsAttente);
                }
                // Si le bus passe avant le temps pour aller à l'arrêt, on l'ignore
                if (tempsAttente >= etape.tempsVersArretMinute && tempsTotalMinute === null) {
                    tempsTotalMinute = tempsAttente + etape.tempsTrajetMinute;
                    
                }
            }
        }
    }

    return {
        tempsTotalMinute: tempsTotalMinute || Infinity,
        prochainsDeparts,
    }
}

export {getBusTravelTime};
export type {etapeBus, getBusTravelTimeResult};