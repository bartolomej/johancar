package main

import (
	"bytes"
	"encoding/json"
	"fmt"
	"html/template"
	"image"
	"image/color"
	"image/jpeg"
	"io"
	"log"
	"math"
	"net/http"
	"sync"
	"time"
)

type Server struct {
	videoBuffer []byte
	videoMutex  sync.RWMutex
	frameCount  int
	frameMutex  sync.Mutex
}

func NewServer() *Server {
	return &Server{
		videoBuffer: make([]byte, 0),
	}
}

func (s *Server) handleVideoFeed(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	body, err := io.ReadAll(r.Body)
	if err != nil {
		http.Error(w, "Failed to read body", http.StatusBadRequest)
		return
	}
	defer r.Body.Close()

	s.videoMutex.Lock()
	s.videoBuffer = body
	s.videoMutex.Unlock()

	w.WriteHeader(http.StatusOK)
}

func (s *Server) generateTestFrame() []byte {
	s.frameMutex.Lock()
	s.frameCount++
	frameNum := s.frameCount
	s.frameMutex.Unlock()

	width := 640
	height := 360
	img := image.NewRGBA(image.Rect(0, 0, width, height))

	timeValue := float64(frameNum) * 0.1
	for y := 0; y < height; y++ {
		for x := 0; x < width; x++ {
			r := uint8(128 + 127*math.Sin(float64(x)/50+timeValue))
			g := uint8(128 + 127*math.Sin(float64(y)/50+timeValue))
			b := uint8(128 + 127*math.Sin(float64(x+y)/100+timeValue))
			img.Set(x, y, color.RGBA{r, g, b, 255})
		}
	}

	var buf bytes.Buffer
	jpeg.Encode(&buf, img, &jpeg.Options{Quality: 80})
	return buf.Bytes()
}

func (s *Server) streamVideo(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "multipart/x-mixed-replace; boundary=frame")
	w.Header().Set("Cache-Control", "no-cache")
	w.Header().Set("Connection", "keep-alive")

	ticker := time.NewTicker(100 * time.Millisecond)
	defer ticker.Stop()

	for {
		select {
		case <-r.Context().Done():
			return
		case <-ticker.C:
			s.videoMutex.RLock()
			var frame []byte
			if len(s.videoBuffer) > 0 {
				frame = s.videoBuffer
			} else {
				frame = s.generateTestFrame()
			}
			s.videoMutex.RUnlock()

			fmt.Fprintf(w, "--frame\r\n")
			fmt.Fprintf(w, "Content-Type: image/jpeg\r\n")
			fmt.Fprintf(w, "Content-Length: %d\r\n\r\n", len(frame))
			w.Write(frame)
			fmt.Fprintf(w, "\r\n")
			if f, ok := w.(http.Flusher); ok {
				f.Flush()
			}
		}
	}
}

func (s *Server) pushCommandToHardware(command string) {
	log.Printf("Pushing command to hardware: %s", command)
}

func (s *Server) handleCommand(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	var cmd struct {
		Command string `json:"command"`
	}

	if err := json.NewDecoder(r.Body).Decode(&cmd); err != nil {
		http.Error(w, "Invalid JSON", http.StatusBadRequest)
		return
	}

	if cmd.Command != "left" && cmd.Command != "right" {
		http.Error(w, "Invalid command. Use 'left' or 'right'", http.StatusBadRequest)
		return
	}

	s.pushCommandToHardware(cmd.Command)

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(map[string]string{"status": "ok", "command": cmd.Command})
}

func (s *Server) handleWebView(w http.ResponseWriter, r *http.Request) {
	t, err := template.ParseFiles("templates/index.html")
	if err != nil {
		http.Error(w, "Failed to parse template", http.StatusInternalServerError)
		return
	}

	w.Header().Set("Content-Type", "text/html; charset=utf-8")
	t.Execute(w, nil)
}

func main() {
	server := NewServer()

	http.HandleFunc("/", server.handleWebView)
	http.HandleFunc("/video", server.streamVideo)
	http.HandleFunc("/api/video", server.handleVideoFeed)
	http.HandleFunc("/api/command", server.handleCommand)

	port := ":8080"
	log.Printf("Server starting on http://localhost%s", port)
	log.Printf("Web interface: http://localhost%s", port)
	log.Printf("Video feed endpoint: http://localhost%s/api/video (POST)", port)
	log.Printf("Command endpoint: http://localhost%s/api/command (POST)", port)

	if err := http.ListenAndServe(port, nil); err != nil {
		log.Fatal(err)
	}
}
