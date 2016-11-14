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

#ifndef CROSSING_DETECTOR_EDITOR_H_INCLUDED
#define CROSSING_DETECTOR_EDITOR_H_INCLUDED

#include <EditorHeaders.h>
#include <string>
#include <climits>
#include <cfloat>

#define INPUT_CHAN_TOOLTIP "Continuous channel to analyze"
#define RISING_TOOLTIP "Trigger events when past samples are below and future samples are above the threshold"
#define FALLING_TOOLTIP "Trigger events when past samples are above and future samples are below the threshold"
#define THRESH_TOOLTIP "Threshold voltage"
#define PCT_PREV_TOOLTIP "Percent of considered past samples required to be above/below threshold"
#define NUM_PREV_TOOLTIP "Number of past samples considered"
#define PCT_NEXT_TOOLTIP "Percent of considered future samples required to be above/below threshold"
#define NUM_NEXT_TOOLTIP "Number of future samples considered"
#define OUTC_TOOLTIP "Event channel to output on when triggered"
#define DURATION_TOOLTIP "Duration of each event, in samples"
#define TIMEOUT_TOOLTIP "Minimum number of samples between consecutive events"

/*
Editor consists of:
-Combo box to select crossing direction to detect
-Combo box to select input (continuous) channel
-Combo box to select output (event) channel
-Editable label to specify the duration of each event, in samples
-Editable label to specify the timeout period after each event, in samples
-Editable label to enter the threshold sample value (crossing of which triggers an event)
-Editable label to enter the percentage of past values required
-Editable label to enter the number of past values to consider
-Editable label to enter the number of future values required
-Editable label to enter the number of future values to consider

@see GenericEditor

*/

class CrossingDetectorEditor : public GenericEditor,
    public ComboBox::Listener, public Label::Listener
{
public:
    CrossingDetectorEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors = false);
    ~CrossingDetectorEditor();

    // implements ComboBox::Listener
    void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override;

    // implements Label::Listener
    void labelTextChanged(Label* labelThatHasChanged) override;

    // overrides GenericEditor
    void buttonEvent(Button* button) override;

    // allow editor to react to changing # of channels
    void updateSettings() override;

    void saveCustomParameters(XmlElement* xml) override;
    void loadCustomParameters(XmlElement* xml) override;

private:
    ScopedPointer<ComboBox> inputBox;
    ScopedPointer<ComboBox> eventBox;

    ScopedPointer<UtilityButton> risingButton;
    ScopedPointer<UtilityButton> fallingButton;

    ScopedPointer<Label> durationEditable;
    ScopedPointer<Label> timeoutEditable;
    ScopedPointer<Label> thresholdEditable;
    ScopedPointer<Label> pctPrevEditable;
    ScopedPointer<Label> numPrevEditable;
    ScopedPointer<Label> pctNextEditable;
    ScopedPointer<Label> numNextEditable;

    // static labels
    ScopedPointer<Label> inputLabel;
    ScopedPointer<Label> acrossLabel;
    ScopedPointer<Label> beforeLabel;
    ScopedPointer<Label> bPctLabel;
    ScopedPointer<Label> bSampLabel;
    ScopedPointer<Label> afterLabel;
    ScopedPointer<Label> aPctLabel;
    ScopedPointer<Label> aSampLabel;
    ScopedPointer<Label> outputLabel;
    ScopedPointer<Label> durLabel;
    ScopedPointer<Label> timeoutLabel;
};

#endif // CROSSING_DETECTOR_EDITOR_H_INCLUDED