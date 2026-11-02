#pragma once

#include <juce_graphics/juce_graphics.h>
#include <array>

/**
 * Centralized vintage theme tokens for FisEQ.
 *
 * A single source of truth for the plugin's colour language: a warm,
 * gritty 1970s film palette — aged paper, tobacco, oxblood, amber and gold
 * over deep plum-brown shadows. Every component draws from here so the UI
 * stays cohesive and a palette change is one edit.
 */
namespace Theme
{
    // ---- Surfaces (deep, warm shadows — not neutral black) ----
    inline constexpr juce::uint32 bgDeep       = 0xff140d0a;  // darkest base (plum-brown)
    inline constexpr juce::uint32 bgBase       = 0xff1c1310;  // main background
    inline constexpr juce::uint32 bgRaised     = 0xff241812;  // raised panel / top bar
    inline constexpr juce::uint32 bgPanel      = 0xff2a1d16;  // tooltip / inset panel

    // ---- Lines / borders ----
    inline constexpr juce::uint32 line         = 0xff3a2a20;  // separators
    inline constexpr juce::uint32 lineSoft     = 0xff2e2018;  // faint grid
    inline constexpr juce::uint32 lineStrong   = 0xff5a4230;  // emphasized grid

    // ---- Text (warm cream / tobacco ramp) ----
    inline constexpr juce::uint32 textBright   = 0xfff4e4c4;  // bleached cream (primary)
    inline constexpr juce::uint32 textMid      = 0xffc9a86a;  // aged gold (labels)
    inline constexpr juce::uint32 textDim      = 0xff8a6d4a;  // tobacco (muted, still legible)

    // ---- Accents ----
    inline constexpr juce::uint32 accent       = 0xffe8862a;  // blood-orange / amber (primary accent)
    inline constexpr juce::uint32 accentBright = 0xfff4b45a;  // faded gold highlight
    inline constexpr juce::uint32 accentDeep   = 0xff9e2836;  // oxblood (deep accent)
    inline constexpr juce::uint32 accentGlow   = 0x55e8862a;  // accent with alpha for glow

    // ---- EQ curve / spectrum ----
    inline constexpr juce::uint32 curve        = 0xfff4e4c4;  // composite EQ curve (cream)
    inline constexpr juce::uint32 spectrumLine = 0xffe8a24a;  // analyser line (amber)
    inline constexpr juce::uint32 spectrumFill = 0xffb85a28;  // analyser fill base

    // ---- EQ band nodes: all a single dusty plum (identity by position) ----
    inline constexpr juce::uint32 nodePlum       = 0xff9d5a86;  // dusty plum disc
    inline constexpr juce::uint32 nodePlumBright  = 0xffc78fb0;  // lit plum (active ring)

    // All five bands share the plum (kept as an array so call sites don't change).
    inline constexpr std::array<juce::uint32, 5> bands = {
        nodePlum, nodePlum, nodePlum, nodePlum, nodePlum
    };

    // Convenience accessors
    inline juce::Colour col(juce::uint32 argb) { return juce::Colour(argb); }
    inline juce::Colour band(int i)
    {
        return juce::Colour(bands[static_cast<size_t>(juce::jlimit(0, 4, i))]);
    }
}
