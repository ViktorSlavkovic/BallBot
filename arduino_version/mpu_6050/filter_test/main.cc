#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <ctime>

using namespace std;

const double dt = 0.0001;
const double tmax = 10;
const double freq = 50;
const double pi = 3.14159265358979;
const double drift = 0.05;
const double drift_down_percent = 0.75;
const double drift_up_percent = 1 - drift_down_percent;
const double drift_up = drift * drift_up_percent;
const double drift_down = drift * drift_down_percent;

double f(double t) {
  return sin(2 * pi * freq * t);
}

int main() {
  FILE* real = fopen("real.csv", "w");
  FILE* gyro = fopen("gyro.csv", "w");
  FILE* accel = fopen("accel.csv", "w");

  double lin_a = 0;
  double ang_v = 0;
  double f0 = 0;
  double f1;
  double f_gyro = 0;
  srand(time(0));

  for (double t = 0; t < tmax; t += dt) {
    //double T = 1 / freq;
    //double x = t / T - floor(t / T);
    //double alpha = 2 * pi * freq * t;
    f1 = f(t);
    ang_v = (f1 - f0) / dt * (1 + ((double)rand() / RAND_MAX * drift - (f1 > f0 ? drift_down : drift_up)));
    f_gyro += ang_v * dt;
    fprintf(real, "%.5f\t%.5f\n", t, f1);
    fprintf(gyro, "%.5f\t%.5f\n", t, f_gyro);
    f0 = f1;
  }

  fclose(real);
  fclose(gyro);
  fclose(accel);
  return 0;
}
