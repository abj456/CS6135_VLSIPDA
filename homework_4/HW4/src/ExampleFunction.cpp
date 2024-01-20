#include "ExampleFunction.h"
#include <bits/stdc++.h>

// minimize 3*x^2 + 2*x*y + 2*y^2 + 7

ExampleFunction::ExampleFunction(wrapper::Placement& placement, GlobalPlacer& globalplacer)
    : _placement(placement), _globalplacer(globalplacer)
{
    bound_width = _placement.boundryRight() - _placement.boundryLeft();
    bound_height = _placement.boundryTop() - _placement.boundryBottom();
    core_area = bound_width * bound_height;
    num_modules = _placement.numModules();

    lambda = 0;
    eta = (bound_width + bound_height) / 100;
    // eta = 500;
    bin_cut = 18;

    grad.resize(num_modules * 2);
    x_exp.resize(num_modules * 4);

    bin_width = bound_width / bin_cut;
    bin_height = bound_height / bin_cut;
    bin_total_num = bin_cut * bin_cut;
    bin_area = bin_width * bin_height;

    avg_density = 0.0;
    double total_area = 0;
    for (unsigned i = 0; i < num_modules; i++) {
        total_area += _placement.module(i).area();
    }
    avg_density = total_area / core_area;
}

double ExampleFunction::calculateLSE_F(const vector<double>& x) {
    double total_LSE = 0.0;
    // x_exp.assign(x_exp.size(), 0);

    // LSE WL
    for (unsigned i = 0; i < num_modules; i++) {
        x_exp[4 * i] = exp(x[2 * i] / eta);
        x_exp[4 * i + 1] = exp(-x[2 * i] / eta);
        x_exp[4 * i + 2] = exp(x[2 * i + 1] / eta);
        x_exp[4 * i + 3] = exp(-x[2 * i + 1] / eta);
    }

    for (unsigned i = 0; i < _placement.numNets(); i++) {
        double sum_x1 = 0.0, sum_x2 = 0.0, sum_y1 = 0.0, sum_y2 = 0.0;

        for (unsigned j = 0; j < _placement.net(i).numPins(); j++) {
            const int module_id = _placement.net(i).pin(j).moduleId();
            sum_x1 += x_exp[4 * module_id];
            sum_x2 += x_exp[4 * module_id + 1];
            sum_y1 += x_exp[4 * module_id + 2];
            sum_y2 += x_exp[4 * module_id + 3];
        }

        total_LSE += (log(sum_x1) + log(sum_x2) + log(sum_y1) + log(sum_y2));
        // total_LSE += eta * (log(sum_x1) + log(sum_x2) + log(sum_y1) + log(sum_y2));
    }

    return eta * total_LSE;
    // return total_LSE;
}

double ExampleFunction::calculateLSE_FG(const vector<double>& x, vector<double>& g) {
    double total_LSE = 0.0;
    // x_exp.assign(x_exp.size(), 0);

    // LSE WL
    for (unsigned i = 0; i < num_modules; i++) {
        x_exp[4 * i] = exp(x[2 * i] / eta);
        x_exp[4 * i + 1] = exp(-x[2 * i] / eta);
        x_exp[4 * i + 2] = exp(x[2 * i + 1] / eta);
        x_exp[4 * i + 3] = exp(-x[2 * i + 1] / eta);
    }

    for (unsigned i = 0; i < _placement.numNets(); i++) {
        double sum_x1 = 0.0, sum_x2 = 0.0, sum_y1 = 0.0, sum_y2 = 0.0;

        for (unsigned j = 0; j < _placement.net(i).numPins(); j++) {
            int module_id = _placement.net(i).pin(j).moduleId();
            sum_x1 += x_exp[4 * module_id];
            sum_x2 += x_exp[4 * module_id + 1];
            sum_y1 += x_exp[4 * module_id + 2];
            sum_y2 += x_exp[4 * module_id + 3];
        }

        total_LSE += log(sum_x1) + log(sum_x2) + log(sum_y1) + log(sum_y2);
        // total_LSE += eta * (log(sum_x1) + log(sum_x2) + log(sum_y1) + log(sum_y2));

        for (unsigned j = 0; j < _placement.net(i).numPins(); j++) {
            int module_id = _placement.net(i).pin(j).moduleId();
            if (!_placement.module(module_id).isFixed()) {
                g[2 * module_id] += x_exp[4 * module_id] / (sum_x1);
                g[2 * module_id] -= x_exp[4 * module_id + 1] / (sum_x2);
                g[2 * module_id + 1] += x_exp[4 * module_id + 2] / (sum_y1);
                g[2 * module_id + 1] -= x_exp[4 * module_id + 3] / (sum_y2);

                // g[2 * module_id] += x_exp[4 * module_id] / (eta * sum_x1);
                // g[2 * module_id] -= x_exp[4 * module_id + 1] / (eta * sum_x2);
                // g[2 * module_id + 1] += x_exp[4 * module_id + 2] / (eta * sum_y1);
                // g[2 * module_id + 1] -= x_exp[4 * module_id + 3] / (eta * sum_y2);
            }
        }
    }

    return eta * total_LSE;
    // return total_LSE;
}


