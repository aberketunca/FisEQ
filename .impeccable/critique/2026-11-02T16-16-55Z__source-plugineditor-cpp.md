---
target: Source/PluginEditor.cpp
total_score: 22
max_score: 40
na_heuristics: 
p0_count: 1
p1_count: 2
timestamp: 2026-11-02T16-16-55Z
slug: source-plugineditor-cpp
---
# FisEQ UI Critique — Pro-Q Interaction + Scientific Spectrogram Pass

Method: dual-agent (A: design-review · B: detector-evidence)

## Design Health Score

| # | Heuristic | Score | Key Issue |
|---|-----------|-------|-----------|
| 1 | Visibility of System Status | 3 | Live spectrum + curve + tooltip; but Q has no persistent readout, no pre/post or clip indication |
| 2 | Match System / Real World | 4 | dBFS, Hz/kHz, octave grid, log axes, Q, bells — fluent audio-engineer language |
| 3 | User Control and Freedom | 1 | No bypass, no flatten/clear, no undo surfaced; only reset is the hidden double-click |
| 4 | Consistency and Standards | 3 | EQ conventions followed; knob diverges from node numeric-entry pattern |
| 5 | Error Prevention | 3 | All values jlimit-clamped; but overlapping node hit-zones cause wrong-band grabs |
| 6 | Recognition Rather Than Recall | 1 | 3 of 4 primary gestures are invisible, must be recalled not recognized |
| 7 | Flexibility and Efficiency | 2 | Fast for experts who know the gestures; no keyboard, no typed entry, no shift-fine-drag |
| 8 | Aesthetic and Minimalist Design | 4 | Standout — restrained palette, purposeful depth, analytical spectrum, ships-ready look |
| 9 | Error Recovery | 1 | Essentially absent; no error/recovery affordances, no analyzer-stall indication |
| 10 | Help and Documentation | 0 | None — and the hidden gestures make the absence actively harmful |
| **Total** | | **22/40** | **Good-looking and domain-fluent, dragged down by discoverability/control gaps** |

## Design Specificity Verdict

**LLM assessment**: This now reads as genuinely authored for an audio EQ — not category-interchangeable. The recent changes pushed it from "generic dark dashboard with a graph" into recognizably Pro-Q-adjacent territory. The interaction vocabulary is the real EQ idiom (drag = freq/gain, scroll = Q logarithmic, double-click = reset to 0), and the spectrum rewrite consciously chooses "measurement tool, not visualizer." Most importantly, the log-frequency and magnitude math is honest and shared across components, so node, curve, and spectrum line up on one coordinate truth. But it stops short of domain-leading: no filter-type/slope, no per-band bypass/solo, no dynamic EQ, no spectrum pre/post. Roughly 60% of the way to flagship — "respectable boutique EQ," not Pro-Q.

**Deterministic scan**: CLI detector returned empty (exit 0) — correct and expected for C++ JUCE files, not a false positive. Manual review found: 26 unique hex colors with 60+ inline literal instances and no token system; the palette is coherent (Tailwind slate + cyan/sky + band-400s) but never named in code. 5 font sizes with four clustered in the 9–11px band (no real scale). Two text colors fail WCAG AA: `#64748b` (~3.4:1, subtitle + GAIN label) and `#475569` (~1.9:1, minor axis labels). Zero keyboard/focus/AccessibilityHandler support. Performance: 3× redundant spectrum sampling loops, ~120k std::pow/sec, ~3000+ biquad evals/frame, per-frame Path allocations, no dirty-checking, repaints at 60fps even on silence.

## Overall Impression

A tasteful, domain-fluent 5-band EQ that nails aesthetics (H8=4) and audio-world match (H2=4), with correct underlying coordinate math. The recent changes genuinely moved it toward Pro-Q's interaction territory. It is held back by a single structural weakness expressed five ways: **advanced interactions were implemented but never made discoverable.** Three expert gestures (drag, scroll-Q, double-click-reset) ship invisible. Fix discoverability and typography/control and this jumps to the high-20s without touching the DSP.

## What's Working

1. **Correct, honest coordinate math across components.** frequencyToX/xToFrequency use matching exponents in both SpectrumDisplay and EQCurveEditor, and per-band curves are drawn from the real filter magnitude. Node, curve, and spectrum share one coordinate truth — the hardest thing to get right in an EQ UI, and it's right.

2. **The "measurement tool, not visualizer" spectrum restraint is disciplined.** Thin 1px core over a faint AA pass, 0.75px peak-hold ghost at 0.3 alpha, cold desaturated fill, dBFS-calibrated grid. Reads as instrumentation and correctly cedes visual priority to the EQ curve. Both assessments independently praised this.

3. **Expert interaction grammar is complete and conventional.** Drag-xy / scroll-Q(log) / double-click-reset-to-0 is exactly Pro-Q muscle memory, with sign-aware, unit-aware tooltip formatting. Frictionless for the target power user.

## Priority Issues

**[P0] Primary interactions are invisible**
- **Why it matters**: This single gap drives 3 of 8 cognitive-load failures and 4 low Nielsen scores (H3, H6, H7, H10). A first-timer can open the plugin and not realize it's interactive at all. Features that were built are effectively unshipped because nobody finds them. The cursor only changes after a successful hover-hit.
- **Fix**: Add a persistent low-key hint line ("Drag · Scroll = Q · Double-click = reset") that fades after first interaction. Show Q on the always-visible node or as a ring. Make the whole graph show a grab/crosshair cursor so the surface reads as interactive. A right-click context menu (reset/bypass/filter-type) doubles as self-documentation.
- **Suggested command**: `/impeccable onboard`

