/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2014 Open Ephys

------------------------------------------------------------------

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "CrossingDetector.h"
#include "CrossingDetectorEditor.h"

CrossingDetector::CrossingDetector() : GenericProcessor("Crossing Detector"),
threshold(START_THRESH), direction(START_DIRECTION), inputChan(START_INPUT), eventChan(START_OUTPUT),
eventDuration(START_DURATION), timeout(START_TIMEOUT), fracPrev(START_FRAC_PREV), numPrev(START_NUM_PREV),
fracNext(START_FRAC_NEXT), numNext(START_NUM_NEXT), sampsToShutoff(-1), sampsToReenable(numPrev), shutoffChan(-1)
{}

CrossingDetector::~CrossingDetector() {}

bool CrossingDetector::hasEditor() const
{
    return true;
}

AudioProcessorEditor* CrossingDetector::createEditor()
{
    editor = new CrossingDetectorEditor(this);
    return editor;
}

void CrossingDetector::process(AudioSampleBuffer& buffer, MidiBuffer& events)
{
    // atomic field access
    int currChan = inputChan;
    int currNumPrev = numPrev;
    int currNumNext = numNext;

    if (currChan < 0 || currChan >= buffer.getNumChannels()) // (shouldn't really happen)
        return;

#ifdef SEND_PRECISE_TIME
    // get timestamp at start of processing
#ifdef _WIN32
    LARGE_INTEGER tsStartUnion;
    QueryPerformanceCounter(&tsStartUnion);
    long long tsStart = tsStartUnion.QuadPart;
#else
    chrono::steady_clock::time_point tsStart = chrono::steady_clock::now();
#endif // _WIN32
#endif // SEND_PRECISE_TIME

    int nSamples = getNumSamples(currChan);
    const float* rp = buffer.getReadPointer(currChan);

    // loop has two functions: detect crossings and turn on events for the end of the previous buffer and most of the current buffer,
    // or if an event is currently on, turn it off if it has been on for long enough.
    for (int i = -currNumNext + 1; i < nSamples; i++)
    {
        // if enabled, check whether to trigger an event (operates on [-currNumNext+1, nSamples - currNumNext] )
        if (i >= sampsToReenable && i <= nSamples - currNumNext && shouldTrigger(rp, nSamples, i, direction, currNumPrev, currNumNext))
        {
            // trigger event
            int eventTime = std::max(i, 0); // actual sample when event fires (start of current buffer if the crossing was in prev. buffer.)
			float eventLevel = rp[eventTime];
			uint8 dataSize;
			uint8* dataPtr;

            // construct the event's data field
#ifdef SEND_PRECISE_TIME
            // experimental buffer position adjustment
//            int sampleNumOffset = i - nSamples;
//            double fracOfSecondToAdd = ((float)sampleNumOffset) / settings.sampleRate;
            double fracOfSecondToAdd = 0;

#ifdef _WIN32
			// add timestamp using the Windows performance counter
            const static double pCountFreq = (double)[]{
                LARGE_INTEGER frequency;
                QueryPerformanceFrequency(&frequency);
                return frequency.QuadPart;
            }();

            long long ts = tsStart + (long long)(fracOfSecondToAdd * pCountFreq);

#define DSIZE sizeof(float) + sizeof(long long)
			dataSize = (uint8)DSIZE;
			uint8 data[DSIZE];
			std::memcpy(data, &eventLevel, sizeof(float));
			std::memcpy(data+sizeof(float), &ts, sizeof(long long));
			dataPtr = data;
#undef DSIZE
#else
			// add timestamp using std::chrono::steady_clock
            long long usToAdd = (long long)(fracOfSecondToAdd * pow(10, 6));
            chrono::steady_clock::time_point ts = tsStart + (chrono::microseconds)(usToAdd);
#define DSIZE sizeof(float) + sizeof(chrono::steady_clock::time_point)
			dataSize = (uint8)DSIZE;
			uint8 data[DSIZE];
			std::memcpy(data, &eventLevel, sizeof(float));
			std::memcpy(data+sizeof(float), &ts, sizeof(chrono::steady_clock::time_point));
			dataPtr = data;
#undef DSIZE
#endif // _WIN32
#else
            // data field is the level at the sample at which the event is enabled.
			dataSize = (uint8)sizeof(float);
			dataPtr = (uint8*)&eventLevel;
#endif // SEND_PRECISE_TIME

			addEvent(events, TTL, eventTime, 1, eventChan, dataSize, dataPtr);
            // schedule event turning off and timeout period ending
			sampsToShutoff = eventTime + eventDuration;
            sampsToReenable = eventTime + timeout;
        }
        // if not triggering, check whether event should be shut off (operates on [0, nSamples) )
        else if (i >= 0 && i == sampsToShutoff)
        {
            addEvent(events, TTL, i, 0, (shutoffChan != -1 ? shutoffChan : eventChan));
            shutoffChan = -1;
        }
    }

    if (sampsToShutoff >= nSamples)
        // shift so it is relative to the next buffer
        sampsToShutoff -= nSamples;
    else
        // no scheduled shutoff, so keep it at -1
        sampsToShutoff = -1;

    if (sampsToReenable >= -currNumNext)
        // shift so it is relative to the next buffer
        sampsToReenable -= nSamples;

    // save this buffer for the next execution
    lastBuffer.clearQuick();
    lastBuffer.addArray(rp, nSamples);
}

