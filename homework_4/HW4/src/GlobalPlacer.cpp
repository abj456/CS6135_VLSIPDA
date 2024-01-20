#include "GlobalPlacer.h"
#include "ExampleFunction.h"
#include "NumericalOptimizer.h"
#include <cstdlib>
#include <iostream>
#include <bits/stdc++.h>


GlobalPlacer::GlobalPlacer(wrapper::Placement& placement)
    : _placement(placement)
{
    bin_cut = 18;
    bound_width = _placement.boundryRight() - _placement.boundryLeft();
    bound_height = _placement.boundryTop() - _placement.boundryBottom();
    nets_num = _placement.numNets();
    bin_width = bound_width / bin_cut;
    bin_height = bound_height / bin_cut;

    double max_density = 0.0;
    for (unsigned i = 0; i < _placement.numModules(); i++) {
        max_density += _placement.module(i).area();
    }
    max_density /= (bound_width * bound_height);
    bin_max_area = max_density * bin_width * bin_height;

    module_isPlaced.assign(_placement.numModules(), false);
}

void GlobalPlacer::randomPlace()
{
    srand(0);
    
    double coreWidth = _placement.boundryRight() - _placement.boundryLeft();
    double coreHeight = _placement.boundryTop() - _placement.boundryBottom();
    for (size_t i = 0; i < _placement.numModules(); ++i)
    {
        if (_placement.module(i).isFixed())
            continue;

        double width = _placement.module(i).width();
        double height = _placement.module(i).height();
        double x = rand() % (int)(coreWidth - width) + _placement.boundryLeft();
        double y = rand() % (int)(coreHeight - height) + _placement.boundryBottom();
        _placement.module(i).setPosition(x, y);
    }
}

void GlobalPlacer::net_place(unsigned seed) {
    // random_device rd;
    // mt19937 g(rd());
    vector<int> net_place_order(nets_num);
    for (unsigned i = 0; i < nets_num; i++) {
        net_place_order[i] = i;
    }

    srand(seed);
    // shuffle(net_place_order.begin(), net_place_order.end(), seed);
    random_shuffle(net_place_order.begin(), net_place_order.end());

    int bin_cnt = 0;
    double cur_bin_area = 0.0;
    for (auto i : net_place_order) {
        for (unsigned j = 0; j < _placement.net(i).numPins(); j++) {

            int cur_module_id = _placement.net(i).pin(j).moduleId();
            if (_placement.module(cur_module_id).isFixed() || module_isPlaced[cur_module_id]) {
                continue;
            }

            double  module_w = _placement.module(cur_module_id).width(),
                module_h = _placement.module(cur_module_id).height();

            if (cur_bin_area + _placement.module(cur_module_id).area() > bin_max_area) {
                cur_bin_area = 0;
                bin_cnt++;
            }

            double x_coor = (bin_cnt % bin_cut) * bin_width + rand() % (int)(module_w)+_placement.boundryLeft();
            double y_coor = (bin_cnt / bin_cut) * bin_height + rand() % (int)(module_h)+_placement.boundryBottom();
            _placement.module(cur_module_id).setPosition(x_coor, y_coor);

            // x[2 * cur_module_id] = _placement.module(cur_module_id).x();
            // x[2 * cur_module_id + 1] = _placement.module(cur_module_id).y();
            // x[2 * cur_module_id] = _placement.module(cur_module_id).centerX();
            // x[2 * cur_module_id + 1] = _placement.module(cur_module_id).centerY();
            

            module_isPlaced[cur_module_id] = true;
            cur_bin_area += _placement.module(cur_module_id).area();
        }
    }

    for (unsigned i = 0; i < _placement.numModules(); i++) {
        module_isPlaced[i] = false;
    }

}

void GlobalPlacer::findBestInitPlace(int try_times) {
    double BestHPWL = 0.0, curHPWL = 0.0;
    unsigned BestSeed = 0, seed = 0;

    // int try_times = 2500;
    for (int i = 0; i < try_times; i++) {
        random_device rd;
        seed = rd();
        // mt19937 g(rd());

        net_place(seed);
        curHPWL = _placement.computeHpwl();

        if (curHPWL < BestHPWL || i == 0) {
            BestHPWL = curHPWL;
            BestSeed = seed;
            cout << "Debug: curHPWL = " << curHPWL << endl;
        }
        if (i == try_times - 1) {
            net_place(BestSeed);
            curHPWL = _placement.computeHpwl();
            // cout << "Debug: computeHPWL() = " << curHPWL << ", BestHPWL = " << BestHPWL << endl;
        }
    }

}

