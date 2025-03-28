# Cricket Scoreboard Application

A professional cricket scoreboard application with a C backend server and a modern web interface.

## Features

- Real-time score updates for two teams
- Track runs, wickets, overs, and extras
- Modern and responsive UI
- Easy-to-use controls for score management
- Team name swapping functionality
- Score reset capability
- Mobile-friendly design

## Prerequisites

- GCC compiler
- Web browser
- Basic understanding of terminal/command prompt

## Installation

1. Clone this repository:
```bash
git clone <repository-url>
cd cricket-scoreboard
```

2. Compile the server:
```bash
make
```

## Running the Application

1. Start the server:
```bash
make run
```

2. Open `index.html` in your web browser

3. The application will be running at `http://localhost:8080`

## Usage

- Use the +1, +4, and +6 buttons to add runs
- Use the +1 Wicket button to add wickets
- Use the +1 Extra button to add extras (automatically adds to total runs)
- Use the +0.1 Over button to increment overs
- Use "Reset Scores" to start fresh
- Use "Swap Teams" to switch team names

## Technical Details

- Backend: C-based HTTP server
- Frontend: HTML5, CSS3, JavaScript
- Real-time updates using fetch API
- Responsive design using CSS Grid and Flexbox

## Development

To modify the application:

1. Server code is in `server.c`
2. Frontend interface is in `index.html`
3. Styles are in `styles.css`
4. JavaScript functionality is in `script.js`

## Contributing

Feel free to submit issues and enhancement requests! 