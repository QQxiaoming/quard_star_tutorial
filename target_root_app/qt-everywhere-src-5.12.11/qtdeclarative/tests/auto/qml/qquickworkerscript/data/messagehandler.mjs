
export function messageHandler(msg) {
    WorkerScript.sendMessage("Hello from the module")
}
