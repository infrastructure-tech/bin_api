FROM restbed:ubuntu

COPY /src/example /entrypoint
RUN chmod +x /entrypoint

EXPOSE 1984

ENTRYPOINT ["/entrypoint"]