double ExampleFunction::calculateBinDensity_F(const vector<double>& x) {
    double total_bin_density = 0.0;
    if (lambda == 0) return total_bin_density;

    // bin density
    double boundary_left = _placement.boundryLeft(), boundary_bottom = _placement.boundryBottom();

    double m_w = 0.0, m_h = 0.0;
    double c = 0.0;
    double theta_x = 0.0, theta_y = 0.0, d_x = 0.0, d_y = 0.0, abs_d_x = 0.0, abs_d_y = 0.0, a_x = 0.0, b_x = 0.0, a_y = 0.0, b_y = 0.0;

    for (unsigned y_bin_id = 0; y_bin_id < bin_cut; y_bin_id++) {
        for (unsigned x_bin_id = 0; x_bin_id < bin_cut; x_bin_id++) {
            double bin_d = 0;

            for (unsigned i = 0; i < num_modules; i++) {
                m_w = _placement.module(i).width();
                m_h = _placement.module(i).height();

                if (!_placement.module(i).isFixed()) {
                    c = _placement.module(i).area() / bin_area;
                    // c = 1.0;

                    // double d_x = _placement.module(i).centerX() - (((double)x_bin_id + 0.5) * bin_width + boundary_left);
                    d_x = x[2 * i] - (((double)x_bin_id + 0.5) * bin_width + boundary_left);
                    abs_d_x = abs(d_x);

                    // double d_y = _placement.module(i).centerY() - (((double)y_bin_id + 0.5) * bin_height + boundary_bottom);
                    d_y = x[2 * i + 1] - (((double)y_bin_id + 0.5) * bin_height + boundary_bottom);
                    abs_d_y = abs(d_y);

                    a_x = 4 / ((bin_width + m_w) * (2 * bin_width + m_w));
                    b_x = 4 / (bin_width * (2 * bin_width + m_w));

                    a_y = 4 / ((bin_height + m_h) * (2 * bin_height + m_h));
                    b_y = 4 / (bin_height * (2 * bin_height + m_h));


                    theta_x = (abs_d_x <= bin_width / 2 + m_w / 2) ? (1 - a_x * pow(abs_d_x, 2))
                        : (abs_d_x <= bin_width + m_w / 2) ? (b_x * pow(abs_d_x - bin_width - m_w / 2, 2))
                        : 0;
                    theta_y = (abs_d_y <= bin_height / 2 + m_h / 2) ? (1 - a_y * pow(abs_d_y, 2))
                        : (abs_d_y <= bin_height + m_h / 2) ? (b_y * pow(abs_d_y - bin_height - m_h / 2, 2))
                        : 0;

                    bin_d += c * theta_x * theta_y;
                    // bin_d += theta_x * theta_y;
                }
            }

            // total_bin_density += lambda * pow(bin_d - avg_density, 2);
            total_bin_density += pow(bin_d - avg_density, 2);
            // total_bin_density += pow(bin_d - avg_density, 2);
        }
    }
    // return total_bin_density;
    return lambda * total_bin_density;
}

