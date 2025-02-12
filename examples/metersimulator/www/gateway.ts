import net from 'net';
 import { WebSocketServer } from 'bun';

 // Configuration
 const PORT_WS = 3333;
 const PORT_TCP = 4444;

 let wsClient: WebSocket | null = null;
let tcpClient: net.Socket | null = null;

// ==========================================================================================
// WEBSOCKET SERVER
// ==========================================================================================

Bun.serve({
    port: PORT_WS,
    fetch(req, server) {
        if (server.upgrade(req)) {
            return;
        }
        return new Response("Upgrade failed", { status: 500 });
    },
    websocket: {
        open(ws) {
            if (wsClient) {
                ws.close(1000, "Another WebSocket client connected. Closing previous.");
                return;
            }
            console.log('Client WebSocket connecté');
            wsClient = ws;

            ws.onmessage = (message) => {
                if (tcpClient) {
                    tcpClient.write(message.data);
                }
            };

            ws.onclose = () => {
                console.log('Client WebSocket déconnecté');
                wsClient = null;
                if (tcpClient) {
                    tcpClient.destroy(); // Close TCP if WS closes
                    tcpClient = null;
                }
            };

            ws.onerror = (error) => {
                console.error('Erreur WebSocket :', error);
                wsClient = null;
                if (tcpClient) {
                    tcpClient.destroy();
                    tcpClient = null;
                }
            };
        },
        message() { }, // Handled in open
        close() { }, // Handled in open
        drain() { },
    },
});

console.log(`Serveur WebSocket en écoute sur le port ${PORT_WS}`);

// ==========================================================================================
// TCP SERVER
// ==========================================================================================

const serverTCP = net.createServer((socket) => {
    if (tcpClient) {
        socket.destroy(new Error("Another TCP client connected. Closing previous."));
        return;
    }

    console.log('Client TCP connecté');
    tcpClient = socket;

    socket.on('data', (data) => {
        if (wsClient && wsClient.readyState === WebSocket.OPEN) {
            wsClient.send(data);
        }
    });

    socket.on('end', () => {
        console.log('Client TCP déconnecté');
        tcpClient = null;
        if (wsClient) {
            wsClient.close(); // Close WS if TCP closes
            wsClient = null;
        }
    });

    socket.on('error', (error) => {
        console.error('Erreur TCP client :', error);
        tcpClient = null;
        if (wsClient) {
            wsClient.close();
            wsClient = null;
        }
    });
});

serverTCP.listen(PORT_TCP, () => {
    console.log(`Serveur TCP en écoute sur le port ${PORT_TCP}`);
});

serverTCP.on('error', (error) => {
    console.error('Erreur serveur TCP :', error);
});