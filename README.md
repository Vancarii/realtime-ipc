# IPC and Concurrency Program 🤝

A program that allows two running processes to communicate in real-time. Each process has 4 concurrent threads running for reading, writing, taking input, and displaying on the screen. The running processes can either be both on the same device with two running terminals or two separate devices.

## Getting Started

### How to run s-talk with 2 terminals:

### Terminal 1:
  1. make clean
  2. make
  3. ./s-talk 12345 127.0.0.1 33221

### Terminal 2:
  1. ./s-talk 33221 127.0.0.1 12345
