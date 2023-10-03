//
// Copyright (C) 2006-2011 Christoph Sommer <christoph.sommer@uibk.ac.at>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#pragma once

#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include "veins/modules/messages/BeaconMessage_m.h"
#include <queue>
#include <math.h>
#include <random>
#include <ctime>

namespace veins {

/**
 * @brief
 * A tutorial demo for TraCI. When the car is stopped for longer than 10 seconds
 * it will send a message out to other cars containing the blocked road id.
 * Receiving cars will then trigger a reroute via TraCI.
 * When channel switching between SCH and CCH is enabled on the MAC, the message is
 * instead send out on a service channel following a Service Advertisement
 * on the CCH.
 *
 * @author Christoph Sommer : initial DemoApp
 * @author David Eckhoff : rewriting, moving functionality to DemoBaseApplLayer, adding WSA
 *
 */

struct task
{
    int require_cpu;
    int require_memory;
    int packet_size;
    double delay_limit;
    int qos;
    double start_time;
    double expire_time;


    task (int q)
    {
        qos = q;
        //EV << "Time : " << (unsigned int)time(NULL) << endl;
        srand((unsigned int)time(NULL));
        std::default_random_engine rnd_generator(rand());
        std::normal_distribution<double> normal_dis(50.0, 16.0); // Give a name to the object
        double rnd = normal_dis(rnd_generator); // Use parentheses
        if(rnd < 0)
            rnd = 0;
        else if(rnd > 100)
            rnd = 100;
        switch (q)
        {
            case 1:
                packet_size = 300 + (int)(700 * rnd / 100); // Priority 1 : packet size = 300 ~ 1000 Bytes
                delay_limit = (10.0 + 40.0 * rnd / 100) / 1000.0; // 10ms~50ms
                require_cpu = 10 + (int)(10 * rnd / 100);
                require_memory = 10 + (int)(10 * rnd / 100);
                break;
            case 2:
                packet_size = 100 + (int)(700 * rnd / 100); // Priority 2 : packet size = 100 ~ 800 Bytes
                delay_limit = 100.0 / 1000.0; // 100ms
                require_cpu = 10 + (int)(5 * rnd / 100);
                require_memory = 10 + (int)(5 * rnd / 100);
                break;
            case 3:

                delay_limit = 150.0 / 1000.0; // 150ms
                require_cpu = 10 + (int)(5 * rnd / 100);
                require_memory = 10 + (int)(5 * rnd / 100);
                break;
            case 4:
                packet_size = 1000 + (int)(5400 * rnd / 100); // Priority 4 : packet size = 1000 ~ 6400 Bytes
                delay_limit = 300.0 / 1000.0; // 300ms
                require_cpu = 5 + (int)(10 * rnd / 100);
                require_memory = 5 + (int)(5 * rnd / 100);
                break;
        }
    }
};

struct resource
{
    int remain_cpu;
    int remain_memory;
    std::queue<task> pending_tasks; //待處理之任務
    std::queue<task> handling_tasks; //正在被處理之任務

    resource (int c, int m)
    {
        remain_cpu = c;
        remain_memory = m;
    }
};

class VEINS_API MyTest11p : public DemoBaseApplLayer {
public:
    void initialize(int stage) override;

protected:
    simtime_t lastDroveAt;
    bool sentMessage;
    int currentSubscribedServiceId;

protected:
    void onWSM(BaseFrame1609_4* wsm) override;
    void onWSA(DemoServiceAdvertisment* wsa) override;
    void onBSM(DemoSafetyMessage* wsm) override;
    void onBM(BeaconMessage* bsm) override;

    void handleSelfMsg(cMessage* msg) override;
    void handlePositionUpdate(cObject* obj) override;
};

} // namespace veins
