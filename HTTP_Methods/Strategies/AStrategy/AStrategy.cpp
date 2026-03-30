#include "AStrategy.hpp"

AStrategy::~AStrategy()
{}

AStrategy::AStrategy() {
    _status = eContinue;
}

int AStrategy::GetStatus() const { return _status; }