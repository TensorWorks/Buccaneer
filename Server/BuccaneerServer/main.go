package main

import (
	"encoding/json"
	"flag"
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
	collectors      sync.Map
	collectorsMutex = sync.RWMutex{}
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
	records map[string]record
}

type record struct {
	value float64
	time  int64
}

type arbitraryJson map[string]interface{}

func (collector *collector) Describe(ch chan<- *prometheus.Desc) {
	collectorsMutex.Lock()
	defer collectorsMutex.Unlock()

	for _, metric := range collector.metrics {
		ch <- metric.description
	}
}

func (collector *collector) Collect(ch chan<- prometheus.Metric) {
	collectorsMutex.Lock()
	defer collectorsMutex.Unlock()

	for _, metric := range collector.metrics {

		if record, ok := metric.records[""]; ok {
			// if we can get a value from no key, this indicates a system metric and isn't per player
			ch <- prometheus.MustNewConstMetric(metric.description, prometheus.GaugeValue, record.value)
		} else {
			// otherwise we must loop through all the player ids
			for key, record := range metric.records {
				ch <- prometheus.MustNewConstMetric(metric.description, prometheus.GaugeValue, record.value, key)
			}
		}
	}
}

func removeStaleCollectors() {
	// sub-routine to remove stale collectors
	for {
		collectorsMutex.Lock()
		collectors.Range(func(key, currCollector interface{}) bool {
			collector := currCollector.(collector)
			// TODO (belchy06): Perhaps this time should be configurable
			for name, metric := range collector.metrics {
				for id, record := range metric.records {
					if time.Now().Unix()-record.time > 60 {
						log.Printf("Deregistering \"%s\" records for player \"%s\"", name, id)
						delete(metric.records, id)
					}
				}
			}

			if time.Now().Unix()-*collector.lastUpdateTime > 60 {
				// Collector hasn't been update in X seconds, unregister and remove from our map
				log.Printf("Deregistering collector for instance \"%s\"", key)
				prometheus.Unregister(&collector)
				collectors.Delete(key)
			}
			return true
		})
		collectorsMutex.Unlock()
		// sleep before running again
		time.Sleep(5 * time.Second)
	}
}

func main() {
	// Determine the port to use: command line arg > environment variable > default
	port := flag.String("port", os.Getenv("PORT"), "Port to listen on (can also be set via PORT environment variable)")
	flag.Parse()

	// Use default if neither flag nor env var is set
	if *port == "" {
		*port = "8000"
	}

	// start sub-routine
	go removeStaleCollectors()

	// either open or create the events log file scraped by promtail
	fs, err := os.OpenFile("./event.log", os.O_APPEND|os.O_WRONLY|os.O_CREATE, 0644)
	if err != nil {
		log.Fatal(err.Error())
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
		if payload["level"] == nil || payload["message"] == nil || payload["id"] == nil {
			http.Error(res, "Malformed event json. Ensure your message contains all the required fields", http.StatusBadRequest)
			return
		}

		var ts = time.Now().Format(time.RFC3339Nano)
		_, _ = fs.WriteString("{" + "\"log\":\"level=" + payload["level"].(string) + " ts=" + ts + " msg=\\\"" + payload["message"].(string) + "\\\"\", \"stream\":\"" + payload["level"].(string) + "\", \"time\":\"" + ts + "\", \"instance\":\"" + payload["id"].(string) + "\"}\n")

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
			collectorsMutex.Lock()

			ts := time.Now().Unix()
			collector := collector{
				metadata:       make(map[string]string),
				metrics:        make(map[string]metric),
				lastUpdateTime: &ts,
			}

			// always add the id to this collectors metadata
			collector.metadata["id"] = id.(string)

			if data := payload["metadata"]; data != nil {
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

					// Get description, defaulting to the metric name if not provided
					description := key
					if desc, ok := metricJson["description"].(string); ok {
						description = desc
					}

					if valueArray, ok := metricJson["value"].([]interface{}); ok {
						// if the value object is an array, then loop through this array
						recordMap := make(map[string]record)
						// if the value object is an array, then loop through this array
						for _, val := range valueArray {
							for k, v := range val.(map[string]interface{}) {
								recordMap[k] = record{
									value: v.(float64),
									time:  ts,
								}
							}
						}

						collector.metrics[key] = metric{
							description: prometheus.NewDesc(key, description, []string{"player"}, collector.metadata),
							records:     recordMap,
						}
					} else {
						recordMap := make(map[string]record)
						recordMap[""] = record{
							value: metricJson["value"].(float64),
							time:  ts,
						}
						collector.metrics[key] = metric{
							description: prometheus.NewDesc(key, description, nil, collector.metadata),
							records:     recordMap,
						}
					}
				}
			}

			// store collector in our internal map
			collectors.Store(id, collector)

			collectorsMutex.Unlock()

			// register collector with Prometheus (must be done outside the mutex lock
			// because Prometheus will call Describe which also needs the mutex)
			prometheus.Register(&collector)
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
		temp, exists := collectors.Load(id.(string))
		if !exists {
			http.Error(res, "Unable to load collector", http.StatusBadRequest)
			return
		}

		collectorsMutex.Lock()

		collector := temp.(collector)
		metricsJson := data.(map[string]interface{})
		// iterate over all the fields in the "metrics" section
		for key, value := range metricsJson {
			metricJson := value.(map[string]interface{})

			// Get description, defaulting to the metric name if not provided
			description := key
			if desc, ok := metricJson["description"].(string); ok {
				description = desc
			}

			if valueArray, ok := metricJson["value"].([]interface{}); ok {
				// if the value object is an array, then loop through this array
				recordMap := make(map[string]record)
				for _, val := range valueArray {
					for k, v := range val.(map[string]interface{}) {
						recordMap[k] = record{
							value: v.(float64),
							time:  time.Now().Unix(),
						}
					}
				}

				collector.metrics[key] = metric{
					description: prometheus.NewDesc(key, description, []string{"player"}, collector.metadata),
					records:     recordMap,
				}
			} else {
				recordMap := make(map[string]record)
				recordMap[""] = record{
					value: metricJson["value"].(float64),
					time:  time.Now().Unix(),
				}
				collector.metrics[key] = metric{
					description: prometheus.NewDesc(key, description, nil, collector.metadata),
					records:     recordMap,
				}
			}
		}

		collectors.Store(id, collector)

		// update lastUpdateTime
		*collector.lastUpdateTime = time.Now().Unix()
		collectorsMutex.Unlock()
		// return OK
		res.WriteHeader(http.StatusOK)
	})

	// handler for root endpoint - health check
	http.HandleFunc("/", func(res http.ResponseWriter, req *http.Request) {
		res.WriteHeader(http.StatusOK)
		res.Write([]byte("Buccaneer Server is Running"))
	})

	// handler for when prometheus scrapes data
	http.Handle("/metrics", promhttp.Handler())

	// Log server startup information
	addr := "127.0.0.1:" + *port
	log.Println("===========================================")
	log.Printf("Buccaneer Server starting on http://%s", addr)
	log.Println("===========================================")
	log.Println("Available endpoints:")
	log.Printf("  GET  http://%s/        - Server health check", addr)
	log.Printf("  POST http://%s/event   - Receive semantic events from Buccaneer clients such as UE Buccaneer plugins", addr)
	log.Printf("  POST http://%s/stats   - Receive performance metrics from Buccaneer clients such as UE Buccaneer plugins", addr)
	log.Printf("  GET  http://%s/metrics - Prometheus scrape endpoint", addr)
	log.Println("===========================================")

	log.Fatal(http.ListenAndServe(":"+*port, nil))
}
