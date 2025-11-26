import { MetricsCollection } from '../../Web/metrics-client/src/index.js';

const BUCCANEER_URL = 'http://localhost:8000';
const PROMETHEUS_URL = 'http://localhost:9090';

/**
 * Helper function to query Prometheus
 */
async function queryPrometheus(query: string): Promise<any> {
  const url = `${PROMETHEUS_URL}/api/v1/query?query=${encodeURIComponent(query)}`;
  const response = await fetch(url);
  
  if (!response.ok) {
    throw new Error(`Prometheus query failed: ${response.statusText}`);
  }
  
  return response.json();
}

/**
 * Helper function to wait for metric to appear in Prometheus
 * Prometheus scrapes every 15 seconds by default, so we need to wait and retry
 */
async function waitForMetric(
  metricName: string, 
  maxAttempts = 20, 
  delayMs = 2000
): Promise<boolean> {
  console.log(`  Waiting for metric "${metricName}" to appear in Prometheus...`);
  
  for (let i = 0; i < maxAttempts; i++) {
    try {
      const result = await queryPrometheus(metricName);
      
      if (result.status === 'success' && result.data.result.length > 0) {
        console.log(`  ✓ Metric found after ${(i + 1) * delayMs / 1000}s`);
        return true;
      }
    } catch (error) {
      // Ignore errors and retry
    }
    
    if (i < maxAttempts - 1) {
      await new Promise(resolve => setTimeout(resolve, delayMs));
    }
  }
  
  console.log(`  ✗ Metric not found after ${maxAttempts * delayMs / 1000}s`);
  return false;
}

/**
 * Helper to get metric value from Prometheus
 */
async function getMetricValue(metricName: string, labels: Record<string, string> = {}): Promise<number | null> {
  let query = metricName;
  
  // Add labels to query if provided
  if (Object.keys(labels).length > 0) {
    const labelStr = Object.entries(labels)
      .map(([key, value]) => `${key}="${value}"`)
      .join(',');
    query = `${metricName}{${labelStr}}`;
  }
  
  const result = await queryPrometheus(query);
  
  if (result.status === 'success' && result.data.result.length > 0) {
    return parseFloat(result.data.result[0].value[1]);
  }
  
  return null;
}

