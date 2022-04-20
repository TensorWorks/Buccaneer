# syntax=docker/dockerfile:1

FROM golang:1.16-alpine

WORKDIR /app

COPY go.mod ./
COPY go.sum ./
RUN go mod download

COPY *.go ./

RUN mkdir /EventsServer && go build -o /EventsServer/EventsServer

EXPOSE 8080

CMD ["/EventsServer/EventsServer"]