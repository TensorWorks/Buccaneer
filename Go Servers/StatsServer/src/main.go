package main

import (
	"StatsServer/collections"
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"time"

	"github.com/google/uuid"
	"github.com/prometheus/client_golang/prometheus"
	"github.com/prometheus/client_golang/prometheus/promhttp"
)

// collector (1) -> instance (1) -> players (many)

type Collector struct {
	// metadata is labels that don't change during runtime
	metadata map[string]string

	metrics map[string]Metric

	lastUpdateTime *int64
}

type Metric struct {
	Description *prometheus.Desc
	Value       *collections.OrderedMap
	ValueType   prometheus.ValueType
	PerPlayer   bool
}

type ArbitraryJson map[string]interface{}

var (
	Collectors = make(map[string]Collector)
)

func (collector *Collector) Describe(ch chan<- *prometheus.Desc) {
	for _, metric := range collector.metrics {
		ch <- metric.Description
	}
}

func (collector *Collector) Collect(ch chan<- prometheus.Metric) {
	for _, metric := range collector.metrics {
		if metric.PerPlayer {
			for el := metric.Value.Front(); el != nil; el = el.Next() {
				ch <- prometheus.MustNewConstMetric(metric.Description, metric.ValueType, el.Value.(float64), el.Key.(string))
			}
		} else {
			val, success := metric.Value.Get("")
			if success {
				ch <- prometheus.MustNewConstMetric(metric.Description, metric.ValueType, val.(float64))
			}
		}
	}
}

func removeStaleCollectors() {
	for {
		for key, collector := range Collectors {
			if time.Now().Unix()-*collector.lastUpdateTime > 60 {
				log.Printf("Deregistering collector for instance \"%s\"", key)
				prometheus.Unregister(&collector)
				delete(Collectors, key)
			}
		}
		time.Sleep(5 * time.Second)
	}
}

func GetValueType(ValueString string) (prometheus.ValueType, error) {
	switch ValueString {
	case "gauge":
		return prometheus.GaugeValue, nil
	case "counter":
		return prometheus.CounterValue, nil
	}
	return prometheus.UntypedValue, errors.New("Unkown value type")
}

func ToStringArray(in []interface{}) []string {
	labelArr := make([]string, len(in))
	for i, v := range in {
		labelArr[i] = v.(string)
	}
	return labelArr
}

