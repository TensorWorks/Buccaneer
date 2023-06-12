package main

import (
	"encoding/json"
	"io"
	"log"
	"net/http"
	"os"
	"sync"
	"time"

	"github.com/prometheus/client_golang/prometheus"
	"github.com/prometheus/client_golang/prometheus/promhttp"
)

var (
	// global map to store our collectors
	collectors sync.Map
)

type collector struct {
	// metadata are labels that don't change during runtime. Eg you could tag instances with
	// their corresponding CL number to gain insight into performance regressions over time
	metadata map[string]string

	// metrics contain both a long description and a value
	metrics map[string]metric

	// unix timestamp to keep track of when this collector was last updated.
	// enables removal of stale collectors
	lastUpdateTime *int64
}

type metric struct {
	// long, human-readable, description
	description *prometheus.Desc
	//
	value float64
}

type arbitraryJson map[string]interface{}

func (collector *collector) Describe(ch chan<- *prometheus.Desc) {
	for _, metric := range collector.metrics {
		ch <- metric.description
	}
}

func (collector *collector) Collect(ch chan<- prometheus.Metric) {
	for _, metric := range collector.metrics {
		ch <- prometheus.MustNewConstMetric(metric.description, prometheus.GaugeValue, metric.value)
	}
}

func removeStaleCollectors() {
	// sub-routine to remove stale collectors
	for {
		collectors.Range(func(key, currCollector interface{}) bool {
			collector := currCollector.(collector)
			// TODO (belchy06): Perhaps this time should be configurable
			if time.Now().Unix()-*collector.lastUpdateTime > 10 {
				// Collector hasn't been update in X seconds, unregister and remove from our map
				log.Printf("Deregistering collector for instance \"%s\"", key)
				prometheus.Unregister(&collector)
				collectors.Delete(key)
			}
			return true
		})

		// sleep before running again
		time.Sleep(5 * time.Second)
	}
}

func main() {
	// start sub-routine
	go removeStaleCollectors()

	// either open or create the events log file scraped by promtail
	fs, err := os.OpenFile("event.log", os.O_APPEND|os.O_WRONLY|os.O_CREATE, 0644)
	if err != nil {
		panic(err)
	}

	defer fs.Close()

	// handler for when an instance posts an event
	http.HandleFunc("/event", func(res http.ResponseWriter, req *http.Request) {
		body, err := io.ReadAll(req.Body)
		if err != nil {
			http.Error(res, err.Error(), http.StatusBadRequest)
			return
		}

		var payload arbitraryJson
		json.Unmarshal([]byte(body), &payload)

		// check the event json contains a level, message and id
		if payload["Level"] == nil || payload["Message"] == nil || payload["ID"] == nil {
			http.Error(res, "Malformed event json. Ensure your message contains all the required fields", http.StatusBadRequest)
			return
		}

		var ts = time.Now().Format(time.RFC3339Nano)
		_, _ = fs.WriteString("{" + "\"log\":\"level=" + payload["Level"].(string) + " ts=" + ts + " msg=\\\"" + payload["Message"].(string) + "\\\"\", \"stream\":\"" + payload["Level"].(string) + "\", \"time\":\"" + ts + "\", \"instance\":\"" + payload["ID"].(string) + "\"}\n")

		res.WriteHeader(http.StatusOK)
	})

	// handler for when an instance posts its stats
	http.HandleFunc("/stats", func(res http.ResponseWriter, req *http.Request) {
		body, err := io.ReadAll(req.Body)
		if err != nil {
			http.Error(res, err.Error(), http.StatusBadRequest)
			return
		}

		var payload arbitraryJson
		json.Unmarshal([]byte(body), &payload)

		id := payload["id"]
		if id == nil {
			// error out if the instance doesn't provide us with its id
			http.Error(res, "ID not present in payload", http.StatusBadRequest)
			return
		}

		if _, exists := collectors.Load(id.(string)); !exists {
			// this is the first time we're seeing this ID, so configure accordingly
			ts := time.Now().Unix()
			collector := collector{
				metadata:       make(map[string]string),
				metrics:        make(map[string]metric),
				lastUpdateTime: &ts,
			}

			if data := payload["metadata"]; data != nil {
				// add the id to this collectors metadata
				collector.metadata["id"] = id.(string)

				// add user specifed metadata to the collector
				metadata := data.(map[string]interface{})
				for key, value := range metadata {
					collector.metadata[key] = value.(string)
				}
			}

			// add all of the metrics in this payload to our collector
			if data := payload["metrics"]; data != nil {
				metrics := data.(map[string]interface{})
				for key, value := range metrics {
					if _, exists := collector.metrics[key]; exists {
						continue
					}

					metricJson := value.(map[string]interface{})
					collector.metrics[key] = metric{
						description: prometheus.NewDesc(key, metricJson["description"].(string), nil, collector.metadata),
						value:       metricJson["value"].(float64),
					}
				}
			}

			// store collector in our internal map
			collectors.Store(id, collector)
			// register collector with Prometheus
			prometheus.MustRegister(&collector)
			log.Printf("Registering collector for instance \"%s\"", id)

			// return OK
			res.WriteHeader(http.StatusOK)
			return
		}

		// collector already exists, just update metric values
		data := payload["metrics"]
		if data == nil {
			http.Error(res, "Malformed stats json. Ensure your message contains all the required fields", http.StatusBadRequest)
			return
		}

		// collector should exist at this point, but check just to be sure
		currCollector, exists := collectors.Load(id.(string))
		if !exists {
			http.Error(res, "Unable to load collector", http.StatusBadRequest)
			return
		}

		metricsJson := data.(map[string]interface{})
		// iterate over all the fields in the "metrics" section
		for key, value := range metricsJson {
			metricJson := value.(map[string]interface{})

			if entry, ok := currCollector.(collector).metrics[key]; ok {
				// update value of copy
				entry.value = metricJson["value"].(float64)
				// assign copy to metric
				currCollector.(collector).metrics[key] = entry
			}
		}

		// update lastUpdateTime
		*currCollector.(collector).lastUpdateTime = time.Now().Unix()
		// return OK
		res.WriteHeader(http.StatusOK)
	})

	// handler for when prometheus scrapes data
	http.Handle("/metrics", promhttp.Handler())

	// TODO (belchy06): Perhaps this port should be configurable
	log.Fatal(http.ListenAndServe(":8000", nil))
}
