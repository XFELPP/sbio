/*
 * sbio - Stream Broker IO
 *
 * Copyright (C) 2025-2026 Gabriel Dorlhiac
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "sbio/formats/xtc1/traversal.hh"

#include "sbio/formats/xtc1/xtc1.hh"

#include <cstdint>

namespace sbio {
  namespace XTC1 {
    SBIO_HD const char* get_detector_name(std::uint8_t det) {
      static const char* names[] = {
        "NoDetector",
        "AmoIMS",
        "AmoGD",
        "AmoETOF",
        "AmoITOF",
        "AmoMBES",
        "AmoVMI",
        "AmoBPS",
        "Camp",
        "EpicsArch",
        "BldEb",
        "SxrBeamline",
        "SxrEndstation",
        "XppSb1Ipm",
        "XppSb1Pim",
        "XppMonPim",
        "XppSb2Ipm",
        "XppSb3Ipm",
        "XppSb3Pim",
        "XppSb4Pim",
        "XppGon",
        "XppLas",
        "XppEndstation",
        "AmoEndstation",
        "CxiEndstation",
        "XcsEndstation",
        "MecEndstation",
        "CxiDg1",
        "CxiDg2",
        "CxiDg3",
        "CxiDg4",
        "CxiKb1",
        "CxiDs1",
        "CxiDs2",
        "CxiDsu",
        "CxiSc1",
        "CxiDsd",
        "XcsBeamline",
        "CxiSc2",
        "MecXuvSpectrometer",
        "MecXrtsForw",
        "MecXrtsBack",
        "MecFdi",
        "MecTimeTool",
        "MecTargetChamber",
        "FeeHxSpectrometer",
        "XrayTransportDiagnostic",
        "Lamp",
        "MfxEndstation",
        "MfxDg1",
        "MfxDg2",
        "XrtDiag",
        "DetLab"
      };

      if (det < sizeof(names) / sizeof(names[0])) {
        return names[det];
      }

      return "UnknownDetector";
    }

    SBIO_HD const char* get_device_name(std::uint8_t dev) {
      static const char* names[] = {
        "NoDevice",
        "Evr",
        "Acqiris",
        "Opal1000",
        "Tm6740",
        "pnCCD",
        "Princeton",
        "Fccd",
        "Ipimb",
        "Encoder",
        "Cspad",
        "AcqTDC",
        "Xamps",
        "Cspad2x2",
        "Fexamp",
        "Gsc16ai",
        "Phasics",
        "Timepix",
        "Opal2000",
        "Opal4000",
        "OceanOptics",
        "Opal1600",
        "Opal8000",
        "Fli",
        "Quartz4A150",
        "Andor",
        "USDUSB",
        "OrcaFl40",
        "Imp",
        "Epix",
        "Rayonix",
        "EpixSampler",
        "Pimax",
        "Fccd960",
        "Epix10k",
        "Epix100a",
        "EpixS",
        "Gotthard",
        "DualAndor",
        "Wave8",
        "LeCroy",
        "ControlsCamera",
        "Archon",
        "Jungfrau",
        "Zyla",
        "Epix10ka",
        "Pixis",
        "Uxi",
        "Epix10ka2M",
        "StreakC7700",
        "Epix10kaQuad",
        "JungfrauSegment",
        "JungfrauSegmentM2",
        "JungfrauSegmentM3",
        "JungfrauSegmentM4",
        "iStar",
        "QuadAdc",
        "Alvium"
      };

      if (dev < sizeof(names)/sizeof(names[0])) {
        return names[dev];
      }

      return "UnknownDevice";
    }

    SBIO_HD const char* get_bld_name(std::uint32_t type) {
      static const char* names[] = {
        "EBeam",
        "PhaseCavity",
        "FEEGasDetEnergy",
        "NH2-SB1-IPM-01",
        "XCS-IPM-01",
        "XCS-DIO-01",
        "XCS-IPM-02",
        "XCS-DIO-02",
        "XCS-IPM-03",
        "XCS-DIO-03",
        "XCS-IPM-03m",
        "XCS-DIO-03m",
        "XCS-YAG-1",
        "XCS-YAG-2",
        "XCS-YAG-3m",
        "XCS-YAG-3",
        "XCS-YAG-mono",
        "XCS-IPM-mono",
        "XCS-DIO-mono",
        "XCS-DEC-mono",
        "MEC-LAS-EM-01",
        "MEC-TCTR-PIP-01",
        "MEC-TCTR-DI-01",
        "MEC-XT2-IPM-02",
        "MEC-XT2-IPM-03",
        "MEC-HXM-IPM-01",
        "GMD",
        "CxiDg1_Imb01",
        "CxiDg2_Imb01",
        "CxiDg2_Imb02",
        "CxiDg3_Imb01",
        "CxiDg1_Pim",
        "CxiDg2_Pim",
        "CxiDg3_Pim",
        "XppMon_Pim0",
        "XppMon_Pim1",
        "XppSb2_Ipm",
        "XppSb3_Ipm",
        "XppSb3_Pim",
        "XppSb4_Pim",
        "XppEnds_Ipm0",
        "XppEnds_Ipm1",
        "MEC-XT2-PIM-02",
        "MEC-XT2-PIM-03",
        "CxiDg3_Spec",
        "NH2-SB1-IPM-02",
        "FEE-SPEC0",
        "SXR-SPEC0",
        "XPP-SPEC0",
        "XCS-USR-IPM-01",
        "XCS-USR-IPM-02",
        "XCS-USR-IPM-03",
        "XCS-USR-IPM-04",
        "XCS-IPM-04",
        "XCS-DIO-04",
        "XCS-IPM-05",
        "XCS-DIO-05",
        "XCS-IPM-gon",
        "XCS-IPM-ladm",
        "XPP-AIN-01",
        "XCS-AIN-01",
        "AMO-AIN-01",
        "MFX-BEAMMON-01",
        "EOrbits",
        "MfxDg1_Pim",
        "MfxDg2_Pim",
        "SXR-AIN-01",
        "HX2-SB1-BMMON",
        "XRT-USB-ENCODER-01",
        "XPP-USB-ENCODER-01",
        "XPP-USB-ENCODER-02",
        "XCS-USB-ENCODER-01",
        "CXI-USB-ENCODER-01",
        "XCS-SND-DIO",
        "MFX-USR-DIO",
        "XPP-SB2-BMMON",
        "XPP-SB3-BMMON",
        "HFX-DG2-BMMON",
        "XCS-SB1-BMMON",
        "XCS-SB2-BMMON",
        "CXI-DG2-BMMON",
        "CXI-DG3-BMMON",
        "MFX-DG1-BMMON",
        "MFX-DG2-BMMON",
        "MFX-AIN-01",
        "MEC-AIN-01",
        "FEE-AIN-01",
        "MEC-XT2-BMMON-02",
        "MEC-XT2-BMMON-03",
        "XPP-USR-DIO",
        "XPP-ALC-DIO",
        "XCS-USR-DIO",
        "CXI-USR-DIO",
        "MEC-USR-DIO",
        "MFX-USB-ENCODER-01",
        "HXX-DG1-BMMON-01",
        "EM3L0-BMMON",
        "SP1L2-BMMON",
        "XRT-HRM-DIO",
      };

      if (type < sizeof(names)/sizeof(names[0])) {
        return names[type];
      }

      return "UnknownBld";
    }

    SBIO_HD void inspect_xtc1(Xtc* xtc,
                              MetadataCollector& collector,
                              std::uint32_t current_offset) {
      if (xtc->damage.value() & (1 << XTC1::Damage::DroppedContribution)) {
        return;
      }
      if (xtc->contains.id() == XTC1::TypeId::Id_Xtc) {
        // It's a container Xtc: traverse its children
        int remaining { xtc->sizeofPayload() };
        auto* subxtc { reinterpret_cast<XTC1::Xtc*>(xtc->payload()) };
        while (remaining > 0) {
          inspect_xtc1(subxtc, collector, current_offset + sizeof(XTC1::Xtc));
          remaining -= subxtc->extent;
          subxtc = subxtc->next();
        }
      } else {
        // It's a leaf data Xtc: register it in the collector
        collector.add_detector(xtc, current_offset);
      }
    }
  } // namespace XTC1
} // namespace sbio
