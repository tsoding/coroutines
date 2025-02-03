module echo;

import std::io;
import std::net;
import coroutine;

bool quit = false;
usz server_id = 0;

fn void main() {
    coroutine::init();

    server_id = coroutine::id();

    const String HOST = "localhost";
    const uint PORT = 6969;
    TcpServerSocket server = tcp::listen(HOST, PORT, 69, REUSEADDR)!!;
    server.sock.set_non_blocking(true)!!;

    io::printfn("[%d] Server listening to %s:%d", coroutine::id(), HOST, PORT);
    while SERVER: (true) {
        coroutine::sleep_read(server.sock);
        if (quit) break SERVER;
        TcpSocket client = tcp::accept(&server)!!;
        client.sock.set_non_blocking(true)!!;
        coroutine::go(fn void(void *arg) {
            io::printfn("[%d] Client connected!", coroutine::id());

            TcpSocket* client = (TcpSocket*)arg;
            char[] buf = mem::new_array(char, 1024);
            defer {
                client.close()!!;
                free(client);
                free(buf.ptr);
            }

            while OUTER: (true) {
                coroutine::sleep_read(client.sock);
                usz n = client.read(buf)!!;
                if (n == 0) break OUTER;
                char[] chunk = buf[0:n];

                switch (((String)chunk).trim()) {
                    case "quit":
                        io::printfn("[%d] Client requested to quit", coroutine::id(), chunk.len);
                        return;
                    case "shutdown":
                        io::printfn("[%d] Client requested to shutdown the server", coroutine::id());
                        quit = true;
                        coroutine::wake_up(server_id);
                        return;
                }

                io::printfn("[%d] Client sent %d bytes", coroutine::id(), chunk.len);

                while (chunk.len > 0) {
                    coroutine::sleep_write(client.sock);
                    usz m = client.write(chunk)!!;
                    if (m == 0) break OUTER;
                    chunk = chunk[m..];
                }
            }
            io::printfn("[%d] Client disconnected", coroutine::id());
        }, @clone(client));
    }
    io::printfn("[%d] Server has been shutdown", coroutine::id());
}
