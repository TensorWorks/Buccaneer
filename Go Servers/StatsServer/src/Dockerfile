# syntax=docker/dockerfile:1

FROM golang:1.16-alpine

WORKDIR /app

COPY go.mod ./
COPY go.sum ./
RUN go mod download

COPY collections ./collections
COPY *.go ./

RUN go build -o /StatsServer

EXPOSE 8000

CMD ["/StatsServer"]