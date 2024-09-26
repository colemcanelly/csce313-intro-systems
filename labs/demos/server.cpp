#include "common.h"

using namespace std;

// typedef vector<string> string_vector;
// typedef vector<string_vector> string_vector2D;

// string_vector who {"The wolf", "The music", "An eagle", "The rain"};
// string_vector what {"played", "sits", "sleeps", "crashes"};
// string_vector how {"loudly", "quietly", "quickly", "slowly", "softly"};
// string_vector where {"through the forest.", "in the sky.", "at night.", "in the tree.", "at dinner."};

// string_vector2D mat {who, what, how, where};

int main () {
    mkfifo("client_read", 0666);
    int write_fd = open("client_read", O_WRONLY);
    mkfifo("server_read", 0666);
    int read_fd = open("server_read", O_RDONLY);

    char received[MAX_MESSAGE];
    int nbytes = read(read_fd, received, MAX_MESSAGE);

    char* to_write = new char[nbytes];
    for (int i = 0; i < nbytes; i++) {
        to_write[i] = received[nbytes - i - 1];
    }

    write(write_fd, to_write, nbytes);
    delete[] to_write;
    to_write = nullptr;
}