**[P1] Typography size + contrast below legible floor**
- **Why it matters**: Reading the freq/dB axes is core to the Operate task. At 9px low-slate on near-black, the numbers that let a user interpret the curve are the least readable thing on screen. `#475569` axis labels measure ~1.9:1 contrast; `#64748b` subtitle/GAIN ~3.4:1 — both fail WCAG AA. Worsens on scaled hi-DPI DAW windows.
- **Fix**: Floor axis/body text at 11–12px, raise base label color to ~`#94a3b8` (passes at 6.9:1), reserve dimmer slate for gridlines only. Scale font sizes with window size.
- **Suggested command**: `/impeccable typeset`

**[P1] No user control: no bypass, no clear/flat, no visible reset or undo**
- **Why it matters**: H3 = 1/4. A/B'ing against flat and bypassing a band are fundamental EQ workflows. Their absence caps how serious this can feel regardless of how good it looks. There is a gain knob in the chrome but no "compare to flat" — arguably backwards priority for an Operate tool.
- **Fix**: Add per-band bypass (alt-click node or small toggle), a global bypass in the top bar, "flatten all," and a right-click context menu per node.
- **Suggested command**: `/impeccable shape`

**[P2] Band selection is ambiguous when nodes cluster**
- **Why it matters**: findBandAt returns the lowest-index band within a 16px radius; near 0 dB all five nodes converge and hit-zones overlap, so grabbing a specific band is a coin toss. "I grabbed band 3 but band 1 moved" is a trust-breaking, repeatable frustration.
- **Fix**: Prefer the nearest node, not the first; raise the active band's z-order and hit priority; consider vertical fanning of labels.
- **Suggested command**: `/impeccable harden`

**[P2] Bandwidth indicator doesn't match real filter geometry**
- **Why it matters**: drawBandwidthIndicator uses a linear `freq ± (freq/Q)*0.5` approximation (the code comment writes the correct geometric formula then uses the cruder one) while the adjacent per-band curve uses true magnitude. The shaded region won't line up with the curve's skirts, badly at low freq/high Q. For a UI selling "scientific," that's a credibility leak.
- **Fix**: Derive the shaded extent from the real -3dB/half-gain points, or drop the shade and let the accurate per-band curve carry it.
- **Suggested command**: `/impeccable harden`

## Deterministic-Only Findings (caught by Assessment B, not the design review)

- **dBFS calibration mismatch (real bug)**: the analyzer normalizes against a -100 dB floor (per code comments) while SpectrumDisplay maps `minDBFS = -90`. That's a 10 dB offset between the spectrum data and the drawn grid — the "scientific" grid is slightly miscalibrated. Stale comments in SpectrumDisplay.h:30 / .cpp:56 still say "-100 dBFS at bottom."
- **Dual unlabeled vertical scales**: the shared graph overlays the EQ curve (±24 dB) and the spectrum (-90..0 dBFS), but only the ±24 dB scale is labeled on the left axis. The spectrum's dBFS grid has no labels.
- **Q wheel clamp hard-coded** to [0.1, 10.0] literals rather than the parameter's actual NormalisableRange — silent mismatch risk if the range changes.
- **No token system**: `#1e293b` appears 5+ times, `#22d3ee` 4+, `#64748b` 3, `#0f172a` 3 — exactly what a token would consolidate.

## Persona Red Flags

**Alex (power user / producer):**
Loves the gestures once found, but immediately misses filter-type/slope selection, per-band bypass/solo, and spectrum pre/post — the first things a pro reaches for. No typed numeric entry and no shift-fine-drag makes precise targets fiddly. The approximate bandwidth shade reads as sloppy to someone who reads curves for a living. Likely verdict: "nice, but I'll keep Pro-Q."

**Jordan (first-timer / bedroom producer):**
May not realize the plugin is interactive at all — no hint, no onboarding, cursor only reacts post-hit. High risk of "it's just a pretty graph" abandonment. Won't discover scroll=Q or double-click-reset without docs that don't exist (H10=0). Small low-contrast axes make it hard to learn what the frequencies even are.

**Sam (accessibility-focused):**
No keyboard navigation or screen-reader handler at all — mouse-only, a hard blocker. Hue-only band coding (pink/amber/emerald/blue/purple) collapses under deuteranopia/protanopia with no shape/pattern/label fallback. Sub-11px low-slate-on-black text fails contrast broadly. No focus indicators.

## Minor Observations

- Gain knob is cramped: 70×~40px slot holding the arc + a 9px "GAIN" caption + a 10px in-knob value, both fighting for the lower half.
- The knob's active arc only draws when gain ≠ unity — a correct but undocumented "neutral" cue a user might read as "broken."
- 0 dB center line in the curve editor is at white alpha 0.08 — very faint for the single most important reference line in the UI.
- Tooltip vanishes abruptly on mouseUp; a brief dwell-fade would feel less jarring.
- Peak-hold line has no label or reset — users won't know what the faint upper trace is.
- Resizable to 640×380 but nothing reflows/hides; axis labels crowd at min size.
- Spectrum repaints at 60fps even on total silence (fallback branch still decays + repaints); no visibility pause.

## Questions to Consider

1. You built three expert gestures and surfaced none. If a feature ships invisible, did you ship it — or just spend the engineering?
2. The spectrum got a "scientific" rewrite, but the bandwidth indicator is an admitted approximation sitting next to a mathematically-correct curve. Which one is the product's actual claim — precise instrument, or pretty enough?
3. Who is this for? The interaction model assumes Pro-Q muscle memory (expert), but the feature set is beginner-simple (5 bells, no filter types). Are you building the first EQ someone learns on, or the one they graduate to? Right now it fully serves neither.
4. There's a gain knob in the chrome but no bypass and no A/B. For an Operate tool, isn't "compare to flat" more fundamental than "trim the output"?
