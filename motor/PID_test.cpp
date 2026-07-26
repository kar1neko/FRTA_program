#include <iostream>
using namespace std;

#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
   

#define rep(i, c, n) for (int i = c; i < n; ++i)

class PIDcontroller {
    private:
        double kp_t, ki_t, kd_t; // P, I, D ゲイン
        double prev_err=0.0, integral=0.0;

    public:
        PIDcontroller(double kp, double ki, double kd)
            :kp_t(kp), ki_t(ki), kd_t(kd), prev_err(0.0), integral(0.0) {}

        double update(double target, double cur) {
            double err = target - cur;
            integral += err;
            double diff = err - prev_err;
            prev_err = err;
            double result = kp_t * err + ki_t * integral + kd_t * diff;
            return result;
        }
};

void PID_cal(PIDcontroller& pidcon, double& cur_val) {
    double target = 800;
    rep(i, 0, 200) {
        double output = pidcon.update(target, cur_val);
        cur_val += output;
        double result = pidcon.update(target, cur_val);
        double clapped_result = constrain(result, 0.0, 8.0);
        cout << "result: " << result << " " << " clapped: "  << clapped_result << "\n";
    }

}

//* test code
int main() {
    PIDcontroller pidcon(0.13, 0.01, 0.02);
    double cur_val=0;
    PID_cal(pidcon, cur_val);

}