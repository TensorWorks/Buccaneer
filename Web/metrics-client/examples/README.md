# Buccaneer Metrics Client Examples

Examples demonstrating how to use the `@tensorworks/buccaneer-metrics-client` library.

## Setup

1. Build the parent library:
   ```bash
   cd ..
   npm install
   npm run build
   ```

2. Install example dependencies:
   ```bash
   cd examples
   npm install
   ```

## Running Examples

### Basic Example

Demonstrates single-value and grouped metrics:

```bash
npm run example:basic
```

This example shows:
- Creating a metrics collection
- Adding single-value metrics (fps, memory, cpu)
- Adding grouped metrics (per-player stats)
- Sending metrics to a Buccaneer server
- Clearing metrics

## Prerequisites

Some examples require a running Buccaneer server. You can start one using Docker Compose:

```bash
cd ../../Examples/Compose
docker compose up -d
```

The server will be available at `http://localhost:8000`.
