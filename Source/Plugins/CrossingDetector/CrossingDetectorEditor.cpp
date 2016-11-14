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

#include "CrossingDetectorEditor.h"
#include "CrossingDetector.h"

// local utility
namespace {

    /* Attempts to parse the current text of a label as an int between min and max inclusive.
    *  If successful, sets "*out" and the label text to this value and and returns true.
    *  Otherwise, sets the label text to defaultValue and returns false.
    */
    bool updateIntLabel(Label* label, int min, int max, int defaultValue, int* out)
    {
        String& in = label->getText();
        int parsedInt;
        try
        {
            parsedInt = std::stoi(in.toRawUTF8());
        }
        catch (const std::exception& e)
        {
            label->setText(String(defaultValue), dontSendNotification);
            return false;
        }

        if (parsedInt < min)
            *out = min;
        else if (parsedInt > max)
            *out = max;
        else
            *out = parsedInt;

        label->setText(String(*out), dontSendNotification);
        return true;
    }

    // Like updateIntLabel, but for floats
    bool updateFloatLabel(Label* label, float min, float max, float defaultValue, float* out)
    {
        String& in = label->getText();
        float parsedFloat;
        try
        {
            parsedFloat = std::stof(in.toRawUTF8());
        }
        catch (const std::exception& e)
        {
            label->setText(String(defaultValue), dontSendNotification);
            return false;
        }

        if (parsedFloat < min)
            *out = min;
        else if (parsedFloat > max)
            *out = max;
        else
            *out = parsedFloat;

        label->setText(String(*out), dontSendNotification);
        return true;
    }
        
}

