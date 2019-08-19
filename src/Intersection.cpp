#include <iostream>
#include <thread>
#include <chrono>
#include <future>
#include <random>

#include "Street.h"
#include "Intersection.h"
#include "Vehicle.h"

int WaitingVehicles::getSize()
{
    return _vehicles.size();
}

void WaitingVehicles::pushBack(std::shared_ptr<Vehicle> vehicle, std::promise<void> &&promise)
{
    _vehicles.push_back(vehicle);
    _promises.push_back(std::move(promise));
}

void WaitingVehicles::permitEntryToFirstInQueue()
{
    auto prms_01  = _promises.begin();
    auto vehicle_01 = _vehicles.begin();

    prms_01->set_value();

    _promises.erase(prms_01);
    _vehicles.erase(vehicle_01);
}

Intersection::Intersection()
{
    _type = ObjectType::objectIntersection;
    _isBlocked = false;
}

void Intersection::simulate()
{
    threads.emplace_back(std::thread(&Intersection::processVehicleQueue, this));
}

void Intersection::processVehicleQueue()
{
    
    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if(_waitingVehicles.getSize() > 0 && !_isBlocked)
        {
            this->setIsBlocked(true);

            _waitingVehicles.permitEntryToFirstInQueue();
        }
    }
}

void Intersection::addStreet(std::shared_ptr<Street> street)
{
    _streets.push_back(street);
}

std::vector<std::shared_ptr<Street>> Intersection::queryStreets(std::shared_ptr<Street> incoming)
{
    // store all outgoing streets in a vector ...
    std::vector<std::shared_ptr<Street>> outgoings;
    for (auto it : _streets)
    {
        if (incoming->getID() != it->getID()) // ... except the street making the inquiry
        {
            outgoings.push_back(it);
        }
    }

    return outgoings;
}

void Intersection::addVehicleToQueue(std::shared_ptr<Vehicle> vehicle)
{
    std::cout << "Intersection #" << _id << "::addVehicleToQueue: thread id = " << std::this_thread::get_id() << std::endl;

    std::promise<void> prms;
    std::future<void> ftr = prms.get_future();
    _waitingVehicles.pushBack(vehicle, std::move(prms));

    ftr.get();

    std::cout << "Intersection #" << _id << ": Vehicle #" << vehicle->getID() << " is granted entry" << std::endl;

}

void Intersection::vehicleHasLeft(std::shared_ptr<Vehicle> vehicle)
{
    std::cout << "Intersection #" << _id << ": Vehicle #" << vehicle->getID() << " has left." << std::endl;

    this->setIsBlocked(false);
}

void Intersection::setIsBlocked(bool isBlocked)
{
    this->_isBlocked = isBlocked;
}