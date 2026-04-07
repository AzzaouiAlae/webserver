#include "AStrategy.hpp"

AStrategy::~AStrategy()
{}

AStrategy::AStrategy() {
    _status = eContinue;
    _isBusy = false;
}

int AStrategy::GetStatus() const { return _status; }

bool AStrategy::IsBusy() {
    return _isBusy;
}