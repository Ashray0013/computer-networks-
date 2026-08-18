# Computer Networks - TCP Client-Server Assignment 1

A TCP-based client-server C application where a client sends its name and a user-provided integer (1–100) to the server. The server logs the client's information, selects its own integer (1–100), computes the sum of both numbers, and sends back its name and integer. Both parties print the details and calculate the combined sum before gracefully closing the connection.

---

## Features

- **Socket Programming:** Direct implementation using POSIX TCP sockets (`socket`, `bind`, `listen`, `accept`, `connect`).
- **Data Exchange:** Transfers structured data (Name and Integer) over network sockets.
- **Computation:** Both client and server display:
  - Client Name
  - Server Name
  - Client Integer
  - Server Integer
  - Sum of both integers

---

## File Structure

```text
.
├── client.c    # TCP Client implementation
└── server.c    # TCP Server implementation