// all new values should be validated before this function is called!
void CrossingDetector::setParameter(int parameterIndex, float newValue)
{
    switch (parameterIndex)
    {
    case pThreshold:
        threshold = newValue;
        break;

    case pDirection:
        direction = (CrossingDirection)(int)newValue;
        break;

    case pInputChan:
        if (getNumInputs() > newValue)
            inputChan = (int)newValue;
        break;

    case pEventChan:
        shutoffChan = eventChan;
        eventChan = (int)newValue;
        break;

    case pEventDur:
        eventDuration = (int)newValue;
        break;

    case pTimeout:
        timeout = (int)newValue;
        break;

    case pNumPrev:
        numPrev = (int)newValue;
        sampsToReenable = numPrev;
        break;

    case pFracPrev:
        fracPrev = newValue;
        break;

    case pNumNext:
        numNext = (int)newValue;
        break;

    case pFracNext:
        fracNext = newValue;
        break;
    }
}

bool CrossingDetector::disable()
{
    // set this to numPrev so that we don't trigger on old data when we start again.
    sampsToReenable = numPrev;
    return true;
}

float CrossingDetector::getThreshold()
{
    return threshold;
}

int CrossingDetector::getEventDuration()
{
    return eventDuration;
}

int CrossingDetector::getTimeout()
{
    return timeout;
}

float CrossingDetector::getFracPrev()
{
    return fracPrev;
}

int CrossingDetector::getNumPrev()
{
    return numPrev;
}

float CrossingDetector::getFracNext()
{
    return fracNext;
}

int CrossingDetector::getNumNext()
{
    return numNext;
}

// private
bool CrossingDetector::shouldTrigger(const float* rpCurr, int nSamples, int t0, CrossingDirection dir, int nPrev, int nNext)
{
    if (dir == dNone)
        return false;

    if (dir == dPosOrNeg)
        return shouldTrigger(rpCurr, nSamples, t0, dPos, nPrev, nNext) || shouldTrigger(rpCurr, nSamples, t0, dNeg, nPrev, nNext);

    // atomic field access
    float currThresh = threshold;

    int minInd = t0 - nPrev;
    int maxInd = t0 + nNext - 1;

    // check whether we have enough data
    if (minInd < -lastBuffer.size() || maxInd >= nSamples)
        return false;

    const float* rpLast = lastBuffer.end();

// allow us to treat the previous and current buffers as one array
#define rp(x) ((x)>=0 ? rpCurr[(x)] : rpLast[(x)])

    int numPrevRequired = (int)ceil(nPrev * fracPrev);
    int numNextRequired = (int)ceil(nNext * fracNext);

    for (int i = minInd; i < t0 && numPrevRequired > 0; i++)
        if (dir == dPos ? rp(i) < currThresh : rp(i) > currThresh)
            numPrevRequired--;

    if (numPrevRequired == 0) // "prev" condition satisfied
    {
        for (int i = t0; i <= maxInd && numNextRequired > 0; i++)
            if (dir == dPos ? rp(i) > currThresh : rp(i) < currThresh)
                numNextRequired--;

        if (numNextRequired == 0) // "next" condition satisfied
            return true;
    }
    
    return false;

#undef rp
}