void GlobalPlacer::place()
{
    ///////////////////////////////////////////////////////////////////
    // The following example is only for analytical methods.
    // if you use other methods, you can skip and delete it directly.
    //////////////////////////////////////////////////////////////////
    const double boundary_left = _placement.boundryLeft();
    const double boundary_right = _placement.boundryRight();
    const double boundary_bottom = _placement.boundryBottom();
    const double boundary_top = _placement.boundryTop();

    ExampleFunction ef(_placement, *this); // require to define the object function and gradient function

    vector<double> x(ef.dimension()); // solution vector, size: num_blocks*2
    // each 2 variables represent the X and Y dimensions of a block
    // x[0] = 100;          // initialize the solution vector
    // x[1] = 100;
    // randomPlace();
    net_place(0);
    // findBestInitPlace(4000);

    for(unsigned module_id = 0; module_id < _placement.numModules(); module_id++){
        x[2 * module_id] = _placement.module(module_id).centerX();
        x[2 * module_id + 1] = _placement.module(module_id).centerY();
    }


    NumericalOptimizer no(ef);
    double bound_width = boundary_right - boundary_left;
    for (int i = 0; i < 3; i++) {
        cout << "Debug: solve iteration " << i + 1 << endl;
        no.setX(x);             // set initial solution
        no.setStepSizeBound(bound_width * 5); // user-specified parameter
        // no.setStepSizeBound((i == 0) ? bound_width * 2.5 : bound_width * 5); // user-specified parameter
        no.setNumIteration((i == 0) ? 150 : 35); // user-specified parameter
        no.solve();             // Conjugate Gradient solver
        ef.increase_lambda(10000);

        for (unsigned j = 0; j < _placement.numModules(); j++) {
            const double m_w = _placement.module(j).width() / 2;
            const double m_h = _placement.module(j).height() / 2;
            // const double m_w = _placement.module(j).width();
            // const double m_h = _placement.module(j).height();
            
            double m_x = no.x(2 * j);
            double m_y = no.x(2 * j + 1);

            if (!_placement.module(j).isFixed()) {
                // m_x = (m_x + m_w > boundary_right) ? (boundary_right - m_w)
                //     : (m_x - m_w < boundary_left) ? boundary_left : m_x;
                m_x = (m_x + m_w > boundary_right) ? (boundary_right - m_w)
                    : (m_x - m_w < boundary_left) ? (boundary_left + m_w) : m_x;

                // m_y = (m_y + m_h > boundary_top) ? (boundary_top - m_h)
                //     : (m_y - m_h < boundary_bottom) ? (boundary_bottom) : m_y;
                m_y = (m_y + m_h > boundary_top) ? (boundary_top - m_h)
                    : (m_y - m_h < boundary_bottom) ? (boundary_bottom + m_h) : m_y;
            }

            // _placement.module(j).setPosition(m_x - m_w, m_y - m_h);

            x[2 * j] = m_x;
            x[2 * j + 1] = m_y;
            
            // _placement.module(j).setPosition(m_x, m_y);
            _placement.module(j).setPosition(m_x - m_w, m_y - m_h);
            // _placement.module(j).setCenterPosition(m_x, m_y);
        }
        // no.setX(x);
    }

    // cout << "Current solution:\n";
    // for (unsigned i = 0; i < no.dimension(); i++)
    // {
    //     cout << "x[" << i << "] = " << no.x(i) << "\n";
    // }
    cout << "Objective: " << no.objective() << "\n";
    ////////////////////////////////////////////////////////////////

    // An example of random placement implemented by TA.
    // If you want to use it, please uncomment the folllwing 1 line.
    // randomPlace(x);
    // net_place(x);

    /* @@@ TODO
     * 1. Understand above example and modify ExampleFunction.cpp to implement the analytical placement
     * 2. You can choose LSE or WA as the wirelength model, the former is easier to calculate the gradient
     * 3. For the bin density model, you could refer to the lecture notes
     * 4. You should first calculate the form of wirelength model and bin density model and the forms of their gradients ON YOUR OWN
     * 5. Replace the value of f in evaluateF() by the form like "f = alpha*WL() + beta*BinDensity()"
     * 6. Replace the form of g[] in evaluateG() by the form like "g = grad(WL()) + grad(BinDensity())"
     * 7. Set the initial vector x in place(), set step size, set #iteration, and call the solver like above example
     * */
}
