#include "types.h"
#include "stat.h"
#include "user.h"

// Recursive Fibonacci function
int fib(int n) {
    if (n <= 0) return 0;
    if (n == 1) return 1;
    if (n == 2) return 1;
    return fib(n - 1) + fib(n - 2);
}

int main() {
    int i = 0;
    while (i<100) {
        printf(1, "This is normal code running\n");
        fib(35); // Doing CPU-intensive work
        i++;
    }
    exit();
}
