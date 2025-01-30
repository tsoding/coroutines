module echo;

import std::io;
import std::net;
import coroutine;

fn void client(void *arg) {
    io::printfn("Client connected!");

    TcpSocket* client_sock = (TcpSocket*)arg;
    defer {
        client_sock.close()!!;
        free(client_sock);
    }

    char[] buf = mem::new_array(char, 1024);
    defer free(buf.ptr);

    while OUTER: (true) {
        char[] chunk;
        while READ: (true) {
            usz! n = client_sock.read(buf);
            if (catch nani = n) {
                // TODO: do not spin on reading the data
                case IoError.WOULD_BLOCK: coroutine::yield();
                default: nani?!!;
            } else {
                if (n == 0) break OUTER;
                chunk = buf[0:n];
                break READ;
            }
        }
        while WRITE: (chunk.len > 0) {
            usz! n = client_sock.write(chunk);
            if (catch nani = n) {
                // TODO: do not spin on writing the data
                case IoError.WOULD_BLOCK: coroutine::yield();
                default: nani?!!;
            } else {
                if (n == 0) break OUTER;
                chunk = chunk[n..];
            }
        }
    }
    io::printfn("Client disconnected");
}

fn void main() {
    coroutine::init();

    const String HOST = "localhost";
    const uint PORT = 6969;
    TcpServerSocket server_sock = tcp::listen(HOST, PORT, 69, REUSEADDR)!!;
    server_sock.sock.set_non_blocking(true)!!;

    io::printfn("Listening to %s:%d", HOST, PORT);
    while (true) {
        TcpSocket! client_sock = tcp::accept(&server_sock);
        if (catch nani = client_sock) {
            switch (os::socket_error()) {
                // TODO: do not spin on accepting connections
                case IoError.WOULD_BLOCK: coroutine::yield();
                default: nani?!!;
            }
        } else {
            client_sock.sock.set_non_blocking(true)!!;
            coroutine::go(&client, @clone(client_sock));
        }
    }
}
