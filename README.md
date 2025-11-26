# Remote Car Control Server

A simple Go server for remotely controlling a car toy over the internet, with video streaming capabilities.

## Features

- **Video Streaming**: Receives video feed from hardware and streams it to the web interface via MJPEG
- **Control Commands**: REST API endpoints for sending left/right/stop commands
- **Web Interface**: Beautiful, responsive web UI with video feed and control buttons

## Setup

1. Install Go (1.21 or later)

2. Run the server:
```bash
go run main.go
```

3. Open your browser and navigate to:
```
http://localhost:8080
```

## API Endpoints

### Web Interface
- `GET /` - Main web interface with video feed and controls

### Video
- `POST /api/video` - Hardware sends video frames (JPEG images) here
- `GET /video` - Streams video feed to web interface (MJPEG stream)

### Control
- `POST /api/command` - Send control commands (left/right/stop)
  ```json
  {
    "command": "left"  // or "right" or "stop"
  }
  ```
- `GET /api/command/get` - Hardware polls this endpoint to get the latest command

## Hardware Integration

### Sending Video Feed
Your hardware should POST JPEG frames to `/api/video`:
```bash
curl -X POST http://localhost:8080/api/video \
  -H "Content-Type: image/jpeg" \
  --data-binary @frame.jpg
```

### Receiving Commands
Your hardware should poll `/api/command/get` to receive commands:
```bash
curl http://localhost:8080/api/command/get
# Returns: {"command":"left"} or {"command":"right"} or {"command":"stop"}
```

Commands expire after 5 seconds (automatically becomes "stop" if not polled).

## Keyboard Controls

- `←` Arrow Left: Turn left
- `→` Arrow Right: Turn right
- `Space`: Stop

## Architecture

The server maintains:
- A video buffer that stores the latest frame from the hardware
- A command queue that stores the latest command from the user
- MJPEG streaming for real-time video display
- REST API for bidirectional communication

