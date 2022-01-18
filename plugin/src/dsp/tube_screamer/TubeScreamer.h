#pragma once

#include "../CircuitModel.h"

class TubeScreamer : public CircuitModel
{
public:
    TubeScreamer (const String& prefix, AudioProcessorValueTreeState& vts);

    static void addParameters (chowdsp::Parameters& params, const String& prefix);

    void prepare (double sampleRate, int samplesPerBlock) override;

    void process (AudioBuffer<float>& buffer) override;

private:
    std::atomic<float>* gainDBParam = nullptr;
    std::atomic<float>* driveParam = nullptr;

    dsp::Gain<float> inputGain;

    // Port B
    wdft::ResistiveVoltageSourceT<float> Vin;
    wdft::CapacitorT<float> C2 { 1.0e-6f };
    wdft::WDFSeriesT<float, decltype (Vin), decltype (C2)> S1 { Vin, C2 };

    wdft::ResistorT<float> R5 { 10.0e3f };
    wdft::WDFParallelT<float, decltype (S1), decltype (R5)> P1 { S1, R5 };

    // Port C
    wdft::ResistorT<float> R4 { 4.7e3f };
    wdft::CapacitorT<float> C3 { 0.047e-6f };
    wdft::WDFSeriesT<float, decltype (R4), decltype (C3)> S2 { R4, C3 };

    // Port D
    wdft::ResistorT<float> RL { 1.0e6f };

