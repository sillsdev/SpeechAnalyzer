---
title: Pitch graphs
---

*Pitch* is the perceived fundamental frequency of a sound. While the actual fundamental frequency can be precisely determined through physical measurement, it may differ from the perceived pitch because of overtones, or partials, in the sound.

In acoustic phonetics, *absolute pitch* is the fundamental frequency of voiced speech. It is the acoustic correlate of the (psychoacoustic) pitch. *Frequency* is the number of times a repeated event occurs per unit of time. The *fundamental frequency* (or f0) of a periodic signal is the inverse of the period length. The period is, in turn, the smallest repeating unit of a signal.

Speech Analyzer provides four different pitch graphs:

- The [Raw Pitch graph](raw-pitch) displays a pitch contour determined by three parameters that you can set.
- The [Custom Pitch graph](custom-pitch) displays a pitch contour determined by six parameters that you can set.
- The [Smoothed Pitch graph](smoothed-pitch) displays a smoothed pitch contour determined by six parameters that you can set.
- The [Auto Pitch graph](auto-pitch) displays a smoothed pitch contour determined by six parameters that Speech Analyzer sets to provide the *best overall performance*.

#### ![](../../../../images/001.png) **Note**.

Speech Analyzer computes pitch using the Average Magnitude Difference Function (AMDF), which is a measure of how well a signal matches a time-shifted version of itself, as a function of the amount of time shift. The match is calculated using subtraction (instead of multiplication, as in autocorrelation), which reduces computational time.

#### **Related Topics**
[Graph Types overview](overview)

[CECIL vs. Speech Analyzer pitch terminology](../cecil)