double ExampleFunction::calculateBinDensity_FG(const vector<double>& x, vector<double>& g) {
    double total_bin_density = 0.0;
    if (lambda == 0) return total_bin_density;

    // bin density
    double boundary_left = _placement.boundryLeft(), boundary_bottom = _placement.boundryBottom();
    grad.assign(g.size(), 0.0);

    double m_w = 0.0, m_h = 0.0;
    double c = 0.0;
    double theta_x = 0.0, theta_y = 0.0, d_x = 0.0, d_y = 0.0, abs_d_x = 0.0, abs_d_y = 0.0, a_x = 0.0, b_x = 0.0, a_y = 0.0, b_y = 0.0;

    for (unsigned y_bin_id = 0; y_bin_id < bin_cut; y_bin_id++) {
        for (unsigned x_bin_id = 0; x_bin_id < bin_cut; x_bin_id++) {
            // grad.assign(bin_total_num, 0.0);
            grad.assign(g.size(), 0.0);
            double bin_d = 0;

            for (unsigned i = 0; i < num_modules; i++) {
                m_w = _placement.module(i).width();
                m_h = _placement.module(i).height();

                if (!_placement.module(i).isFixed()) {
                    c = _placement.module(i).area() / bin_area;
                    // c = 1.0;

                    // double d_x = _placement.module(i).centerX() - (((double)x_bin_id + 0.5) * bin_width + boundary_left);
                    d_x = x[2 * i] - (((double)x_bin_id + 0.5) * bin_width + boundary_left);
                    abs_d_x = abs(d_x);
                    // double d_y = _placement.module(i).centerY() - (((double)y_bin_id + 0.5) * bin_height + boundary_bottom);
                    d_y = x[2 * i + 1] - (((double)y_bin_id + 0.5) * bin_height + boundary_bottom);
                    abs_d_y = abs(d_y);

                    a_x = 4 / ((bin_width + m_w) * (2 * bin_width + m_w));
                    b_x = 4 / (bin_width * (2 * bin_width + m_w));

                    a_y = 4 / ((bin_height + m_h) * (2 * bin_height + m_h));
                    b_y = 4 / (bin_height * (2 * bin_height + m_h));


                    theta_x = (abs_d_x <= m_w / 2 + bin_width / 2) ? (1 - a_x * pow(abs_d_x, 2))
                        : (abs_d_x <= m_w / 2 + bin_width) ? (b_x * pow(abs_d_x - bin_width - m_w / 2, 2))
                        : 0;
                    theta_y = (abs_d_y <= m_h / 2 + bin_height / 2) ? (1 - a_y * pow(abs_d_y, 2))
                        : (abs_d_y <= m_h / 2 + bin_height) ? (b_y * pow(abs_d_y - bin_height - m_h / 2, 2))
                        : 0;


                    bin_d += c * theta_x * theta_y;
                    // bin_d += theta_x * theta_y;

                    double sign_x = (d_x > 0) ? 1.0 : -1.0;
                    double sign_y = (d_y > 0) ? 1.0 : -1.0;
                    double theta_x_g = (abs_d_x <= m_w / 2 + bin_width / 2) ? (-2 * a_x * d_x)
                        : (abs_d_x <= m_w / 2 + bin_width) ? (2 * b_x * (d_x - bin_width - m_w / 2) * sign_x)
                        : 0;
                    double theta_y_g = (abs_d_y <= m_h / 2 + bin_height / 2) ? (-2 * a_y * d_y)
                        : (abs_d_y <= m_h / 2 + bin_height) ? (2 * b_y * (d_y - bin_height - m_h / 2) * sign_y)
                        : 0;

                    grad[2 * i] = theta_x_g * theta_y * c;
                    grad[2 * i + 1] = theta_x * theta_y_g * c;
                    // grad[2 * i] = theta_x_g * theta_y;
                    // grad[2 * i + 1] = theta_x * theta_y_g;

                }
            }

            // total_bin_density += lambda * pow(bin_d - avg_density, 2);
            total_bin_density += pow(bin_d - avg_density, 2);

            for (unsigned i = 0; i < num_modules; i++) {
                g[2 * i] += lambda * 2 * (bin_d - avg_density) * grad[2 * i];
                g[2 * i + 1] += lambda * 2 * (bin_d - avg_density) * grad[2 * i + 1];
                // g[2 * i] += lambda * 2 * bin_d * grad[2 * i];
                // g[2 * i + 1] += lambda * 2 * bin_d * grad[2 * i + 1];
            }
        }
    }

    // return total_bin_density;
    return lambda * total_bin_density;
}

void ExampleFunction::evaluateFG(const vector<double>& x, double& f, vector<double>& g)
{
    // f = 3 * x[0] * x[0] + 2 * x[0] * x[1] + 2 * x[1] * x[1] + 7; // objective function
    // g[0] = 6 * x[0] + 2 * x[1];                                  // gradient function of X
    // g[1] = 2 * x[0] + 4 * x[1];                                  // gradient function of Y

    g.assign(g.size(), 0.0);

    double LSE = calculateLSE_FG(x, g);
    double bin_density = calculateBinDensity_FG(x, g);

    f = LSE + bin_density;
}

void ExampleFunction::evaluateF(const vector<double>& x, double& f)
{
    // f = 3 * x[0] * x[0] + 2 * x[0] * x[1] + 2 * x[1] * x[1] + 7; // objective function
    // cout << "Debug: evaluateF is called\n";
    double LSE = calculateLSE_F(x);
    double bin_density = calculateBinDensity_F(x);

    f = LSE + bin_density;

}

void ExampleFunction::increase_lambda(int inc) {
    lambda += inc;
    cout << "Debug: lambda increase " << inc << ", now = " << lambda << endl;
}

unsigned ExampleFunction::dimension()
{
    return 2 * num_modules; // num_blocks*2
    // each two dimension represent the X and Y dimensions of each block
}
