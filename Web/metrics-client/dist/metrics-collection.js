/**
 * Helper function to format metric names to be Prometheus-compatible
 * Valid Prometheus names must be alphanumeric or underscores
 */
export function formatMetricName(name) {
    return name
        .replace(/-/g, '_')
        .replace(/\./g, '_')
        .replace(/ /g, '_')
        .replace(/\//g, '_')
        .replace(/\\/g, '_')
        .replace(/\(/g, '')
        .replace(/\)/g, '')
        .replace(/"/g, '')
        .replace(/'/g, '')
        .replace(/>/g, '')
        .replace(/</g, '');
}
/**
 * MetricsCollection class for managing and sending metrics to Buccaneer server
 * Structured to produce JSON in the exact format expected by the server
 */
export class MetricsCollection {
    id;
    metadata = {};
    metrics = {};
    constructor(id, metadata) {
        this.id = id;
        if (metadata) {
            this.metadata = { ...metadata };
        }
    }
    /**
     * Add or update a single-value metric
     * @param statName - The name of the metric
     * @param statValue - The numeric value
     * @param description - Optional description (defaults to stat name)
     */
    pushStat(statName, statValue, description) {
        const formattedName = formatMetricName(statName);
        this.metrics[formattedName] = {
            description: description || statName,
            value: statValue
        };
    }
    /**
     * Add or update a grouped metric (e.g., per-player stats)
     * @param groupName - The group identifier (e.g., "player0", "player1")
     * @param statName - The name of the metric
     * @param statValue - The numeric value
     * @param description - Optional description (defaults to stat name)
     */
    pushStatByGroup(groupName, statName, statValue, description) {
        const formattedName = formatMetricName(statName);
        const existing = this.metrics[formattedName];
        if (!existing || !('value' in existing && Array.isArray(existing.value))) {
            // Create new grouped metric
            this.metrics[formattedName] = {
                description: description || statName,
                value: [{ [groupName]: statValue }]
            };
        }
        else {
            // Update existing grouped metric
            const groupedMetric = existing;
            // Find if this group already exists
            const existingGroupIndex = groupedMetric.value.findIndex(v => groupName in v);
            if (existingGroupIndex >= 0) {
                // Update existing group value
                groupedMetric.value[existingGroupIndex][groupName] = statValue;
            }
            else {
                // Add new group
                groupedMetric.value.push({ [groupName]: statValue });
            }
        }
    }
    /**
     * Get the ID for this metrics collection
     */
    getId() {
        return this.id;
    }
    /**
     * Get the metadata for this metrics collection
     */
    getMetadata() {
        return this.metadata;
    }
    /**
     * Add or update metadata
     */
    setMetadata(key, value) {
        this.metadata[key] = value;
    }
    /**
     * Convert to JSON payload format expected by Buccaneer server
     */
    toJSON() {
        return {
            id: this.id,
            metadata: Object.keys(this.metadata).length > 0 ? this.metadata : undefined,
            metrics: this.metrics,
            timestamp: Math.floor(Date.now() / 1000)
        };
    }
    /**
     * Clear all metrics (keeps metadata)
     */
    clear() {
        this.metrics = {};
    }
    /**
     * Send metrics to Buccaneer server
     * @param serverUrl - The URL of the Buccaneer server (e.g., "http://localhost:8000")
     * @returns Promise that resolves when metrics are sent
     */
    async send(serverUrl) {
        const payload = this.toJSON();
        const url = `${serverUrl}/stats`;
        const response = await fetch(url, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(payload)
        });
        if (!response.ok) {
            throw new Error(`Failed to send metrics: ${response.status} ${response.statusText}`);
        }
    }
}
