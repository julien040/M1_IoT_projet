type TravelTimeResponse = {
  distanceMetres: number;
  dureeSeconde: number;
};

async function getTravelTime(
    origin: string,
    destination: string,
    apiKey: string
): Promise<TravelTimeResponse | null> {
    const url = "https://routes.googleapis.com/directions/v2:computeRoutes";

    const payload = {
        origin: { address: origin },
        destination: { address: destination },
        travelMode: "DRIVE",
        routingPreference: "TRAFFIC_AWARE", // Critical for real-time traffic
    };

    try {
        const response = await fetch(url, {
            method: "POST",
            headers: {
                "Content-Type": "application/json",
                "X-Goog-Api-Key": apiKey,
                // FieldMask reduces cost/bandwidth by asking ONLY for time & distance
                "X-Goog-FieldMask":
                    "routes.duration,routes.distanceMeters,routes.staticDuration",
            },
            body: JSON.stringify(payload),
        });

        const data = await response.json();

        if (data.routes && data.routes.length > 0) {
            const route = data.routes[0];
            // Note: Duration strings are returned like "3600s"
            console.log(`Distance: ${route.distanceMeters} meters`);
            console.log(`Duration (Traffic): ${route.duration}`);
            console.log(`Duration (Static): ${route.staticDuration}`);

            // Extract numeric value from duration string
            const durationSeconds = parseInt(
                route.duration.replace("s", ""),
                10
            );
            return {
                distanceMetres: route.distanceMeters,
                dureeSeconde: durationSeconds,
            }
        } else {
            console.error("No routes found");
            return null;
        }
    } catch (error) {
        console.error("API Error:", error);
        return null;
    }
}


export {getTravelTime};