describe('Buccaneer E2E: Metrics Pipeline', () => {
  // Wait for services to be ready before running tests
  beforeAll(async () => {
    console.log('\n=== Setting up E2E Test Environment ===\n');
    
    // Wait for Buccaneer server
    console.log('Checking Buccaneer server...');
    let buccaneerReady = false;
    for (let i = 0; i < 30; i++) {
      try {
        const response = await fetch(BUCCANEER_URL);
        if (response.ok) {
          buccaneerReady = true;
          console.log('✓ Buccaneer server is ready\n');
          break;
        }
      } catch (error) {
        await new Promise(resolve => setTimeout(resolve, 1000));
      }
    }
    
    if (!buccaneerReady) {
      throw new Error('Buccaneer server failed to start');
    }
    
    // Wait for Prometheus
    console.log('Checking Prometheus...');
    let prometheusReady = false;
    for (let i = 0; i < 30; i++) {
      try {
        const response = await fetch(`${PROMETHEUS_URL}/-/ready`);
        if (response.ok) {
          prometheusReady = true;
          console.log('✓ Prometheus is ready\n');
          break;
        }
      } catch (error) {
        await new Promise(resolve => setTimeout(resolve, 1000));
      }
    }
    
    if (!prometheusReady) {
      throw new Error('Prometheus failed to start');
    }
    
    console.log('=== Environment Ready ===\n');
  });

  test('should push single-value metrics and verify in Prometheus', async () => {
    console.log('\n--- Test: Single-value metrics ---\n');
    
    const metrics = new MetricsCollection('e2e-test-single', {
      testRun: Date.now().toString()
    });

    // Use timestamp to create unique metric names
    const timestamp = Date.now();
    const metricName = `test_single_metric_${timestamp}`;
    const metricValue = 42.5;
    
    metrics.pushStat(metricName, metricValue, 'E2E test single metric');

    // Send to Buccaneer server
    console.log(`Sending metric: ${metricName} = ${metricValue}`);
    await metrics.send(BUCCANEER_URL);
    console.log('✓ Metric sent to Buccaneer server\n');

    // Wait for Prometheus to scrape it
    const found = await waitForMetric(metricName);
    expect(found).toBe(true);

    // Verify the value
    const value = await getMetricValue(metricName);
    expect(value).toBe(metricValue);
    
    console.log(`✓ Verified in Prometheus: ${metricName} = ${value}\n`);
  });

  test('should push grouped metrics and verify in Prometheus', async () => {
    console.log('\n--- Test: Grouped metrics ---\n');
    
    const metrics = new MetricsCollection('e2e-test-grouped', {
      testRun: Date.now().toString()
    });

    const timestamp = Date.now();
    const metricName = `test_grouped_metric_${timestamp}`;
    
    // Push metrics for multiple groups (e.g., per-player stats)
    const groups = ['player_0', 'player_1', 'player_2'];
    const values = [100, 200, 300];
    
    console.log(`Sending grouped metric: ${metricName}`);
    groups.forEach((group, index) => {
      metrics.pushStatByGroup(group, metricName, values[index], 'E2E test grouped metric');
      console.log(`  ${group}: ${values[index]}`);
    });

    await metrics.send(BUCCANEER_URL);
    console.log('✓ Metrics sent to Buccaneer server\n');

    // Wait for metric to appear
    const found = await waitForMetric(metricName);
    expect(found).toBe(true);

    // Verify each group's value
    console.log('Verifying grouped values:');
    for (let i = 0; i < groups.length; i++) {
      const value = await getMetricValue(metricName, { player: groups[i] });
      expect(value).toBe(values[i]);
      console.log(`  ✓ ${metricName}{player="${groups[i]}"} = ${value}`);
    }
    console.log('');
  });

  test('should handle multiple metric batches from same instance', async () => {
    console.log('\n--- Test: Multiple batches ---\n');
    
    const instanceId = `e2e-multi-${Date.now()}`;
    const timestamp = Date.now();
    
    // Send first batch
    console.log('Sending batch 1...');
    const metrics1 = new MetricsCollection(instanceId, { batch: '1' });
    const metric1Name = `test_batch_1_${timestamp}`;
    metrics1.pushStat(metric1Name, 111, 'First batch');
    await metrics1.send(BUCCANEER_URL);
    console.log(`  ✓ Sent: ${metric1Name} = 111`);
    
    // Send second batch
    console.log('Sending batch 2...');
    const metrics2 = new MetricsCollection(instanceId, { batch: '2' });
    const metric2Name = `test_batch_2_${timestamp}`;
    metrics2.pushStat(metric2Name, 222, 'Second batch');
    await metrics2.send(BUCCANEER_URL);
    console.log(`  ✓ Sent: ${metric2Name} = 222\n`);

    // Wait for both to be scraped
    const found1 = await waitForMetric(metric1Name);
    const found2 = await waitForMetric(metric2Name);
    
    expect(found1).toBe(true);
    expect(found2).toBe(true);

    const value1 = await getMetricValue(metric1Name);
    const value2 = await getMetricValue(metric2Name);
    
    expect(value1).toBe(111);
    expect(value2).toBe(222);
    
    console.log('Verified both batches:');
    console.log(`  ✓ ${metric1Name} = ${value1}`);
    console.log(`  ✓ ${metric2Name} = ${value2}\n`);
  });

  test('should handle metrics with special characters in names', async () => {
    console.log('\n--- Test: Metric name formatting ---\n');
    
    const metrics = new MetricsCollection('e2e-test-formatting', {
      testRun: Date.now().toString()
    });

    const timestamp = Date.now();
    
    // Test various special characters that should be formatted
    const originalName = `Frame Time (ms) - ${timestamp}`;
    const expectedFormattedName = `Frame_Time_ms___${timestamp}`;
    
    metrics.pushStat(originalName, 16.67, 'Frame time metric');
    
    console.log(`Original name: "${originalName}"`);
    console.log(`Expected formatted: "${expectedFormattedName}"`);
    
    await metrics.send(BUCCANEER_URL);
    console.log('✓ Metric sent to Buccaneer server\n');
    
    // Wait for the formatted metric name
    const found = await waitForMetric(expectedFormattedName);
    expect(found).toBe(true);
    
    const value = await getMetricValue(expectedFormattedName);
    expect(value).toBe(16.67);
    
    console.log(`✓ Verified formatted metric: ${expectedFormattedName} = ${value}\n`);
  });
});