CrossingDetectorEditor::CrossingDetectorEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors)
    : GenericEditor(parentNode, useDefaultParameterEditors)
{
    desiredWidth = 341;

    /* ------------- CRITERIA SECTION ---------------- */

    inputLabel = new Label("InputChanL", "Input");
    inputLabel->setBounds(8, 36, 50, 18);
    inputLabel->setFont(Font("Small Text", 12, Font::plain));
    inputLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(inputLabel);

    inputBox = new ComboBox("Input channel");
    inputBox->setTooltip(INPUT_CHAN_TOOLTIP);
    inputBox->setBounds(60, 36, 40, 18);
    inputBox->addListener(this);
    addAndMakeVisible(inputBox);

    risingButton = new UtilityButton("RISING", Font("Default", 10, Font::plain));
    risingButton->addListener(this);
    risingButton->setBounds(105, 26, 60, 18);
    risingButton->setClickingTogglesState(true);
    bool enable = (START_DIRECTION == dPos || START_DIRECTION == dPosOrNeg);
    risingButton->setToggleState(enable, dontSendNotification);
    risingButton->setTooltip(RISING_TOOLTIP);
    addAndMakeVisible(risingButton);

    fallingButton = new UtilityButton("FALLING", Font("Default", 10, Font::plain));
    fallingButton->addListener(this);
    fallingButton->setBounds(105, 46, 60, 18);
    fallingButton->setClickingTogglesState(true);
    enable = (START_DIRECTION == dNeg || START_DIRECTION == dPosOrNeg);
    fallingButton->setToggleState(enable, dontSendNotification);
    fallingButton->setTooltip(FALLING_TOOLTIP);
    addAndMakeVisible(fallingButton);

    acrossLabel = new Label("AcrossL", "across");
    acrossLabel->setBounds(168, 36, 60, 18);
    acrossLabel->setFont(Font("Small Text", 12, Font::plain));
    acrossLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(acrossLabel);

    thresholdEditable = new Label("Threshold", String(START_THRESH));
    thresholdEditable->setEditable(true);
    thresholdEditable->addListener(this);
    thresholdEditable->setBounds(230, 36, 50, 18);
    thresholdEditable->setColour(Label::backgroundColourId, Colours::grey);
    thresholdEditable->setColour(Label::textColourId, Colours::white);
    thresholdEditable->setTooltip(THRESH_TOOLTIP);
    addAndMakeVisible(thresholdEditable);

    /* -------------- BEFORE SECTION ----------------- */

    beforeLabel = new Label("BeforeL", "Before:");
    beforeLabel->setBounds(8, 68, 65, 18);
    beforeLabel->setFont(Font("Small Text", 12, Font::plain));
    beforeLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(beforeLabel);

    pctPrevEditable = new Label("Percent Prev", String(100 * START_FRAC_PREV));
    pctPrevEditable->setEditable(true);
    pctPrevEditable->addListener(this);
    pctPrevEditable->setBounds(75, 68, 33, 18);
    pctPrevEditable->setColour(Label::backgroundColourId, Colours::grey);
    pctPrevEditable->setColour(Label::textColourId, Colours::white);
    pctPrevEditable->setTooltip(PCT_PREV_TOOLTIP);
    addAndMakeVisible(pctPrevEditable);

    bPctLabel = new Label("PctPrevL", "% of");
    bPctLabel->setBounds(110, 68, 40, 18);
    bPctLabel->setFont(Font("Small Text", 12, Font::plain));
    bPctLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(bPctLabel);

    numPrevEditable = new Label("Num Prev", String(START_NUM_PREV));
    numPrevEditable->setEditable(true);
    numPrevEditable->addListener(this);
    numPrevEditable->setBounds(152, 68, 33, 18);
    numPrevEditable->setColour(Label::backgroundColourId, Colours::grey);
    numPrevEditable->setColour(Label::textColourId, Colours::white);
    numPrevEditable->setTooltip(NUM_PREV_TOOLTIP);
    addAndMakeVisible(numPrevEditable);

    bSampLabel = new Label("SampPrevL", "sample(s)");
    bSampLabel->setBounds(188, 68, 85, 18);
    bSampLabel->setFont(Font("Small Text", 12, Font::plain));
    bSampLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(bSampLabel);

    /* --------------- AFTER SECTION ----------------- */

    afterLabel = new Label("AfterL", "After:");
    afterLabel->setBounds(8, 88, 65, 18);
    afterLabel->setFont(Font("Small Text", 12, Font::plain));
    afterLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(afterLabel);

    pctNextEditable = new Label("Percent Next", String(100 * START_FRAC_NEXT));
    pctNextEditable->setEditable(true);
    pctNextEditable->addListener(this);
    pctNextEditable->setBounds(75, 88, 33, 18);
    pctNextEditable->setColour(Label::backgroundColourId, Colours::grey);
    pctNextEditable->setColour(Label::textColourId, Colours::white);
    pctNextEditable->setTooltip(PCT_NEXT_TOOLTIP);
    addAndMakeVisible(pctNextEditable);

    aPctLabel = new Label("PctNextL", "% of");
    aPctLabel->setBounds(110, 88, 40, 18);
    aPctLabel->setFont(Font("Small Text", 12, Font::plain));
    aPctLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(aPctLabel);

    numNextEditable = new Label("Num Next", String(START_NUM_NEXT));
    numNextEditable->setEditable(true);
    numNextEditable->addListener(this);
    numNextEditable->setBounds(152, 88, 33, 18);
    numNextEditable->setColour(Label::backgroundColourId, Colours::grey);
    numNextEditable->setColour(Label::textColourId, Colours::white);
    numNextEditable->setTooltip(NUM_NEXT_TOOLTIP);
    addAndMakeVisible(numNextEditable);

    aSampLabel = new Label("SampNextL", "sample(s)");
    aSampLabel->setBounds(188, 88, 85, 18);
    aSampLabel->setFont(Font("Small Text", 12, Font::plain));
    aSampLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(aSampLabel);

    /* -------------- OUTPUT SECTION ----------------- */

    outputLabel = new Label("OutL", "Output:");
    outputLabel->setBounds(8, 108, 62, 18);
    outputLabel->setFont(Font("Small Text", 12, Font::plain));
    outputLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(outputLabel);

    eventBox = new ComboBox("Out event channel");
    for (int chan = 1; chan <= 8; chan++)
        eventBox->addItem(String(chan), chan);
    eventBox->setSelectedId(START_OUTPUT + 1);
    eventBox->setBounds(72, 108, 35, 18);
    eventBox->setTooltip(OUTC_TOOLTIP);
    eventBox->addListener(this);
    addAndMakeVisible(eventBox);

    durLabel = new Label("DurL", "Dur:");
    durLabel->setBounds(112, 108, 35, 18);
    durLabel->setFont(Font("Small Text", 12, Font::plain));
    durLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(durLabel);

    durationEditable = new Label("Event Duration", String(START_DURATION));
    durationEditable->setEditable(true);
    durationEditable->addListener(this);
    durationEditable->setBounds(151, 108, 50, 18);
    durationEditable->setColour(Label::backgroundColourId, Colours::grey);
    durationEditable->setColour(Label::textColourId, Colours::white);
    durationEditable->setTooltip(DURATION_TOOLTIP);
    addAndMakeVisible(durationEditable);

    timeoutLabel = new Label("TimeoutL", "Timeout:");
    timeoutLabel->setBounds(206, 108, 64, 18);
    timeoutLabel->setFont(Font("Small Text", 12, Font::plain));
    timeoutLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(timeoutLabel);

    timeoutEditable = new Label("Timeout", String(START_TIMEOUT));
    timeoutEditable->setEditable(true);
    timeoutEditable->addListener(this);
    timeoutEditable->setBounds(274, 108, 50, 18);
    timeoutEditable->setColour(Label::backgroundColourId, Colours::grey);
    timeoutEditable->setColour(Label::textColourId, Colours::white);
    timeoutEditable->setTooltip(TIMEOUT_TOOLTIP);
    addAndMakeVisible(timeoutEditable);
}

