FROM restbed:ubuntu

COPY /build/out/entrypoint /entrypoint
RUN chmod +x /entrypoint

EXPOSE 1984

ENTRYPOINT ["/entrypoint"]


