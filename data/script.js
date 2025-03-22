let deviceStates = new Array(4).fill(false);
const statusSpan = document.getElementById('connection-status');
let wsConnection = null;

function initWebSocket() {
    wsConnection = new WebSocket(`ws://${window.location.hostname}/ws`);
    wsConnection.onopen = onOpen;
    wsConnection.onclose = onClose;
    wsConnection.onmessage = onMessage;
}

function onOpen() {
    statusSpan.textContent = 'Connected';
    statusSpan.style.color = '#4CAF50';
}

function onClose() {
    statusSpan.textContent = 'Disconnected';
    statusSpan.style.color = '#f44336';
    setTimeout(initWebSocket, 2000);
}

function onMessage(event) {
    const data = JSON.parse(event.data);
    if (data.type === 'states') {
        updateDeviceStates(data.states);
    }
}

function updateDeviceStates(states) {
    deviceStates = states;
    states.forEach((state, index) => {
        const button = document.querySelector(`button[data-relay="${index}"]`);
        if (state) {
            button.classList.add('active');
        } else {
            button.classList.remove('active');
        }
    });
}

function toggleDevice(index) {
    if (wsConnection && wsConnection.readyState === WebSocket.OPEN) {
        const message = {
            type: 'toggle',
            relay: index
        };
        wsConnection.send(JSON.stringify(message));
    }
}

// Initialize connection when page loads
window.addEventListener('load', initWebSocket);
