/**
 * Helper function to format metric names to be Prometheus-compatible
 * Valid Prometheus names must be alphanumeric or underscores
 */
export declare function formatMetricName(name: string): string;
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
export declare class MetricsCollection {
    private id;
    private metadata;
    private metrics;
    constructor(id: string, metadata?: Record<string, string>);
    /**
     * Add or update a single-value metric
     * @param statName - The name of the metric
     * @param statValue - The numeric value
     * @param description - Optional description (defaults to stat name)
     */
    storeStat(statName: string, statValue: number, description?: string): void;
    /**
     * Add or update a grouped metric (e.g., per-player stats)
     * @param groupName - The group identifier (e.g., "player0", "player1")
     * @param statName - The name of the metric
     * @param statValue - The numeric value
     * @param description - Optional description (defaults to stat name)
     */
    storeStatByGroup(groupName: string, statName: string, statValue: number, description?: string): void;
    /**
     * Get the ID for this metrics collection
     */
    getId(): string;
    /**
     * Get the metadata for this metrics collection
     */
    getMetadata(): Readonly<Record<string, string>>;
    /**
     * Add or update metadata
     */
    setMetadata(key: string, value: string): void;
    /**
     * Convert to JSON payload format expected by Buccaneer server
     */
    toJSON(): MetricsPayload;
    /**
     * Clear all metrics (keeps metadata)
     */
    clear(): void;
    /**
     * Send metrics to Buccaneer server
     * @param serverUrl - The URL of the Buccaneer server (e.g., "http://localhost:8000")
     * @returns Promise that resolves when metrics are sent
     */
    send(serverUrl: string): Promise<void>;
}
export {};
//# sourceMappingURL=metrics-collection.d.ts.map