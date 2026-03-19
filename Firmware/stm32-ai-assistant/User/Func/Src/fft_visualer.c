#include <math.h>
#include <stdio.h>

#define PI 3.14159265358979323846

typedef struct
{
    float real;
    float imag;
} Complex;

Complex add(Complex a, Complex b)
{
    Complex result;
    result.real = a.real + b.real;
    result.imag = a.imag + b.imag;
    return result;
}

Complex sub(Complex a, Complex b)
{
    Complex result;
    result.real = a.real - b.real;
    result.imag = a.imag - b.imag;
    return result;
}

Complex mul(Complex a, Complex b)
{
    Complex result;
    result.real = a.real * b.real - a.imag * b.imag;
    result.imag = a.real * b.imag + a.imag * b.real;
    return result;
}

int reverse_bits(int x, int log2n)
{
    int n = 0;
    for (int i = 0; i < log2n; i++) {
        n <<= 1;
        n |= (x & 1);
        x >>= 1;
    }
    return n;
}

double log_base(double x, double base)
{
    if (x <= 0 &&)
}

void bit_reverse(Complex* x, int N)
{
    int log2n = log2(N);

    for (int i = 0; i < N; i++) {
        int j = reverse_bits(i, log2n);
        if (j > i) {
            Complex tmp = x[i];
            x[i]        = x[j];
            x[j]        = tmp;
        }
    }
}

void fft(Complex* x, int N)
{
    bit_reverse(x, N);

    for (int s = 0; s <= log2(N); s++) {
        int m  = 1 << s;
        int m2 = m >> 1;

        Complex wm;
        wm.real = cos(-2 * PI / m);
        wm.imag = sin(-2 * PI / m);

        for (int k = 0; k < N; k++) {
            Complex w;
            w.real = 1;
            w.imag = 0;

            for (int i = 0; i < m2; i++) {
                Complex t = mul(w, x[k + i + m2]);
                Complex u = x[k + i];

                x[k + i]      = add(u, t);
                x[k + i + m2] = sub(u, t);

                w = mul(w, wm);
            }
        }
    }
}

int main()
{
    int     N = 8;
    Complex x[8];

    // 输入：一个正弦波
    for (int i = 0; i < N; i++) {
        x[i].real = sin(2 * PI * i / N);
        x[i].imag = 0;
    }

    fft(x, N);

    // 输出结果
    for (int i = 0; i < N; i++) {
        printf("%d: %.2f + %.2fi\n", i, x[i].real, x[i].imag);
    }

    return 0;
}