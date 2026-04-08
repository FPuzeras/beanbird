# BeanBird
The Intelligent Caffeine & Circadian Optimization Engine for Pebble.

BeanBird isn't just a counter; it’s a predictive tool designed to bridge the gap between your espresso machine and your nervous system. By modeling caffeine pharmacokinetics (PK) directly on your wrist, BeanBird helps you master alertness without sacrificing sleep.

**Disclaimer: BeanBird is a mathematical model for informational purposes and is not medical advice.**

## The Philosophy

Most trackers treat caffeine like a simple tally. BeanBird treats it like a drug. It simulates the journey from the first sip to the final metabolic clearance, even accounting for absorption lag.

1. Zero Tether: Runs 100% on-watch. No phone required for calculations.
2. Zero Bloat: Calculations happen on-demand. No background battery drain.
3. Privacy First: Your metabolic data never leaves your wrist.

##  Development Roadmap
### Phase 1: Biometric Foundations (Current)

Focus: PK Metabolic Modeling & Data Ingestion

  [x] Quick-Log Interface: 6 customizable presets + free-form input.

  [x] Multi-Phase Engine: Simulates the "Gut-to-Blood" transition, not just blood decay.

  [x] Dynamic Durations: 3 configurable drinking speeds (e.g., 5m espresso vs. 45m latte).

  [x] Clearance Tracking: Real-time estimation of mg in system (Blood & Gut).

  [ ] Sleep-Gap Analysis: Calculation of "Deep Sleep Readiness" (Time to <50mg).

  [ ] Daily Limits: Warnings when exceeding set doses per day.

  [ ] UI/UX Overhaul: Transitioning from functional wireframes to polished Pebble aesthetics.

### Phase 2: The Closed-Loop Predictor (Future)

Focus: Improved Accuracy & Active Suggestions.

  [ ] Sensor Fusion: Syncing Heart Rate (HR) and HRV data.

  [ ] Energy Budgeting: Importing sleep stages to adjust baseline fatigue levels.

  [ ] Vigilance Testing: Simple Psychomotor Vigilance Tasks (PVT) to calibrate the model.

  [ ] Auto-Calibration: An algorithmic loop that adjusts your half-life based on HRV response.

  [ ] Smart Windows: Suggestions for the "Optimal Next Dose" timing.

  - TBA (to be added)

## The Science

BeanBird currently utilizes a One-Compartment Pharmacokinetic Model. While generic trackers assume instant absorption, BeanBird models the reality of your biology:

  1. Ingestion: You set the drinking duration.

  2. Absorption (Ka​): Caffeine moves from the gut to the bloodstream.

  3. Elimination (Ke​): Caffeine is metabolized based on a customizable half-life (typically 2–9 hours).

  Why this matters: Depending on gastric load (if you've eaten) and genetics, the "peak" of your coffee might hit at 30 minutes or 90 minutes. BeanBird aims to help you predict that peak so you don't over-stack your doses.

## Features

- Local Storage	All logs are stored in persistent watch storage.
- Fine-Tuning	Adjust your specific half-life and absorption rates in settings.
- Open Source	100% transparent code for the community.
- 6 customizable presets + free-form input.
- Extremely customizable - every aspect can be changed to your liking (but with sane defaults).
- TBA (to be added)

## Developing

The app was developed using the CloudPebble environment. Feel free to contribute!

## The Science for nerds
TBA (to be added)

## Citations
TBA (to be added)
