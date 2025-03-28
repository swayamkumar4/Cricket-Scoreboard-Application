// State management
let state = {
    team1: {
        name: "Team 1",
        runs: 0,
        wickets: 0,
        overs: 0.0,
        extras: 0
    },
    team2: {
        name: "Team 2",
        runs: 0,
        wickets: 0,
        overs: 0.0,
        extras: 0
    }
};

let isAdmin = false;

// Check admin status and update UI accordingly
function checkAdminStatus() {
    const adminAuth = localStorage.getItem('adminAuth');
    if (adminAuth) {
        fetch('http://localhost:8080/api/auth', {
            method: 'POST',
            headers: {
                'Authorization': 'Basic ' + adminAuth
            }
        })
        .then(response => response.json())
        .then(data => {
            isAdmin = data.isAdmin;
            updateControlsVisibility();
        })
        .catch(error => {
            console.error('Error:', error);
            isAdmin = false;
            updateControlsVisibility();
        });
    } else {
        isAdmin = false;
        updateControlsVisibility();
    }
}

// Update controls visibility based on admin status
function updateControlsVisibility() {
    const controls = document.querySelectorAll('.controls, .match-controls');
    const loginLink = document.getElementById('loginLink');
    const logoutLink = document.getElementById('logoutLink');

    controls.forEach(control => {
        control.style.display = isAdmin ? 'grid' : 'none';
    });

    if (loginLink && logoutLink) {
        loginLink.style.display = isAdmin ? 'none' : 'inline';
        logoutLink.style.display = isAdmin ? 'inline' : 'none';
    }
}

// Update the display with current state
function updateDisplay() {
    for (let teamNum = 1; teamNum <= 2; teamNum++) {
        const team = state[`team${teamNum}`];
        document.getElementById(`team${teamNum}-name`).textContent = team.name;
        document.getElementById(`team${teamNum}-runs`).textContent = team.runs;
        document.getElementById(`team${teamNum}-wickets`).textContent = team.wickets;
        document.getElementById(`team${teamNum}-overs`).textContent = team.overs.toFixed(1);
        document.getElementById(`team${teamNum}-extras`).textContent = team.extras;
    }
}

// Update score for a team
function updateScore(teamNum, type, value) {
    if (!isAdmin) {
        alert('Please login as admin to update scores.');
        window.location.href = '/login.html';
        return;
    }

    const team = state[`team${teamNum}`];
    
    if (type === 'wickets' && team.wickets >= 10) {
        alert('All out! Maximum wickets reached.');
        return;
    }

    if (type === 'runs' || type === 'extras') {
        team[type] += value;
        if (type === 'extras') {
            team.runs += value; // Extras are included in total runs
        }
    } else if (type === 'wickets') {
        team[type] += value;
    }

    // Send update to server with admin authentication
    fetch('http://localhost:8080/api/scores', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
            'Authorization': 'Basic ' + localStorage.getItem('adminAuth')
        },
        body: JSON.stringify(state)
    }).catch(error => console.error('Error:', error));

    updateDisplay();
}

// Update overs
function updateOvers(teamNum, value) {
    if (!isAdmin) {
        alert('Please login as admin to update scores.');
        window.location.href = '/login.html';
        return;
    }

    const team = state[`team${teamNum}`];
    let currentOvers = team.overs;
    let newOvers = currentOvers + value;
    
    // Handle over completion (after 0.5 overs, increment to next over)
    if ((newOvers * 10) % 10 === 6) {
        newOvers = Math.floor(newOvers) + 1;
    }
    
    team.overs = parseFloat(newOvers.toFixed(1));
    
    // Send update to server with admin authentication
    fetch('http://localhost:8080/api/scores', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
            'Authorization': 'Basic ' + localStorage.getItem('adminAuth')
        },
        body: JSON.stringify(state)
    }).catch(error => console.error('Error:', error));

    updateDisplay();
}

// Reset scores
function resetScores() {
    if (!isAdmin) {
        alert('Please login as admin to reset scores.');
        window.location.href = '/login.html';
        return;
    }

    if (!confirm('Are you sure you want to reset all scores?')) {
        return;
    }
    
    state.team1 = {
        name: "Team 1",
        runs: 0,
        wickets: 0,
        overs: 0.0,
        extras: 0
    };
    
    state.team2 = {
        name: "Team 2",
        runs: 0,
        wickets: 0,
        overs: 0.0,
        extras: 0
    };
    
    // Send update to server with admin authentication
    fetch('http://localhost:8080/api/scores', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
            'Authorization': 'Basic ' + localStorage.getItem('adminAuth')
        },
        body: JSON.stringify(state)
    }).catch(error => console.error('Error:', error));

    updateDisplay();
}

// Swap team names
function swapTeams() {
    if (!isAdmin) {
        alert('Please login as admin to swap teams.');
        window.location.href = '/login.html';
        return;
    }

    const temp = state.team1.name;
    state.team1.name = state.team2.name;
    state.team2.name = temp;
    
    // Send update to server with admin authentication
    fetch('http://localhost:8080/api/scores', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
            'Authorization': 'Basic ' + localStorage.getItem('adminAuth')
        },
        body: JSON.stringify(state)
    }).catch(error => console.error('Error:', error));

    updateDisplay();
}

// Logout function
function logout() {
    localStorage.removeItem('adminAuth');
    isAdmin = false;
    updateControlsVisibility();
}

// Initial setup
document.addEventListener('DOMContentLoaded', () => {
    // Add login/logout links to the page
    const header = document.querySelector('header');
    const authLinks = document.createElement('div');
    authLinks.style.marginTop = '10px';
    authLinks.innerHTML = `
        <a href="/login.html" id="loginLink" style="color: white; text-decoration: none; margin-right: 20px;">Admin Login</a>
        <a href="#" id="logoutLink" style="color: white; text-decoration: none; display: none;" onclick="logout()">Logout</a>
    `;
    header.appendChild(authLinks);

    // Check admin status
    checkAdminStatus();

    // Fetch initial state from server
    fetch('http://localhost:8080/api/scores')
        .then(response => response.json())
        .then(data => {
            state = data;
            updateDisplay();
        })
        .catch(error => {
            console.error('Error fetching initial state:', error);
            updateDisplay(); // Fall back to default state
        });
}); 