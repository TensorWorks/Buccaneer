package main

import (
	"net/http"
	"os"
	"time"

	"github.com/gin-gonic/gin"
)

type Event struct {
	ID      string `json:"id" binding:"required"`
	Level   string `json:"level" binding:"required"`
	Message string `json:"message" binding:"required"`
}

func main() {
	r := gin.Default()
	fs, err := os.OpenFile("event.log", os.O_APPEND|os.O_WRONLY|os.O_CREATE, 0644)
	if err != nil {
		panic(err)
	}

	defer fs.Close()

	r.GET("/", func(c *gin.Context) {
		c.JSON(http.StatusOK, gin.H{"data": "hello world"})
	})

	r.POST("/event", func(c *gin.Context) {
		var event Event
		// Missing elements
		if err := c.ShouldBindJSON(&event); err != nil {
			c.JSON(http.StatusBadRequest, gin.H{"status": "error"})
			return
		}
		var ts = time.Now().Format(time.RFC3339Nano)
		_, err = fs.WriteString("{" + "\"log\":\"level=" + event.Level + " ts=" + ts + " msg=\\\"" + event.Message + "\\\"\", \"stream\":\"" + event.Level + "\", \"time\":\"" + ts + "\", \"instance\":\"" + event.ID + "\"}\n")
		c.JSON(http.StatusOK, gin.H{"status": "success"})
	})

	r.Run()
}
