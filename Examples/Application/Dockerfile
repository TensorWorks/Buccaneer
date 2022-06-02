FROM ghcr.io/epicgames/unreal-engine:runtime-pixel-streaming

# Copy UE4 project
COPY --chown=ue4:ue4 ./LinuxNoEditor /home/ue4/project

EXPOSE 8888

USER ue4
RUN ["chmod", "+x", "/home/ue4/project/TP4.sh"]
 ENTRYPOINT ["/usr/bin/entrypoint.sh", "/home/ue4/project/TP4.sh",  "-StatsEmitterURL=http://127.0.0.1:8000",  "-EventEmitterURL=http://127.0.0.1:8080", "-RenderOffScreen", "-Unattended", "-Vulkan", "-PixelStreamingURL=ws://127.0.0.1:8888", "-ResX=1920", "-ResY=1080", "-ForceRes" ]