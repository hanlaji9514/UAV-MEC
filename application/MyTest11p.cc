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

#include "veins/modules/application/traci/MyTest11p.h"

#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"

#include "veins/modules/messages/DemoSafetyMessage_m.h"

#include "veins/modules/messages/BeaconMessage_m.h"

using namespace veins;

Define_Module(veins::MyTest11p);

resource node_resource(100,100);
std::map<LAddress::L2Type, double> UAV_map;

void MyTest11p::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);
    if (stage == 0) {
        sentMessage = false;
        lastDroveAt = simTime();
        currentSubscribedServiceId = -1;
    }
    else if(stage == 1)
    {
        cMessage *taskMsg = new cMessage("generate_task");
        scheduleAt(simTime() + uniform(0.1 , 5.0), taskMsg);
        cMessage *resourceMsg = new cMessage("check_resource");
        scheduleAt(simTime() + 0.1, resourceMsg);
        cMessage *UAV_mapTimer = new cMessage("UAV_map");
        scheduleAt(simTime() + 2, UAV_mapTimer);
        TraCIDemo11pMessage *test = new TraCIDemo11pMessage;
        populateWSM(test);
        test->setTimestamp(simTime());
        test->setByteLength(1000);
        test->setSenderAddress(myId);
        sendDown(test);
    }
}

void MyTest11p::onWSA(DemoServiceAdvertisment* wsa)
{
    if (currentSubscribedServiceId == -1) {
        mac->changeServiceChannel(static_cast<Channel>(wsa->getTargetChannel()));
        currentSubscribedServiceId = wsa->getPsid();
        if (currentOfferedServiceId != wsa->getPsid()) {
            stopService();
            startService(static_cast<Channel>(wsa->getTargetChannel()), wsa->getPsid(), "Mirrored Traffic Service");
        }
    }
}

void MyTest11p::onWSM(BaseFrame1609_4* frame)
{
    TraCIDemo11pMessage* wsm = check_and_cast<TraCIDemo11pMessage*>(frame);

    findHost()->getDisplayString().setTagArg("i", 1, "green");

    EV_INFO << myId << ": Receive a packet from: " << wsm->getSenderAddress() << " at time: " << wsm->getTimestamp() << " And the data: " << wsm->getDemoData() << "Delay = " << simTime() - wsm->getTimestamp();

    /*if (mobility->getRoadId()[0] != ':') traciVehicle->changeRoute(wsm->getDemoData(), 9999);
    if (!sentMessage) {
        sentMessage = true;
        // repeat the received traffic update once in 2 seconds plus some random delay
        wsm->setSenderAddress(myId);//myId是該node的網卡（nic)的id
        wsm->setSerial(3);
        scheduleAt(simTime() + 2 + uniform(0.01, 0.2), wsm->dup());
    }*/
}

void MyTest11p::onBSM(DemoSafetyMessage* wsm)
{

}

void MyTest11p::onBM(BeaconMessage* bsm)
{
    BeaconMessage* bm = check_and_cast<BeaconMessage*>(bsm);
    //findHost()->getDisplayString().setTagArg("i", 1, "blue");
    EV_INFO << myId << ": I Receive a beacon from UAV " << bsm->getSenderAddress() << " and it's position : " << bsm->getSenderPos() << " / speed : " << bsm->getSenderSpeed() << " / direction : " << bsm->getSenderDirection();
    UAV_map.insert(std::pair<LAddress::L2Type, double>(bsm->getSenderAddress(), simTime().dbl()));
}

void MyTest11p::handleSelfMsg(cMessage* msg)
{
    if(!strcmp(msg->getName(), "generate_task"))
    {
        int numtasks = intuniform(1,5);
        for(int i=0; i<numtasks; i++)
        {
            int task_p =  intuniform(1,100);
            task *t;
            if (task_p >= 1 && task_p <= 20) // 使用if-else來判斷範圍
            {
                t = new task(1);
                //pending_task.push_back(t);
            }
            else if (task_p >= 21 && task_p <= 50)
            {
                t = new task(2);
                //pending_task.push_back(t);
            }
            else if (task_p >= 51 && task_p <= 75)
            {
                t = new task(3);
                //pending_task.push_back(t);
            }
            else if (task_p >= 76 && task_p <= 100)
            {
                t = new task(4);
                //pending_task.push_back(t);
            }
            t->start_time = simTime().dbl();
            t->expire_time = t->start_time + t->delay_limit;
            EV << "I'm " << myId << " and I generate a task: QoS = " << t->qos << " , Delay_limit : " << t->delay_limit <<  " , start_time = " << t->start_time << " , expire_time = " << t->expire_time << " , size = " << t->packet_size << endl;
        }

        cMessage *taskMsg = new cMessage("generate_task");
        scheduleAt(simTime() + uniform(0.1 , 5.0), taskMsg);
    }
    else if(!strcmp(msg->getName(), "check_resource"))
    {
        EV << "I'm " << myId << " and my remained cpu = " << node_resource.remain_cpu << ", remained memory = " << node_resource.remain_memory << endl;
        cMessage *resourceMsg = new cMessage("check_resource");
        scheduleAt(simTime() + 0.1, resourceMsg);
    }
    else if(!strcmp(msg->getName(), "UAV_map"))
    {
        if(!UAV_map.empty())
        {
            for(std::map<LAddress::L2Type, double>::iterator it = UAV_map.begin(); it != UAV_map.end();)
            {
                //EV << myId << " : UAV = " << (*it).first << " / time = " << (*it).second;
                if(simTime().dbl() >= (*it).second + 3.0)
                {
                    EV << "I'm " << myId << ", UAV " << (*it).first << " is no longer in my area!";
                    it = UAV_map.erase(it);
                }
                else
                {
                    it++;
                }
            }
        }
        cMessage *UAV_mapTimer = new cMessage("UAV_map");
        scheduleAt(simTime() + 2, UAV_mapTimer);
    }
}

void MyTest11p::handlePositionUpdate(cObject* obj)
{
    DemoBaseApplLayer::handlePositionUpdate(obj);

    /*
    // stopped for for at least 10s?
    if (mobility->getSpeed() < 1) {
        if (simTime() - lastDroveAt >= 10 && sentMessage == false) {
            findHost()->getDisplayString().setTagArg("i", 1, "red");
            sentMessage = true;

            TraCIDemo11pMessage* wsm = new TraCIDemo11pMessage();
            populateWSM(wsm);
            wsm->setDemoData(mobility->getRoadId().c_str());
            wsm->setTimestamp(simTime());
            wsm->setSenderAddress(myId);
            EV << "I'm " << myId << "and I sent a packet: " << wsm->getDemoData();
            // host is standing still due to crash
            if (dataOnSch) {
                startService(Channel::sch2, 42, "Traffic Information Service");
                // started service and server advertising, schedule message to self to send later
                scheduleAt(computeAsynchronousSendingTime(1, ChannelType::service), wsm);
            }
            else {
                // send right away on CCH, because channel switching is disabled
                sendDown(wsm);
            }
        }
    }
    else {
        lastDroveAt = simTime();
    }*/
}
