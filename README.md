# Cauto

An advanced Minecraft autoclicker written in C to test the language's capabilities for automation tasks. Features multiple clicking modes with sophisticated humanization techniques to simulate natural clicking behavior.

**Note:** The web-based application is still in development. Currently, only the CLI version is fully functional.

## Features

### Left Clicker

Configurable autoclicker with humanization features.

**Settings**
- CPS (Clicks Per Second)
- Min/Max click intervals
- Click duration
- Click in inventory option

**Randomization Options**
- Drop chance and amount (simulates fatigue)
- Spike chance and amount (simulates attention peaks)
- Gaussian distribution for natural timing
- Mouse jitter

### Click Recorder

Record and save your exact clicking patterns.

**Recording Settings**
- Record keybind selection
- Beep on start notification
- Minecraft-only mode
- Click duration threshold
- Delay threshold

**Features**
- Record clicks with precise timing
- Save and load recordings
- View average CPS and total clicks
- Customizable recording thresholds

### Click Player

Playback recorded patterns with full control.

**Loading Options**
- Load from file (file dialog)
- Load from clipboard (hex-encoded)
- Load from string (predefined configs)
- Random starting position
- Pattern looping

### Global Settings

**Application Behavior**
- Minecraft-only mode (pause when unfocused)
- Break blocks mode
- Click in inventory globally
- Beep on toggle

**Jitter Configuration**
- Jitter chance percentage
- Jitter X offset
- Jitter Y offset

**Sound**
- Click sounds enabled/disabled
- Load WAV files

## Web-Based Interface

The web-based configuration menu is currently in development. Once completed, it will provide:
- Real-time configuration management with WebSockets
- Remote access to clicker controls
- Configuration file upload/download

For now, use the CLI interface by running the executable directly.

## TODO

- [x] Left Clicker
- [ ] Right Clicker
- [x] Click Recorder
- [x] Click Player
- [x] Advanced humanization features (drops, spikes, jitter)
- [ ] Web-based GUI
- [ ] Preset system

## License

GPL 3.0 License. See LICENSE for details.