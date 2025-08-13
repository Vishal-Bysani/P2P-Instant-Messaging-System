# P2P-Instant-Messaging-System

## 📌 Overview
**P2PText** is a peer-to-peer text messaging system with a central **Tracker Server** that helps peers discover each other.  
Peers can:
- **Register** with the tracker to obtain a listening port.
- **Store and reload** contacts from disk.
- **Send and receive** messages directly without routing through the tracker.

The tracker acts **only** as a directory service — it does not store or forward messages.

---

## ⚙️ Components

### **1. Tracker Server (`tracker.cpp`)**
- Maintains a **directory of connected peers** (`name`, `IP`, `port`).
- Supports:
  - **Registration (`R`)** → Assigns a port to a new peer.
  - **Peer lookup (`G`)** → Returns IP and port of a given peer name.
- Runs on **multiple ports** (from `available_ports` in `config.hpp`).
- Thread-safe using `pthread_mutex_t`.

---

### **2. Peer Client (`peer.cpp`)**
Runs **two threads**:
- **Receiving Thread**
  - Opens a TCP socket and listens for incoming messages.
  - Prints `[ sender_name ]: message` to the console.
- **Sending Thread**
  - Registers the peer or loads previous session contacts from disk.
  - Queries tracker for other peers’ details.
  - Sends messages directly via TCP.

---


## 🚀 Getting Started

### 1️⃣ Compile
```bash
make
```

### 2️⃣ Run Tracker
```bash
./tracker
```

### 3️⃣ Run Peer
```bash
./peer <peer_name>
```

---

## 📄 Configuration
The `config.hpp` file contains:
- **Tracker IP** and **port list**
- **Peer port range**

---

## 📚 Example
```
# Terminal 1 (Tracker)
./tracker

# Terminal 2 (Peer Alice)
./peer Alice

# Terminal 3 (Peer Bob)
./peer Bob

# In Alice's terminal:
> G Bob
> Hello Bob!

# Bob receives:
[ Alice ]: Hello Bob!
```

---

## 🔒 Notes
- Tracker does **not** store messages — only peer addresses.
- Peers must be online to receive messages.
- Contacts persist locally in a file for reconnection.

---