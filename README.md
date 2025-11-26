# JohanCar - Remote Car Control Server

A simple Go server for remotely controlling a car toy over the internet, with video streaming capabilities.

## Features

- **Video Streaming**: Receives video feed from hardware and streams it to the web interface via MJPEG
- **Direct Command Control**: Commands are pushed directly to hardware on the local network
- **Web Interface**: Responsive web UI with video feed and control buttons
- **Test Video**: Generates animated test pattern when hardware video is unavailable

## Quick Start

1. Install Go (1.21 or later)

2. Run the server:
```bash
go run main.go
```

3. Open your browser:
```
http://localhost:8080
```

## API Endpoints

- `GET /` - Web interface with video feed and controls
- `GET /video` - MJPEG video stream
- `POST /api/video` - Hardware sends JPEG frames here
- `POST /api/command` - Send control commands (left/right)
  ```json
  {
    "command": "left | right"
  }
  ```

## Hardware Integration

### Sending Video Feed
POST JPEG frames to `/api/video`:
```bash
curl -X POST http://localhost:8080/api/video \
  -H "Content-Type: image/jpeg" \
  --data-binary @frame.jpg
```

## Keyboard Controls

- `←` Arrow Left: Turn left
- `→` Arrow Right: Turn right
