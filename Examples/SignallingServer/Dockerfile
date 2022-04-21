FROM ghcr.io/epicgames/pixel-streaming-signalling-server:4.27

# Install the dependencies for the signalling server
WORKDIR /opt/SignallingWebServer
RUN npm install .

# Expose TCP port 80 for player WebSocket connections and web server HTTP access
EXPOSE 80

# Expose TCP port 8888 for streamer WebSocket connections
EXPOSE 8888

ENTRYPOINT ["/usr/local/bin/node", "/opt/SignallingWebServer/cirrus.js"]