    struct ImpedanceCalc
    {
        template <typename RType>
        static float calcImpedance (RType& R)
        {
            constexpr float A = 100.0f; // op-amp gain
            constexpr float Ri = 1.0e6f; // op-amp input impedance
            constexpr float Ro = 10.0f; // op-amp output impedance

            const auto [Rb, Rc, Rd] = R.getPortImpedances();

            // This scattering matrix was derived using the R-Solver python script (https://github.com/jatinchowdhury18/R-Solver),
            // invoked with command: r_solver.py --adapt 0 --out scratch/tube_screamer_scatt.txt scratch/tube_screamer.txt
            R.setSMatrixData ({ { 0, (A * Rd * Ri - Rc * Rd + Rc * Ro) / ((Rb + Rc) * Rd + Rd * Ri - (Rb + Rc + Ri) * Ro), -((A + 1) * Rd * Ri + Rb * Rd - (Rb + Ri) * Ro) / ((Rb + Rc) * Rd + Rd * Ri - (Rb + Rc + Ri) * Ro), -Ro / (Rd - Ro) },
                                { -(Rb * Rc * Rd - Rb * Rc * Ro) / ((A + 1) * Rc * Rd * Ri + Rb * Rc * Rd - (Rb * Rc + (Rb + Rc) * Rd + (Rc + Rd) * Ri) * Ro), ((A + 1) * Rc * Rc * Rd * Ri + (A + 1) * Rc * Rd * Ri * Ri - Rb * Rb * Rc * Rd + (Rb * Rb * Rc - (Rc + Rd) * Ri * Ri + (Rb * Rb - Rc * Rc) * Rd - (Rc * Rc + 2 * Rc * Rd) * Ri) * Ro) / ((A + 1) * Rc * Rd * Ri * Ri + ((A + 2) * Rb * Rc + (A + 1) * Rc * Rc) * Rd * Ri + (Rb * Rb * Rc + Rb * Rc * Rc) * Rd - (Rb * Rb * Rc + Rb * Rc * Rc + (Rc + Rd) * Ri * Ri + (Rb * Rb + 2 * Rb * Rc + Rc * Rc) * Rd + (2 * Rb * Rc + Rc * Rc + 2 * (Rb + Rc) * Rd) * Ri) * Ro), ((A + 1) * Rb * Rc * Rd * Ri + Rb * Rb * Rc * Rd - (Rb * Rb * Rc + 2 * (Rb * Rb + Rb * Rc) * Rd + (Rb * Rc + 2 * Rb * Rd) * Ri) * Ro) / ((A + 1) * Rc * Rd * Ri * Ri + ((A + 2) * Rb * Rc + (A + 1) * Rc * Rc) * Rd * Ri + (Rb * Rb * Rc + Rb * Rc * Rc) * Rd - (Rb * Rb * Rc + Rb * Rc * Rc + (Rc + Rd) * Ri * Ri + (Rb * Rb + 2 * Rb * Rc + Rc * Rc) * Rd + (2 * Rb * Rc + Rc * Rc + 2 * (Rb + Rc) * Rd) * Ri) * Ro), -Rb * Rc * Ro / ((A + 1) * Rc * Rd * Ri + Rb * Rc * Rd - (Rb * Rc + (Rb + Rc) * Rd + (Rc + Rd) * Ri) * Ro) },
                                { -(Rb * Rc * Rd + Rc * Rd * Ri - (Rb * Rc + Rc * Ri) * Ro) / ((A + 1) * Rc * Rd * Ri + Rb * Rc * Rd - (Rb * Rc + (Rb + Rc) * Rd + (Rc + Rd) * Ri) * Ro), (A * Rc * Rd * Ri * Ri + Rb * Rc * Rc * Rd + (A * Rb * Rc + (2 * A + 1) * Rc * Rc) * Rd * Ri - (Rb * Rc * Rc + 2 * (Rb * Rc + Rc * Rc) * Rd + (Rc * Rc + 2 * Rc * Rd) * Ri) * Ro) / ((A + 1) * Rc * Rd * Ri * Ri + ((A + 2) * Rb * Rc + (A + 1) * Rc * Rc) * Rd * Ri + (Rb * Rb * Rc + Rb * Rc * Rc) * Rd - (Rb * Rb * Rc + Rb * Rc * Rc + (Rc + Rd) * Ri * Ri + (Rb * Rb + 2 * Rb * Rc + Rc * Rc) * Rd + (2 * Rb * Rc + Rc * Rc + 2 * (Rb + Rc) * Rd) * Ri) * Ro), -((A + 1) * Rc * Rc * Rd * Ri + Rb * Rc * Rc * Rd - (Rb * Rc * Rc - Rd * Ri * Ri - (Rb * Rb - Rc * Rc) * Rd + (Rc * Rc - 2 * Rb * Rd) * Ri) * Ro) / ((A + 1) * Rc * Rd * Ri * Ri + ((A + 2) * Rb * Rc + (A + 1) * Rc * Rc) * Rd * Ri + (Rb * Rb * Rc + Rb * Rc * Rc) * Rd - (Rb * Rb * Rc + Rb * Rc * Rc + (Rc + Rd) * Ri * Ri + (Rb * Rb + 2 * Rb * Rc + Rc * Rc) * Rd + (2 * Rb * Rc + Rc * Rc + 2 * (Rb + Rc) * Rd) * Ri) * Ro), -(Rb * Rc + Rc * Ri) * Ro / ((A + 1) * Rc * Rd * Ri + Rb * Rc * Rd - (Rb * Rc + (Rb + Rc) * Rd + (Rc + Rd) * Ri) * Ro) },
                                { (A * Rc * Rd * Ri - ((Rb + Rc) * Rd + Rd * Ri) * Ro) / ((A + 1) * Rc * Rd * Ri + Rb * Rc * Rd - (Rb * Rc + (Rb + Rc) * Rd + (Rc + Rd) * Ri) * Ro), (A * (A + 2 * A) * Rc * Rd * Rd * Ri * Ri + (2 * A * Rb * Rc + A * Rc * Rc) * Rd * Rd * Ri + (Rc * Rd * Ri + (Rb * Rc + Rc * Rc) * Rd) * Ro * Ro - ((Rb * Rc + Rc * Rc) * Rd * Rd + (2 * A * Rc * Rd + A * Rd * Rd) * Ri * Ri + ((A * Rb + (A + 1) * Rc) * Rd * Rd + (2 * A * Rb * Rc + A * Rc * Rc) * Rd) * Ri) * Ro) / ((A + 1) * Rc * Rd * Rd * Ri * Ri + ((A + 2) * Rb * Rc + (A + 1) * Rc * Rc) * Rd * Rd * Ri + (Rb * Rb * Rc + Rb * Rc * Rc) * Rd * Rd + (Rb * Rb * Rc + Rb * Rc * Rc + (Rc + Rd) * Ri * Ri + (Rb * Rb + 2 * Rb * Rc + Rc * Rc) * Rd + (2 * Rb * Rc + Rc * Rc + 2 * (Rb + Rc) * Rd) * Ri) * Ro * Ro - ((Rb * Rb + 2 * Rb * Rc + Rc * Rc) * Rd * Rd + ((A + 2) * Rc * Rd + Rd * Rd) * Ri * Ri + 2 * (Rb * Rb * Rc + Rb * Rc * Rc) * Rd + (2 * (Rb + Rc) * Rd * Rd + ((A + 4) * Rb * Rc + (A + 2) * Rc * Rc) * Rd) * Ri) * Ro), -(A * Rb * Rc * Rd * Rd * Ri + (A * (A + A) * Rc * Rd * Rd * Ri * Ri - ((2 * Rb + Rc) * Rd * Ri + Rd * Ri * Ri + (Rb * Rb + Rb * Rc) * Rd) * Ro * Ro + ((Rb * Rb + Rb * Rc) * Rd * Rd - (A * Rc * Rd + (A - 1) * Rd * Rd) * Ri * Ri - (A * Rb * Rc * Rd + ((A - 2) * Rb + (A - 1) * Rc) * Rd * Rd) * Ri) * Ro)) / ((A + 1) * Rc * Rd * Rd * Ri * Ri + ((A + 2) * Rb * Rc + (A + 1) * Rc * Rc) * Rd * Rd * Ri + (Rb * Rb * Rc + Rb * Rc * Rc) * Rd * Rd + (Rb * Rb * Rc + Rb * Rc * Rc + (Rc + Rd) * Ri * Ri + (Rb * Rb + 2 * Rb * Rc + Rc * Rc) * Rd + (2 * Rb * Rc + Rc * Rc + 2 * (Rb + Rc) * Rd) * Ri) * Ro * Ro - ((Rb * Rb + 2 * Rb * Rc + Rc * Rc) * Rd * Rd + ((A + 2) * Rc * Rd + Rd * Rd) * Ri * Ri + 2 * (Rb * Rb * Rc + Rb * Rc * Rc) * Rd + (2 * (Rb + Rc) * Rd * Rd + ((A + 4) * Rb * Rc + (A + 2) * Rc * Rc) * Rd) * Ri) * Ro), -((A + 1) * Rc * Rd * Rd * Ri + Rb * Rc * Rd * Rd - (Rb * Rc + Rc * Ri) * Ro * Ro - ((Rb + Rc) * Rd * Rd + Rd * Rd * Ri) * Ro) / ((A + 1) * Rc * Rd * Rd * Ri + Rb * Rc * Rd * Rd + (Rb * Rc + (Rb + Rc) * Rd + (Rc + Rd) * Ri) * Ro * Ro - (2 * Rb * Rc * Rd + (Rb + Rc) * Rd * Rd + ((A + 2) * Rc * Rd + Rd * Rd) * Ri) * Ro) } });

            const auto Ra = ((A + 1) * Rc * Rd * Ri + Rb * Rc * Rd - (Rb * Rc + (Rb + Rc) * Rd + (Rc + Rd) * Ri) * Ro) / ((Rb + Rc) * Rd + Rd * Ri - (Rb + Rc + Ri) * Ro);
            return Ra;
        }
    };

    wdft::RtypeAdaptor<float, 0, ImpedanceCalc, decltype (P1), decltype (S2), decltype (RL)> R { std::tie (P1, S2, RL) };

    // Port A
    static constexpr auto R6 = 51.0e3f;
    static constexpr auto Pot1 = 500.0e3f;
    wdft::ResistorT<float> R6_P1 { R6 };
    wdft::CapacitorT<float> C4 { 51.0e-12f };
    wdft::WDFParallelT<float, decltype (R6_P1), decltype (C4)> P2 { R6_P1, C4 };
    wdft::WDFParallelT<float, decltype (P2), decltype (R)> P3 { P2, R };

    wdft::DiodePairT<float, decltype (P3)> dp { P3, 4.352e-9f, 25.85e-3f, 1.906f }; // 1N4148

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TubeScreamer)
};
