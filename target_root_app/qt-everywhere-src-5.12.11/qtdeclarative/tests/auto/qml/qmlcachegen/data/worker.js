WorkerScript.onMessage = function(message) {
    WorkerScript.sendMessage({ reply: message });
}
