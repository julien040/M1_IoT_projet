// Node.js example using native fetch (Node 18+)
const pushMetrics = async (
    carTravelTime: number | undefined,
    busTravelTime: number | undefined
) => {
    // 1. CONFIGURATION
    const userId = process.env.GRAFANA_USER_ID!;
    const apiKey = process.env.GRAFANA_API_KEY!;
    const baseUrl = process.env.GRAFANA_BASE_URL!;

    const pushUrl = `${baseUrl}/api/v1/push/influx/write`;

    let metricData = "";
    if (carTravelTime) {
        metricData += `travel_time,mode=car value=${carTravelTime}`;
    }
    if (busTravelTime) {
        metricData += `\ntravel_time,mode=bus value=${busTravelTime}`;
    }
    metricData = metricData.trim();

    // 3. SEND REQUEST
    try {
        const response = await fetch(pushUrl, {
            method: "POST",
            headers: {
                Authorization: `Basic ${btoa(`${userId}:${apiKey}`)}`, // Basic Auth
                "Content-Type": "text/plain",
            },
            body: metricData,
        });

        if (!response.ok) {
            const text = await response.text();
            throw new Error(`Error ${response.status}: ${text}`);
        }

        console.log("Metrics pushed successfully!");
    } catch (error) {
        console.error("Failed to push metrics:", error);
    }
};

export { pushMetrics };
