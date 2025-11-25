# Buccaneer Metrics Client

TypeScript client library for sending metrics to Buccaneer server.

## Installation

```bash
npm install @tensorworks/buccaneer-metrics-client
```

## Usage

### Basic Example

```typescript
import { MetricsCollection } from '@tensorworks/buccaneer-metrics-client';

// Create a metrics collection with an ID
const metrics = new MetricsCollection('my-app-instance');

// Add single-value metrics
metrics.pushStat('fps', 60);
metrics.pushStat('memory_used', 512.5, 'Memory Usage in MB');

// Add grouped metrics (e.g., per-player stats)
metrics.pushStatByGroup('player0', 'score', 1000);
metrics.pushStatByGroup('player1', 'score', 1500);
metrics.pushStatByGroup('player0', 'ping', 25);
metrics.pushStatByGroup('player1', 'ping', 30);

// Send to Buccaneer server
await metrics.send('http://localhost:8000');
```

### With Metadata

```typescript
const metrics = new MetricsCollection('my-app', {
    version: '1.0.0',
    environment: 'production'
});

// Add more metadata later
metrics.setMetadata('region', 'us-east-1');
```

### Manual JSON Export

```typescript
const payload = metrics.toJSON();
console.log(JSON.stringify(payload, null, 2));
```

Output format:
```json
{
  "id": "my-app",
  "metadata": {
    "version": "1.0.0",
    "environment": "production"
  },
  "metrics": {
    "fps": {
      "description": "fps",
      "value": 60
    },
    "score": {
      "description": "score",
      "value": [
        { "player0": 1000 },
        { "player1": 1500 }
      ]
    }
  },
  "timestamp": 1234567890
}
```

## API Reference

### `MetricsCollection`

#### Constructor

```typescript
new MetricsCollection(id: string, metadata?: Record<string, string>)
```

Creates a new metrics collection with the specified ID and optional metadata.

#### Methods

##### `pushStat(statName: string, statValue: number, description?: string): void`

Add or update a single-value metric.

- `statName` - The name of the metric (will be formatted for Prometheus compatibility)
- `statValue` - The numeric value
- `description` - Optional description (defaults to stat name)

##### `pushStatByGroup(groupName: string, statName: string, statValue: number, description?: string): void`

Add or update a grouped metric (e.g., per-player stats).

- `groupName` - The group identifier (e.g., "player0", "player1")
- `statName` - The name of the metric (will be formatted for Prometheus compatibility)
- `statValue` - The numeric value
- `description` - Optional description (defaults to stat name)

##### `setMetadata(key: string, value: string): void`

Add or update metadata for this metrics collection.

##### `getId(): string`

Get the ID for this metrics collection.

##### `getMetadata(): Readonly<Record<string, string>>`

Get the metadata for this metrics collection.

##### `toJSON(): MetricsPayload`

Convert to JSON payload format expected by Buccaneer server.

##### `clear(): void`

Clear all metrics (keeps metadata).

##### `send(serverUrl: string): Promise<void>`

Send metrics to Buccaneer server.

- `serverUrl` - The URL of the Buccaneer server (e.g., "http://localhost:8000")

### `formatMetricName(name: string): string`

Helper function to format metric names to be Prometheus-compatible. Valid Prometheus names must be alphanumeric or underscores.

- Replaces `-`, `.`, ` `, `/`, `\` with `_`
- Removes `(`, `)`, `"`, `'`, `<`, `>`

## Metric Name Formatting

Metric names are automatically formatted to be Prometheus-compatible:

```typescript
formatMetricName('Frame Time (ms)');  // Returns: 'Frame_Time_ms'
formatMetricName('player.health');     // Returns: 'player_health'
formatMetricName('network/latency');   // Returns: 'network_latency'
```

## Examples

For complete working examples, see the [examples](./examples/) directory.

To run the examples:

```bash
# Build the library first
npm run build

# Go to examples directory
cd examples
npm install

# Run the basic example
npm run example:basic
```

## Testing

```bash
# Run unit tests
npm test

# Run tests in watch mode
npm run test:watch

# Run tests with coverage
npm run test:coverage

# Build the project
npm run build

# Clean build artifacts
npm run clean
```

## License

MIT
