#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class XSideAudioProcessorEditor final
    : public juce::AudioProcessorEditor,
      private juce::Timer
{
public:
    explicit XSideAudioProcessorEditor(XSideAudioProcessor&);
    ~XSideAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    XSideAudioProcessor& p;

    juce::TabbedComponent tabs { juce::TabbedButtonBar::TabsAtTop };
    juce::Component mainPage, livePage, songsPage;

    juce::Label title, presetName, info;
    juce::ComboBox presets, effect, variation, engine;
    juce::Slider a,b,low,high,mix;
    juce::ToggleButton bypass { "BYPASS" };

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> ae,av,aeng;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> aa,ab,al,ah,am;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> aby;

    juce::OwnedArray<juce::TextButton> liveRecall;
    juce::OwnedArray<juce::TextButton> liveStore;

    juce::ComboBox songBox;
    juce::TextButton newSong { "NEW SONG" };
    juce::TextButton addStep { "ADD CURRENT" };
    juce::TextButton prevStep { "PREV FX" };
    juce::TextButton nextStep { "NEXT FX" };
    juce::Label songStatus;

    int currentSong=0;
    int currentStep=0;

    void timerCallback() override;

    void rebuildPresetMenu();
    void rebuildLive();
    void rebuildSongs();

    static void setupKnob(juce::Slider&, const juce::String&);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(XSideAudioProcessorEditor)
};
