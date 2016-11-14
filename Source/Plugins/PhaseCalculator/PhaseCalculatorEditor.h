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


#ifndef PHASE_CALCULATOR_EDITOR_H_INCLUDED
#define PHASE_CALCULATOR_EDITOR_H_INCLUDED

#include <EditorHeaders.h>
#include <cmath>
#include <climits>

#define QUEUE_SIZE_TOOLTIP "Change the total amount of data used to calculate the phase (powers of 2 are best)"
#define NUM_FUTURE_TOOLTIP "Select how much actual (past) vs. predicted (future) data to use when calculating the phase"
#define APPLY_TO_CHAN_TOOLTIP "When this button is off, selected channels pass through unchanged"
#define APPLY_TO_ADC_TOOLTIP "When this button is off, ADC and AUX channels will pass through unchanged"
#define RECALC_INTERVAL_TOOLTIP "Time to wait between calls to update the autoregressive models"
#define GLITCH_LIMIT_TOOLTIP "Maximum number of consecutive samples that can be considered a glitch and corrected by unwrapping and/or smoothing. Set to 0 to turn off glitch correction."

using namespace std;

// modified slider type
class ProcessBufferSlider : public Slider
{
public:
    ProcessBufferSlider(const String& componentName);
    ~ProcessBufferSlider();
    double snapValue(double attemptedValue, DragMode dragMode) override;

    // update the range / position of the slider based on current settings of the PhaseCalculator
    void updateFromProcessor(GenericProcessor* parentNode);

    double getRealMinValue();

private:
    // the actual minimum value that's allowed, although the slider "thinks" the min value is 0 so that the whole range from 0 to processLength is shown.
    double realMinValue;

    LookAndFeel_V3 myLookAndFeel;
};

class PhaseCalculatorEditor : public GenericEditor, public ComboBox::Listener, public Label::Listener
{
public:
    PhaseCalculatorEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors = false);

    ~PhaseCalculatorEditor();

    // enable/disable controls when acquisiton starts/ends
    void startAcquisition() override;
    void stopAcquisition() override;

    // implements ComboBox::Listener
    void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override;

    // implements Label::Listener
    void labelTextChanged(Label* labelThatHasChanged) override;

    // overrides GenericEditor
    void sliderEvent(Slider* slider) override;

    // overrides GenericEditor
    void buttonEvent(Button* button) override;

    // update display based on current channel
    void channelChanged(int chan, bool newState) override;

    void saveCustomParameters(XmlElement* xml) override;
    void loadCustomParameters(XmlElement* xml) override;

private:

    // utility for label listening
    // ouputs whether the label contained a valid input; if so, it is stored in *result.
    template<typename labelType>
    bool updateLabel(Label* labelThatHasChanged,
        labelType minValue, labelType maxValue, labelType defaultValue, labelType* result);

    ScopedPointer<Label>    lowCutLabel;
    ScopedPointer<Label>    lowCutEditable;
    ScopedPointer<Label>    highCutLabel;
    ScopedPointer<Label>    highCutEditable;
    
    ScopedPointer<Label>    processLengthLabel;
    ScopedPointer<Label>    processLengthUnitLabel;
    ScopedPointer<ComboBox> processLengthBox;
    int lastProcessLength;

    ScopedPointer<ProcessBufferSlider> numFutureSlider;
    ScopedPointer<Label>               numPastLabel;
    ScopedPointer<Label>               numPastEditable;
    ScopedPointer<Label>               numFutureLabel;
    ScopedPointer<Label>               numFutureEditable;

    ScopedPointer<UtilityButton> applyToChan;

    // controls whether ADC and AUX channels are processed.
    ScopedPointer<UtilityButton> applyToADC;

    ScopedPointer<Label>    recalcIntervalLabel;
    ScopedPointer<Label>    recalcIntervalEditable;
    ScopedPointer<Label>    recalcIntervalUnit;

    ScopedPointer<Label>    glitchLimLabel;
    ScopedPointer<Label>    glitchLimEditable;
    ScopedPointer<Label>    glitchLimUnit;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhaseCalculatorEditor);
};

#endif // PHASE_CALCULATOR_EDITOR_H_INCLUDED