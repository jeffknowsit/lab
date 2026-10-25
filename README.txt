Networking experiments reconstructed to match the supplied output screenshots (PDF: pages 1-4).

Exp 5 - TCP matrix client/server:
- Port 8080, as shown in the screenshots.
- Client prints "Connected to server!", asks for matrix order, prints generated matrix, then "Server identified matrix as: ..."
- Server prints waiting/client connected, received matrix, and result sent.
- The client deliberately generates valid upper-triangular, lower-triangular, and diagonal matrices depending on N: N=3 upper, N=4 lower, N=5 diagonal. Other sizes use upper-triangular. This reproduces the demonstrated matrix classifications rather than random arbitrary values.

Exp 6 - UDP sentence conversion:
- Port 9002, one request per server run as in the screenshot.
- Replaces the abbreviations listed in the source PDF. Example: "pls, I don't wanna cry." is not a full natural-language translation rule; the implemented substitutions cover the PDF's listed abbreviations. "pls" and "wanna" are not in the PDF's abbreviation list and are left unchanged.

Exp 7 - TCP multi-user chat:
- Port 5000, as shown in the screenshots.
- Client asks for a name, then messages. The server broadcasts messages to connected clients and echoes them to the sender.
- Uses select() so multiple clients can chat concurrently without threads or fork.

Exp 8 - UDP time server:
- Port 8080, as shown in the screenshots.
- Client sends a time request and prints "Current server time: YYYY-MM-DD HH:MM:SS".
- Server prints "UDP Time Server started on port 8080..." and "Time sent to client: ...".

Build each experiment from its folder:
  gcc client.c -o client
  gcc server.c -o server

Run server first in one terminal, then client in another. Exp 7: start one server and multiple client terminals.
Each C source contains only the experiment title as its comment. These are reconstructed implementations matching the screenshots' observable behavior and ports, not exact recovery of source code from screenshots.
