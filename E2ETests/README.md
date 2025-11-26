# Buccaneer E2E Integration Tests

End-to-end integration tests that verify the complete metrics pipeline:

**TypeScript Client → Buccaneer Server → Prometheus**

## Overview

These tests push metrics using the Buccaneer TypeScript client, wait for Prometheus to scrape them, then query the Prometheus API to verify the metrics were stored correctly.

## Prerequisites

- Docker and Docker Compose installed
- Node.js 20+ installed
- Ports available: 8000 (Buccaneer), 9090 (Prometheus), 3000 (Grafana), 3100 (Loki), 9080 (Promtail)

## Quick Start

```bash
# Install dependencies
npm install

# Run E2E tests (automatically starts/stops Docker services)
npm test
```

## Manual Docker Management

```bash
# Start services manually
npm run docker:up

# Run tests without automatic Docker lifecycle
jest

# View service logs
npm run docker:logs

# Stop services manually
npm run docker:down
```

## How It Works

### Test Flow

1. **Setup Phase** (`beforeAll`):
   - Waits for Buccaneer server to be ready (up to 30 seconds)
   - Waits for Prometheus to be ready (up to 30 seconds)

2. **Test Execution**:
   - Creates `MetricsCollection` with test data
   - Sends metrics to Buccaneer server via HTTP POST
   - Waits for Prometheus to scrape metrics (polls every 2 seconds, up to 40 seconds)
   - Queries Prometheus API to retrieve metric values
   - Validates metrics match expected values

3. **Cleanup**:
   - Docker Compose stops and removes all containers
   - Volumes are cleaned up

### Prometheus Scraping

- **Scrape Interval**: 15 seconds (configured in `prometheus.yml`)
- **Test Wait Logic**: Polls Prometheus every 2 seconds, max 20 attempts (40 seconds total)
- **Why the wait?**: Metrics must go through: Client → Buccaneer → exposed /metrics endpoint → Prometheus scrape

## Test Coverage

The test suite covers:

### 1. Single-Value Metrics
Tests basic metric push and retrieval:
```typescript
metrics.pushStat('fps', 60);
// Verifies: fps = 60 in Prometheus
```

### 2. Grouped Metrics
Tests per-group metrics (e.g., per-player stats):
```typescript
metrics.pushStatByGroup('player_0', 'score', 100);
metrics.pushStatByGroup('player_1', 'score', 200);
// Verifies: score{group_id="player_0"} = 100
//           score{group_id="player_1"} = 200
```

### 3. Multiple Batches
Tests that multiple metric collections from the same instance work:
```typescript
// First batch
const metrics1 = new MetricsCollection('instance-1');
metrics1.pushStat('metric_a', 111);
await metrics1.send(BUCCANEER_URL);

// Second batch
const metrics2 = new MetricsCollection('instance-1');
metrics2.pushStat('metric_b', 222);
await metrics2.send(BUCCANEER_URL);
// Verifies: Both metrics exist in Prometheus
```

### 4. Metric Name Formatting
Tests that special characters are properly converted:
```typescript
metrics.pushStat('Frame Time (ms)', 16.67);
// Verifies: Frame_Time_ms_ = 16.67 in Prometheus
```

## Prometheus HTTP API

The tests use Prometheus's HTTP API for verification:

### Query a Metric
```bash
curl "http://localhost:9090/api/v1/query?query=test_metric_123456"
```

### Query with Labels
```bash
curl "http://localhost:9090/api/v1/query?query=score{group_id=\"player_0\"}"
```

### Response Format
```json
{
  "status": "success",
  "data": {
    "resultType": "vector",
    "result": [
      {
        "metric": {
          "__name__": "test_metric_123456",
          "instance": "buccaneerserver:8000",
          "job": "buccaneer"
        },
        "value": [1638360000, "42.5"]
      }
    ]
  }
}
```

## Troubleshooting

### Tests Timeout Waiting for Metrics

**Symptom**: Tests fail with "Metric not found after 40s"

**Solutions**:
1. Check if Prometheus is scraping:
   - Open http://localhost:9090/targets
   - Verify Buccaneer target is "UP"

2. Check scrape interval:
   - Default is 15 seconds in `prometheus.yml`
   - Increase `testTimeout` in `jest.config.js` if needed

3. Verify metrics endpoint:
   - Open http://localhost:8000/metrics
   - Should show Prometheus format metrics

### Docker Services Won't Start

**Symptom**: Services fail to start or tests timeout during setup

**Solutions**:
1. Check port conflicts:
   ```bash
   netstat -ano | findstr "8000 9090"
   ```

2. View container logs:
   ```bash
   npm run docker:logs
   ```

3. Manually verify services:
   ```bash
   docker compose -f ../Examples/Compose/docker-compose.yml ps
   ```

### Metrics Not Appearing in Prometheus

**Symptom**: Metrics sent but not found in Prometheus

**Solutions**:
1. Check Buccaneer server logs:
   ```bash
   docker compose -f ../Examples/Compose/docker-compose.yml logs buccaneerserver
   ```

2. Verify metric was received:
   - Check logs for POST to `/stats`
   - Should see metric registration

3. Check Prometheus targets:
   - http://localhost:9090/targets
   - Buccaneer should be listed and UP

4. Manually query Prometheus:
   - http://localhost:9090/graph
   - Try querying: `{job="buccaneer"}`

### Test Hanging or Never Completing

**Symptom**: Jest process doesn't exit

**Solutions**:
1. Kill Docker containers:
   ```bash
   npm run docker:down
   ```

2. Check for open handles:
   - Add `--detectOpenHandles` to jest command
   - Look for unclosed connections

## CI/CD Integration

The tests are designed to run in GitHub Actions. See `.github/workflows/e2e-tests.yml` for the workflow configuration.

Key points:
- Services start with `--wait` flag to ensure readiness
- Uses `timeout` commands for health checks
- Logs are captured on failure for debugging
- Cleanup runs even if tests fail (`if: always()`)

## Test Output

Successful test run shows:
```
=== Setting up E2E Test Environment ===

Checking Buccaneer server...
✓ Buccaneer server is ready

Checking Prometheus...
✓ Prometheus is ready

=== Environment Ready ===

--- Test: Single-value metrics ---

Sending metric: test_single_metric_1638360000 = 42.5
✓ Metric sent to Buccaneer server

  Waiting for metric "test_single_metric_1638360000" to appear in Prometheus...
  ✓ Metric found after 16s

✓ Verified in Prometheus: test_single_metric_1638360000 = 42.5

 PASS  tests/metrics-pipeline.test.ts
  ✓ should push single-value metrics and verify in Prometheus (18234 ms)
  ✓ should push grouped metrics and verify in Prometheus (19456 ms)
  ✓ should handle multiple metric batches from same instance (22178 ms)
  ✓ should handle metrics with special characters in names (18903 ms)

Test Suites: 1 passed, 1 total
Tests:       4 passed, 4 total
```

## Contributing

When adding new E2E tests:

1. Use unique metric names (timestamp-based) to avoid conflicts
2. Add descriptive console.log statements for test progress
3. Use the helper functions: `waitForMetric()`, `getMetricValue()`, `queryPrometheus()`
4. Set appropriate timeouts for your test scenario
5. Clean up any test data if needed

## Related Documentation

- [Buccaneer Metrics Client](../Web/metrics-client/README.md)
- [Prometheus Querying](https://prometheus.io/docs/prometheus/latest/querying/basics/)
- [Docker Compose Configuration](../Examples/Compose/README.md)
