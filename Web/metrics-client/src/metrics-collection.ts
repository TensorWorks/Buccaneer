/**
 * Helper function to format metric names to be Prometheus-compatible
 * Valid Prometheus names must be alphanumeric or underscores
 */
export function formatMetricName(name: string): string {
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
 * Represents a single metric with description and value
 */
interface SingleMetric {
    description: string;
    value: number;
}

/**
 * Represents a grouped metric with description and array of values per group
 */
interface GroupedMetric {
    description: string;
    value: Array<Record<string, number>>;
}

/**
 * The metrics payload structure that matches the Buccaneer server format
 */
export interface MetricsPayload {
    id: string;
    metadata?: Record<string, string>;
    metrics: Record<string, SingleMetric | GroupedMetric>;
    timestamp: number;
}

/**
 * MetricsCollection class for managing and sending metrics to Buccaneer server
 * Structured to produce JSON in the exact format expected by the server
 */
export class MetricsCollection {
    private id: string;
    private metadata: Record<string, string> = {};
    private metrics: Record<string, SingleMetric | GroupedMetric> = {};

    constructor(id: string, metadata?: Record<string, string>) {
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
    storeStat(statName: string, statValue: number, description?: string): void {
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
    storeStatByGroup(groupName: string, statName: string, statValue: number, description?: string): void {
        const formattedName = formatMetricName(statName);
        const existing = this.metrics[formattedName];

        if (!existing || !('value' in existing && Array.isArray(existing.value))) {
            // Create new grouped metric
            this.metrics[formattedName] = {
                description: description || statName,
                value: [{ [groupName]: statValue }]
            };
        } else {
            // Update existing grouped metric
            const groupedMetric = existing as GroupedMetric;
            
            // Find if this group already exists
            const existingGroupIndex = groupedMetric.value.findIndex(v => groupName in v);
            
            if (existingGroupIndex >= 0) {
                // Update existing group value
                groupedMetric.value[existingGroupIndex][groupName] = statValue;
            } else {
                // Add new group
                groupedMetric.value.push({ [groupName]: statValue });
            }
        }
    }

    /**
     * Get the ID for this metrics collection
     */
    getId(): string {
        return this.id;
    }

    /**
     * Get the metadata for this metrics collection
     */
    getMetadata(): Readonly<Record<string, string>> {
        return this.metadata;
    }

    /**
     * Add or update metadata
     */
    setMetadata(key: string, value: string): void {
        this.metadata[key] = value;
    }

    /**
     * Convert to JSON payload format expected by Buccaneer server
     */
    toJSON(): MetricsPayload {
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
    clear(): void {
        this.metrics = {};
    }

    /**
     * Send metrics to Buccaneer server
     * @param serverUrl - The URL of the Buccaneer server (e.g., "http://localhost:8000")
     * @returns Promise that resolves when metrics are sent
     */
    async send(serverUrl: string): Promise<void> {
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
