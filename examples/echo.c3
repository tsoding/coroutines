module echo;

import std::io;
import std::net;
import coroutine;

fn void main() {
    coroutine::init();

    const String HOST = "localhost";
    const uint PORT = 6969;
    TcpServerSocket server_sock = tcp::listen(HOST, PORT, 69, REUSEADDR)!!;
    server_sock.sock.set_non_blocking(true)!!;

    io::printfn("[%d] Listening to %s:%d", coroutine::id(), HOST, PORT);
    while (true) {
        coroutine::sleep_read(server_sock.sock);
        TcpSocket client_sock = tcp::accept(&server_sock)!!;
        client_sock.sock.set_non_blocking(true)!!;
        coroutine::go(fn void(void *arg) {
            io::printfn("[%d] Client connected!", coroutine::id());

            TcpSocket* client_sock = (TcpSocket*)arg;
            char[] buf = mem::new_array(char, 1024);
            defer {
                client_sock.close()!!;
                free(client_sock);
                free(buf.ptr);
            }

            while OUTER: (true) {
                coroutine::sleep_read(client_sock.sock);
                usz n = client_sock.read(buf)!!;
                if (n == 0) break OUTER;
                char[] chunk = buf[0:n];
                io::printfn("[%d] Client sent %d bytes", coroutine::id(), chunk.len);

                while (chunk.len > 0) {
                    coroutine::sleep_write(client_sock.sock);
                    usz m = client_sock.write(chunk)!!;
                    if (m == 0) break OUTER;
                    chunk = chunk[m..];
                }
            }
            io::printfn("[%d] Client disconnected", coroutine::id());
        }, @clone(client_sock));
    }
}
