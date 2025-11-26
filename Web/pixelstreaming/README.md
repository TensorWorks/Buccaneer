# Buccaneer Pixel Streaming Integration

Pixel Streaming frontend application with integrated Buccaneer metrics collection. Automatically captures Pixel Streaming performance stats and sends them to a Buccaneer server for Prometheus monitoring.

## Features

- 🎮 Full Pixel Streaming frontend with UI library
- 📊 Automatic metrics collection from Pixel Streaming events
- 🏷️ Per-player grouped metrics using Pixel Streaming player IDs
- ⚡ Real-time metrics sent immediately when events fire
- 🔧 Easy configuration via URL parameters or environment variables

## Installation

```bash
npm install
```

## Development

Start the development server with hot module replacement:

```bash
npm run dev
```

This will start Vite dev server on `http://localhost:3000`

## Building

Build for production:

```bash
npm run build
```

Preview the production build:

```bash
npm run preview
```

## Configuration

### Buccaneer Server URL

Configure where metrics are sent:

**Via URL parameter:**
```
http://localhost:3000?buccaneerUrl=http://my-server:8000
```

**Via environment variable:**
```bash
VITE_BUCCANEER_URL=http://my-server:8000 npm run dev
```

**Default:** `http://localhost:8000`

## Metrics Collected

Metrics are sent **immediately** when Pixel Streaming events fire, providing real-time performance data.


## Architecture

```
┌─────────────────────┐
│ Pixel Streaming UE  │
│    Application      │
└──────────┬──────────┘
           │ WebRTC
           ↓
┌─────────────────────┐
│  This Frontend App  │
│  ┌───────────────┐  │
│  │ PS UI Library │  │
│  └───────┬───────┘  │
│          │ events   │
│  ┌───────↓───────┐  │
│  │   Buccaneer   │  │
│  │    Metrics    │  │
│  └───────┬───────┘  │
└──────────┼──────────┘
           │ HTTP POST
           ↓
┌─────────────────────┐
│ Buccaneer Server    │
└──────────┬──────────┘
           │ /metrics
           ↓
┌─────────────────────┐
│    Prometheus       │
└─────────────────────┘
```

## Related Documentation

- [Buccaneer Metrics Client](../metrics-client/README.md)
- [Pixel Streaming Documentation](https://docs.unrealengine.com/5.7/en-US/pixel-streaming-in-unreal-engine/)
- [Pixel Streaming Infrastructure](https://github.com/EpicGamesExt/PixelStreamingInfrastructure)