func main() {
	go removeStaleCollectors()

	http.Handle("/metrics", promhttp.Handler())

	http.HandleFunc("/setup", func(res http.ResponseWriter, req *http.Request) {
		body, err := ioutil.ReadAll(req.Body)
		if err != nil {
			http.Error(res, err.Error(), http.StatusBadRequest)
			return
		}

		var payload ArbitraryJson
		json.Unmarshal([]byte(body), &payload)
		ts := time.Now().Unix()
		collector := Collector{
			metadata:       make(map[string]string),
			metrics:        make(map[string]Metric),
			lastUpdateTime: &ts,
		}

		// Metadata
		data := payload["metadata"]
		if data != nil {
			metadata := data.(map[string]interface{})
			for key, value := range metadata {
				collector.metadata[key] = value.(string)
			}
		}

		if id, exists := collector.metadata["id"]; exists {
			// ID was provided
			if _, exists := Collectors[id]; exists {
				// Setup has been called a second time for this instance. update the original collector with the new information
				collector = Collectors[id]
				if len(collector.metrics) > 0 {
					prometheus.Unregister(&collector)
				}
				log.Printf("Updated logging stats for instance \"%s\"", id)
			}
		} else {
			// ID not provided, generate it
			id := uuid.New().String()
			// Check that the generated UUID is not already used. Continually generate one until we get an unused one
			for _, exists := collector.metadata[id]; exists; _, exists = collector.metadata[id] {
				id = uuid.New().String()
			}
			collector.metadata["id"] = id
			log.Printf("Started logging stats for instance \"%s\"", id)
		}
		id := collector.metadata["id"]

		// Metrics
		data = payload["metrics"]
		if data != nil {
			metrics := data.(map[string]interface{})
			for key, value := range metrics {
				if _, exists := collector.metrics[key]; exists {
					continue
				}
				metric := value.(map[string]interface{})
				valueType, err := GetValueType(metric["type"].(string))
				if err != nil {
					http.Error(res, err.Error(), http.StatusBadRequest)
					return
				}

				perPlayer := func() []string {
					if metric["perPlayer"] != nil && metric["perPlayer"].(bool) {
						return []string{"player"}
					} else {
						return nil
					}
				}()

				valueMap := collections.NewOrderedMap()
				collector.metrics[key] = Metric{
					Description: prometheus.NewDesc(key, metric["description"].(string), perPlayer, collector.metadata),
					ValueType:   valueType,
					Value:       valueMap,
					PerPlayer:   (len(perPlayer) > 0),
				}
			}
		}

		Collectors[id] = collector
		if len(collector.metrics) > 0 {
			prometheus.MustRegister(&collector)
		}

		res.WriteHeader(http.StatusCreated)
		res.Header().Set("Content-Type", "application/json")
		resp := make(map[string]string)
		resp["id"] = id
		// json.NewEncoder(res).Encode(responseData)
		jsonResp, err := json.Marshal(resp)
		if err != nil {
			log.Fatalf("Error happened in JSON marshal. Err: %s", err)
		}
		res.Write(jsonResp)
	})

	http.HandleFunc("/stats", func(res http.ResponseWriter, req *http.Request) {
		body, err := ioutil.ReadAll(req.Body)
		if err != nil {
			http.Error(res, err.Error(), http.StatusBadRequest)
			return
		}

		var payload ArbitraryJson
		json.Unmarshal([]byte(body), &payload)

		id := payload["id"]
		if id == nil {
			http.Error(res, "ID not provided in stats payload", http.StatusBadRequest)
			return
		}
		if _, exists := Collectors[id.(string)]; !exists {
			http.Error(res, "Setup hasn't been called from this instance", http.StatusBadRequest)
			return
		}

		for k, v := range payload {
			if _, exists := Collectors[id.(string)].metrics[k]; !exists || k == "id" {
				continue
			}
			switch i := v.(type) {
			case float64:
				if !Collectors[id.(string)].metrics[k].PerPlayer {
					Collectors[id.(string)].metrics[k].Value.Set("", i)
				} else {
					http.Error(res, fmt.Sprintf("A per player metric (%s) was missing the player number", k), http.StatusBadRequest)
					return
				}
			case int:
				if !Collectors[id.(string)].metrics[k].PerPlayer {
					Collectors[id.(string)].metrics[k].Value.Set("", float64(i))
				} else {
					http.Error(res, fmt.Sprintf("A per player metric (%s) was missing the player number", k), http.StatusBadRequest)
					return
				}
			case interface{}:
				for _, element := range v.([]interface{}) {
					for key, value := range element.(map[string]interface{}) {
						Collectors[id.(string)].metrics[k].Value.Set(key, value)
					}
				}
			}
		}
		*Collectors[id.(string)].lastUpdateTime = time.Now().Unix()
		res.WriteHeader(http.StatusOK)
	})

	http.HandleFunc("/delete", func(res http.ResponseWriter, req *http.Request) {
		body, err := ioutil.ReadAll(req.Body)
		if err != nil {
			http.Error(res, err.Error(), http.StatusBadRequest)
			return
		}

		var payload ArbitraryJson
		json.Unmarshal([]byte(body), &payload)
		id := payload["id"]
		if id == nil {
			http.Error(res, "ID not provided in delete payload", http.StatusBadRequest)
			return
		}

		if col, exists := Collectors[id.(string)]; exists {
			prometheus.Unregister(&col)
			delete(Collectors, id.(string))
		} else {
			http.Error(res, "Provided ID didn't exist", http.StatusBadRequest)
			return
		}

		res.WriteHeader(http.StatusOK)
	})

	http.HandleFunc("/deletePlayer", func(res http.ResponseWriter, req *http.Request) {
		body, err := ioutil.ReadAll(req.Body)
		if err != nil {
			http.Error(res, err.Error(), http.StatusBadRequest)
			return
		}

		var payload ArbitraryJson
		json.Unmarshal([]byte(body), &payload)
		id := payload["id"]
		if id == nil {
			http.Error(res, "Instance ID not provided in delete payload", http.StatusBadRequest)
			return
		}

		playerId := payload["playerId"]
		if playerId == nil {
			http.Error(res, "Instance ID not provided in delete payload", http.StatusBadRequest)
			return
		}

		if collector, exists := Collectors[id.(string)]; exists {
			prometheus.Unregister(&collector)
			for _, metric := range collector.metrics {
				if metric.PerPlayer {
					for el := metric.Value.Front(); el != nil; el = el.Next() {
						if el.Key == playerId.(string) {
							metric.Value.Delete(playerId.(string))

							break
						}
					}
				}
			}
			Collectors[id.(string)] = collector
			prometheus.MustRegister(&collector)
		}

		res.WriteHeader(http.StatusOK)
	})

	log.Fatal(http.ListenAndServe(":8000", nil))
}