CrossingDetectorEditor::~CrossingDetectorEditor() {}

void CrossingDetectorEditor::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == inputBox)
        getProcessor()->setParameter(pInputChan, (float)inputBox->getSelectedId() - 1);
    else if (comboBoxThatHasChanged == eventBox)
        getProcessor()->setParameter(pEventChan, (float)eventBox->getSelectedId() - 1);
}

void CrossingDetectorEditor::labelTextChanged(Label* labelThatHasChanged)
{
    CrossingDetector* processor = dynamic_cast<CrossingDetector*>(getProcessor());

    if (labelThatHasChanged == durationEditable)
    {
        int newVal;
        bool success = updateIntLabel(labelThatHasChanged, 0, INT_MAX, processor->getEventDuration(), &newVal);

        if (success)
            processor->setParameter(pEventDur, (float)newVal);
    }
    else if (labelThatHasChanged == timeoutEditable)
    {
        int newVal;
        bool success = updateIntLabel(labelThatHasChanged, 0, INT_MAX, processor->getTimeout(), &newVal);

        if (success)
            processor->setParameter(pTimeout, (float)newVal);
    }
    else if (labelThatHasChanged == thresholdEditable)
    {
        float newVal;
        bool success = updateFloatLabel(labelThatHasChanged, -FLT_MAX, FLT_MAX, processor->getThreshold(), &newVal);

        if (success)
            processor->setParameter(pThreshold, newVal);
    }
    else if (labelThatHasChanged == pctPrevEditable)
    {
        float newVal;
        bool success = updateFloatLabel(labelThatHasChanged, 0, 100, 100 * processor->getFracPrev(), &newVal);

        if (success)
            processor->setParameter(pFracPrev, newVal / 100);
    }
    else if (labelThatHasChanged == numPrevEditable)
    {
        int newVal;
        bool success = updateIntLabel(labelThatHasChanged, 0, MAX_NUM_PREV, processor->getNumPrev(), &newVal);

        if (success)
            processor->setParameter(pNumPrev, (float)newVal);
    }
    else if (labelThatHasChanged == pctNextEditable)
    {
        float newVal;
        bool success = updateFloatLabel(labelThatHasChanged, 0, 100, 100 * processor->getFracNext(), &newVal);

        if (success)
            processor->setParameter(pFracNext, newVal / 100);
    }
    else if (labelThatHasChanged == numNextEditable)
    {
        int newVal;
        bool success = updateIntLabel(labelThatHasChanged, 0, MAX_NUM_NEXT, processor->getNumNext(), &newVal);

        if (success)
            processor->setParameter(pNumNext, (float)newVal);
    }
}

