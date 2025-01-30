module echo;

import std::io;
import std::net;
import coroutine;

fn void client(void *arg) {
    io::printfn("[%d] Client connected!", coroutine::id());

    TcpSocket* client_sock = (TcpSocket*)arg;
    char[] buf = mem::new_array(char, 1024);
    defer {
        client_sock.close()!!;
        free(client_sock);
        free(buf.ptr);
    }

    while OUTER: (true) {
        char[] chunk;

        while READ: (true) {
            usz! n = client_sock.read(buf);
            if (catch nani = n) {
                case IoError.WOULD_BLOCK:
                    coroutine::yield();
                    continue; // TODO: do not spin on reading the data
                default: nani?!!;
            }
            if (n == 0) break OUTER;

            chunk = buf[0:n];
            break READ;
        }

        io::printfn("[%d] Client sent %d bytes", coroutine::id(), chunk.len);

        while WRITE: (chunk.len > 0) {
            usz! n = client_sock.write(chunk);
            if (catch nani = n) {
                case IoError.WOULD_BLOCK:
                    coroutine::yield();
                    continue; // TODO: do not spin on writing the data
                default: nani?!!;
            }
            if (n == 0) break OUTER;

            chunk = chunk[n..];
        }
    }
    io::printfn("[%d] Client disconnected", coroutine::id());
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
                case IoError.WOULD_BLOCK:
                    coroutine::yield();
                    continue; // TODO: do not spin on accepting connections
                default: nani?!!;
            }
        }

        client_sock.sock.set_non_blocking(true)!!;
        coroutine::go(&client, @clone(client_sock));
    }
}
