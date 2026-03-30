#pragma once
#include "Headers.hpp"

class AStrategy
{
protected:
    int _status;
public:
    enum StrategyStatus {
        eOpenFileError = -5,
        eChunkedError = -4,
        eMultipartError = -3,
        eReadError = -2,
        eWriteError = -1,
        eComplete = 0,
        eContinue = 1,
    };
    int GetStatus() const;
    AStrategy();
    virtual ~AStrategy();
    virtual int Execute() = 0;
};