void CrossingDetectorEditor::buttonEvent(Button* button)
{
    if (button == risingButton || button == fallingButton)
    {
        bool risingOn = risingButton->getToggleState();
        bool fallingOn = fallingButton->getToggleState();

        CrossingDirection newDirection;
        if (risingOn)
            if (fallingOn)
                newDirection = dPosOrNeg;
            else
                newDirection = dPos;
        else
            if (fallingOn)
                newDirection = dNeg;
            else
                newDirection = dNone;

        getProcessor()->setParameter(pDirection, (float)newDirection);
    }
}

void CrossingDetectorEditor::updateSettings()
{
    // update input combo box
    int numInputs = getProcessor()->settings.numInputs;
    int numBoxItems = inputBox->getNumItems();
    if (numInputs != numBoxItems)
    {
        int currId = inputBox->getSelectedId();
        inputBox->clear(dontSendNotification);
        for (int chan = 1; chan <= numInputs; chan++)
            // using 1-based ids since 0 is reserved for "nothing selected"
            inputBox->addItem(String(chan), chan);
        if (numInputs > 0 && (currId < 1 || currId > numInputs))
            inputBox->setSelectedId(START_INPUT + 1, sendNotificationAsync);
        else
            inputBox->setSelectedId(currId, dontSendNotification);
    }
    
}

void CrossingDetectorEditor::saveCustomParameters(XmlElement* xml)
{
    xml->setAttribute("Type", "CrossingDetectorEditor");

    CrossingDetector* processor = dynamic_cast<CrossingDetector*>(getProcessor());
    XmlElement* paramValues = xml->createNewChildElement("VALUES");

    paramValues->setAttribute("inputChanId", inputBox->getSelectedId());
    paramValues->setAttribute("bRising", risingButton->getToggleState());
    paramValues->setAttribute("bFalling", fallingButton->getToggleState());
    paramValues->setAttribute("threshold", thresholdEditable->getText());
    paramValues->setAttribute("prevPct", pctPrevEditable->getText());
    paramValues->setAttribute("prevNum", numPrevEditable->getText());
    paramValues->setAttribute("nextPct", pctNextEditable->getText());
    paramValues->setAttribute("nextNum", numNextEditable->getText());
    paramValues->setAttribute("outputChanId", eventBox->getSelectedId());
    paramValues->setAttribute("duration", durationEditable->getText());
    paramValues->setAttribute("timeout", timeoutEditable->getText());
}

void CrossingDetectorEditor::loadCustomParameters(XmlElement* xml)
{
    forEachXmlChildElementWithTagName(*xml, xmlNode, "VALUES")
    {
        inputBox->setSelectedId(xmlNode->getIntAttribute("inputChanId", inputBox->getSelectedId()), sendNotificationSync);
        risingButton->setToggleState(xmlNode->getBoolAttribute("bRising", risingButton->getToggleState()), sendNotificationSync);
        fallingButton->setToggleState(xmlNode->getBoolAttribute("bFalling", fallingButton->getToggleState()), sendNotificationSync);
        thresholdEditable->setText(xmlNode->getStringAttribute("threshold", thresholdEditable->getText()), sendNotificationSync);
        pctPrevEditable->setText(xmlNode->getStringAttribute("prevPct", pctPrevEditable->getText()), sendNotificationSync);
        numPrevEditable->setText(xmlNode->getStringAttribute("prevNum", numPrevEditable->getText()), sendNotificationSync);
        pctNextEditable->setText(xmlNode->getStringAttribute("nextPct", pctNextEditable->getText()), sendNotificationSync);
        numNextEditable->setText(xmlNode->getStringAttribute("nextNum", numNextEditable->getText()), sendNotificationSync);
        eventBox->setSelectedId(xmlNode->getIntAttribute("outputChanId", eventBox->getSelectedId()), sendNotificationSync);
        durationEditable->setText(xmlNode->getStringAttribute("duration", durationEditable->getText()), sendNotificationSync);
        timeoutEditable->setText(xmlNode->getStringAttribute("timeout", timeoutEditable->getText()), sendNotificationSync);
    }
}