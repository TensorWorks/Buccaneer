FROM ghcr.io/epicgames/unreal-engine:dev-4.27.2

# Copy UE4 project
COPY --chown=ue4:ue4 ./LinuxNoEditor /home/ue4/project

USER ue4
RUN ["chmod", "+x", "/home/ue4/project/TP4.sh"]
# ENTRYPOINT ["/home/ue4/project/TP4.sh", "-renderoffscreen", "-fullstdlogoutput", "-Unattended", "-Vulkan"]
CMD ["/home/ue4/project/TP4.sh", "-renderoffscreen", "-fullstdlogoutput", "-Unattended", "-opengl4"]
 #ENTRYPOINT ["/home/ue4/project/TP4.sh",  "-StatsEmitterURL=http://stats:8000",  "-EventEmitterURL=http://events:8080", "-RenderOffScreen", "-Unattended", "-Vulkan"]