import socket

class TCPClient:
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.socket = None

    def connect(self):
        """Établit une connexion TCP avec le serveur."""
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.connect((self.host, self.port))
            print(f"Connecté à {self.host}:{self.port}")
            return True
        except socket.error as e:
            print(f"Erreur de connexion : {e}")
            return False

    def send_frame(self, frame_hex):
        """Envoie une trame hexadécimale au serveur connecté."""
        if self.socket is None:
            print("Erreur : Aucune connexion établie. Veuillez appeler la méthode 'connect' d'abord.")
            return None

        try:
            frame_binary = bytes.fromhex(frame_hex)
            self.socket.sendall(frame_binary)
            print(f"Trame envoyée (hex) : {frame_hex}")
            response = self.socket.recv(1024)
            response_hex = response.hex()
            print(f"Réponse reçue (hex) : {response_hex}")
            return response_hex
        except socket.error as e:
            print(f"Erreur lors de l'envoi/réception : {e}")
            return None

    def close(self):
        """Ferme la connexion TCP."""
        if self.socket:
            self.socket.close()
            print("Connexion fermée.")
            self.socket = None

# Informations de connexion
host = "127.0.0.1"
port = 4063

# Créer une instance du client TCP
client = TCPClient(host, port)



# Trames à envoyer
frames_to_send = [
    "000100100001001F601DA109060760857405080101BE10040E01000000065F1F040062FEDFFFFF",
    "000100100001000DC001C1000F0000280000FF0200", # Get-Request
    "0001001000010009C002C10000000151BE" # Get-Request Next
]

# Établir la connexion
if client.connect():
    # Envoyer chaque trame
    for frame in frames_to_send:
        response = client.send_frame(frame)
        if response:
            print("Traitement de la réponse...") # Vous pouvez ajouter ici le traitement spécifique de la réponse

    # Fermer la connexion après l'envoi de toutes les trames
    client.close()
