#include "PluginEditor.h"

static const char* effectNames[32] =
{
    "Cathedral","Plate","Small Hall","Room","Studio","Concert","Stage","Vocal",
    "Percussion","Delay","Echo","Gated Reverb","Reverse Reverb","Vocal Distortion",
    "Rotary Speaker","Vocoder","Pitch","Flanger","Chorus","Tremolo & Delay",
    "Delay & Reverb","Pitch & Reverb","Flanger & Reverb","Chorus & Reverb",
    "Pitch / Reverb","Flanger / Reverb","Chorus / Reverb","Tremolo / Reverb",
    "Delay / Reverb","Pitch / Echo","Flanger / Echo","Chorus / Echo"
};

void XSideAudioProcessorEditor::setupKnob(
    juce::Slider& s, const juce::String& suffix)
{
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow,false,78,20);
    s.setTextValueSuffix(suffix);

    s.setColour(juce::Slider::rotarySliderOutlineColourId,
                juce::Colour(0xff3d4349));
    s.setColour(juce::Slider::rotarySliderFillColourId,
                juce::Colour(0xffa8b0b7));
    s.setColour(juce::Slider::thumbColourId,
                juce::Colour(0xfff0eee8));
    s.setColour(juce::Slider::textBoxBackgroundColourId,
                juce::Colour(0xff0d120c));
    s.setColour(juce::Slider::textBoxTextColourId,
                juce::Colour(0xff7dff6b));
}

XSideAudioProcessorEditor::XSideAudioProcessorEditor(
    XSideAudioProcessor& proc)
    : AudioProcessorEditor(&proc), p(proc)
{
    setSize(1120,700);

    title.setText(
        "X-SIDE VST3  •  v4.6  •  PRESETS / LIVE / SONGS  •  MIDI DISABLED",
        juce::dontSendNotification);
    title.setFont(juce::FontOptions(22.0f,juce::Font::bold));
    addAndMakeVisible(title);

    tabs.addTab("MAIN",juce::Colour(0xff171815),&mainPage,false);
    tabs.addTab("LIVE",juce::Colour(0xff171815),&livePage,false);
    tabs.addTab("SONGS",juce::Colour(0xff171815),&songsPage,false);
    addAndMakeVisible(tabs);

    presetName.setColour(
        juce::Label::textColourId,
        juce::Colour(0xff7dff6b));
    presetName.setFont(juce::FontOptions(18.0f,juce::Font::bold));
    mainPage.addAndMakeVisible(presetName);

    info.setColour(
        juce::Label::textColourId,
        juce::Colour(0xffaeb6ad));
    mainPage.addAndMakeVisible(info);

    mainPage.addAndMakeVisible(presets);
    rebuildPresetMenu();

    presets.onChange=[this]
    {
        auto fp=p.getFactoryPresets();
        if (auto* arr=fp.getArray())
        {
            int i=presets.getSelectedId()-1;
            if (juce::isPositiveAndBelow(i,arr->size()))
                p.loadFactoryPreset((*arr)[i]);
        }
    };

    for (int i=0;i<32;++i)
        effect.addItem(
            juce::String(i+1).paddedLeft('0',2)
            +"  "+effectNames[i], i+1);

    for (int i=0;i<32;++i)
        variation.addItem(juce::String(i+1),i+1);

    engine.addItemList({"COUPLE","LEFT","RIGHT"},1);

    mainPage.addAndMakeVisible(effect);
    mainPage.addAndMakeVisible(variation);
    mainPage.addAndMakeVisible(engine);

    setupKnob(a,"");
    setupKnob(b,"");
    setupKnob(low,"");
    setupKnob(high,"");
    setupKnob(mix,"%");

    mainPage.addAndMakeVisible(a);
    mainPage.addAndMakeVisible(b);
    mainPage.addAndMakeVisible(low);
    mainPage.addAndMakeVisible(high);
    mainPage.addAndMakeVisible(mix);
    mainPage.addAndMakeVisible(bypass);

    ae=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.apvts,"effect",effect);
    av=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.apvts,"variation",variation);
    aeng=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.apvts,"engine",engine);

    aa=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.apvts,"editA",a);
    ab=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.apvts,"editB",b);
    al=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.apvts,"lowEQ",low);
    ah=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.apvts,"highEQ",high);
    am=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.apvts,"dryWet",mix);
    aby=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.apvts,"bypass",bypass);

    rebuildLive();

    songsPage.addAndMakeVisible(songBox);
    songsPage.addAndMakeVisible(newSong);
    songsPage.addAndMakeVisible(addStep);
    songsPage.addAndMakeVisible(prevStep);
    songsPage.addAndMakeVisible(nextStep);
    songsPage.addAndMakeVisible(songStatus);

    newSong.onClick=[this]
    {
        p.addSong("Song "+juce::String(p.getSongCount()+1));
        currentSong=juce::jmax(0,p.getSongCount()-1);
        currentStep=0;
        rebuildSongs();
    };

    addStep.onClick=[this]
    {
        if (p.getSongCount()>0)
        {
            p.addCurrentToSong(currentSong);
            rebuildSongs();
        }
    };

    songBox.onChange=[this]
    {
        currentSong=juce::jmax(0,songBox.getSelectedItemIndex());
        currentStep=0;
        rebuildSongs();
    };

    prevStep.onClick=[this]
    {
        if (p.getSongCount()>0)
        {
            currentStep=juce::jmax(0,currentStep-1);
            p.recallSongStep(currentSong,currentStep);
            rebuildSongs();
        }
    };

    nextStep.onClick=[this]
    {
        if (p.getSongCount()>0)
        {
            int n=p.getSongStepCount(currentSong);
            if (n>0)
            {
                currentStep=juce::jmin(n-1,currentStep+1);
                p.recallSongStep(currentSong,currentStep);
                rebuildSongs();
            }
        }
    };

    rebuildSongs();
    startTimerHz(10);
}

