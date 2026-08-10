#pragma once
#include <JuceHeader.h>

class XSideAudioProcessor final : public juce::AudioProcessor
{
public:
    XSideAudioProcessor();
    ~XSideAudioProcessor() override = default;

    void prepareToPlay(double, int) override {}
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "X-Side"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    juce::var getFactoryPresets() const { return factoryPresets; }
    void loadFactoryPreset(const juce::var&);
    juce::String getCurrentPresetName() const { return currentPresetName; }

    void storeLiveSlot(int slot);
    void recallLiveSlot(int slot);
    juce::String getLiveSlotName(int slot) const;

    void addSong(const juce::String& name);
    void addCurrentToSong(int songIndex);
    void recallSongStep(int songIndex, int stepIndex);
    int getSongCount() const;
    juce::String getSongName(int i) const;
    int getSongStepCount(int i) const;

private:
    juce::var factoryPresets;
    juce::String currentPresetName { "DSP Capture" };
    juce::Array<juce::var> liveSlots;
    juce::Array<juce::var> songs;

    juce::var snapshot() const;
    void applySnapshot(const juce::var&);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(XSideAudioProcessor)
};
