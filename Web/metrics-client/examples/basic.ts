/**
 * Example usage of Buccaneer Metrics Client
 * 
 * To run this example:
 * 1. Make sure Buccaneer server is running (e.g., docker compose up in Examples/Compose)
 * 2. Build the library: cd .. && npm run build
 * 3. Install dependencies: npm install
 * 4. Run this example: npm run example:basic
 */

import { MetricsCollection } from '@tensorworks/buccaneer-metrics-client';

async function main() {
    // Create a metrics collection with an ID
    const metrics = new MetricsCollection('example-client', {
        version: '1.0.0',
        environment: 'development'
    });

    console.log('Creating sample metrics...');

    // Add some single-value metrics
    metrics.pushStat('fps', 60, 'Frames Per Second');
    metrics.pushStat('frame_time', 16.67, 'Frame Time in milliseconds');
    metrics.pushStat('memory_used', 512.5, 'Memory Usage in MB');
    metrics.pushStat('cpu_usage', 45.3, 'CPU Usage Percentage');

    // Add grouped metrics (simulating multiple players)
    const players = ['player0', 'player1', 'player2'];
    players.forEach((player, index) => {
        metrics.pushStatByGroup(player, 'score', (index + 1) * 1000, 'Player Score');
        metrics.pushStatByGroup(player, 'ping', 20 + index * 10, 'Player Ping in ms');
        metrics.pushStatByGroup(player, 'health', 100 - index * 10, 'Player Health');
    });

    // Display the JSON payload
    console.log('\nGenerated JSON payload:');
    console.log(JSON.stringify(metrics.toJSON(), null, 2));

    // Send to server (uncomment if server is running)
    try {
        console.log('\nSending metrics to http://localhost:8000...');
        await metrics.send('http://localhost:8000');
        console.log('✓ Metrics sent successfully!');
    } catch (error) {
        console.error('✗ Failed to send metrics:', error instanceof Error ? error.message : error);
        console.log('\nMake sure Buccaneer server is running on http://localhost:8000');
        console.log('You can start it with: docker compose up -d (from Examples/Compose directory)');
    }

    // Demonstrate clearing metrics
    console.log('\nClearing metrics...');
    metrics.clear();
    console.log('Metrics after clear:', metrics.toJSON().metrics);
}

main().catch(console.error);