void XSideAudioProcessorEditor::rebuildPresetMenu()
{
    presets.clear();

    auto fp=p.getFactoryPresets();
    if (auto* arr=fp.getArray())
        for (int i=0;i<arr->size();++i)
            presets.addItem(
                (*arr)[i].getProperty("name","Preset").toString(),
                i+1);
}

void XSideAudioProcessorEditor::rebuildLive()
{
    liveRecall.clear();
    liveStore.clear();

    for (int i=0;i<12;++i)
    {
        auto* r=liveRecall.add(
            new juce::TextButton(p.getLiveSlotName(i)));

        auto* s=liveStore.add(
            new juce::TextButton("SET "+juce::String(i+1)));

        livePage.addAndMakeVisible(r);
        livePage.addAndMakeVisible(s);

        r->onClick=[this,i]
        {
            p.recallLiveSlot(i);
            rebuildLive();
        };

        s->onClick=[this,i]
        {
            p.storeLiveSlot(i);
            rebuildLive();
        };
    }
}

void XSideAudioProcessorEditor::rebuildSongs()
{
    songBox.clear(juce::dontSendNotification);

    for (int i=0;i<p.getSongCount();++i)
        songBox.addItem(p.getSongName(i),i+1);

    if (p.getSongCount()>0)
    {
        currentSong=juce::jlimit(
            0,p.getSongCount()-1,currentSong);

        songBox.setSelectedItemIndex(
            currentSong,
            juce::dontSendNotification);

        int n=p.getSongStepCount(currentSong);

        currentStep=
            n>0
            ? juce::jlimit(0,n-1,currentStep)
            : 0;

        songStatus.setText(
            "STEP "
            +juce::String(n>0?currentStep+1:0)
            +" / "+juce::String(n),
            juce::dontSendNotification);
    }
    else
    {
        songStatus.setText(
            "NO SONGS",
            juce::dontSendNotification);
    }
}

void XSideAudioProcessorEditor::timerCallback()
{
    presetName.setText(
        p.getCurrentPresetName(),
        juce::dontSendNotification);

    int fx=juce::roundToInt(
        p.apvts.getRawParameterValue("effect")->load());

    int var=juce::roundToInt(
        p.apvts.getRawParameterValue("variation")->load());

    info.setText(
        juce::String(effectNames[juce::jlimit(0,31,fx)])
        +"  •  Variation "
        +juce::String(var+1)
        +"  •  MIDI will return in v4.7",
        juce::dontSendNotification);
}

void XSideAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff151612));

    g.setColour(juce::Colour(0xff2a211b));
    g.fillRoundedRectangle(
        getLocalBounds().toFloat().reduced(7),
        12.0f);

    g.setColour(juce::Colour(0xffb7b7b3));
    g.fillRect(12,12,getWidth()-24,52);
}

void XSideAudioProcessorEditor::resized()
{
    title.setBounds(
        26,18,getWidth()-52,38);

    tabs.setBounds(
        18,76,getWidth()-36,getHeight()-94);

    auto r=mainPage.getLocalBounds().reduced(18);

    presetName.setBounds(r.removeFromTop(34));
    info.setBounds(r.removeFromTop(24));

    r.removeFromTop(10);

    auto row=r.removeFromTop(38);

    presets.setBounds(row.removeFromLeft(360));
    row.removeFromLeft(8);

    effect.setBounds(row.removeFromLeft(230));
    row.removeFromLeft(8);

    variation.setBounds(row.removeFromLeft(82));
    row.removeFromLeft(8);

    engine.setBounds(row.removeFromLeft(110));

    r.removeFromTop(20);

    auto knobs=r.removeFromTop(235);
    int w=knobs.getWidth()/5;

    a.setBounds(knobs.removeFromLeft(w).reduced(8));
    b.setBounds(knobs.removeFromLeft(w).reduced(8));
    low.setBounds(knobs.removeFromLeft(w).reduced(8));
    high.setBounds(knobs.removeFromLeft(w).reduced(8));
    mix.setBounds(knobs.removeFromLeft(w).reduced(8));

    bypass.setBounds(
        r.removeFromTop(38).removeFromLeft(120));

    auto lr=livePage.getLocalBounds().reduced(18);
    int bw=lr.getWidth()/4;
    int bh=lr.getHeight()/3;

    for (int i=0;i<12;++i)
    {
        auto cell=juce::Rectangle<int>(
            (i%4)*bw,(i/4)*bh,bw,bh)
            .translated(lr.getX(),lr.getY())
            .reduced(8);

        auto setArea=cell.removeFromBottom(32);

        liveRecall[i]->setBounds(cell);
        liveStore[i]->setBounds(setArea);
    }

    auto sr=songsPage.getLocalBounds().reduced(20);

    songBox.setBounds(
        sr.removeFromTop(36).removeFromLeft(360));

    sr.removeFromTop(14);

    auto buttons=sr.removeFromTop(40);

    newSong.setBounds(buttons.removeFromLeft(120));
    buttons.removeFromLeft(8);

    addStep.setBounds(buttons.removeFromLeft(130));
    buttons.removeFromLeft(8);

    prevStep.setBounds(buttons.removeFromLeft(100));
    buttons.removeFromLeft(8);

    nextStep.setBounds(buttons.removeFromLeft(100));
    buttons.removeFromLeft(12);

    songStatus.setBounds(buttons);
}
