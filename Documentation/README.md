# JB 160XT research library

Collected 2026-11-15 for an accurate VST recreation of the user's physical dbx 160XT. This folder is the research stage: source documents, original vendor graphics, engineering notes, and a plan for measurements. The existing `DBX-160xt-manual` folder is preserved.

Start with [research findings](00_Index/RESEARCH_FINDINGS.md), then the [source index](00_Index/SOURCE_INDEX.md). Use the [hardware measurement plan](07_Hardware_Measurement_Plan/MEASUREMENT_PLAN.md) to turn the documentation into evidence from your own compressor.

For the JUCE/C++ build, begin with [instructions.md](instructions.md). It defines the user's pixel-for-pixel hardware UI goal, measured audio-fidelity priorities and the coding assistant's implementation/validation workflow.

| Folder | Contents |
| --- | --- |
| `00_Index` | Findings, source register, provenance, searchable text, access gaps, verification and research utilities |
| `01_Hardware_Service` | Complete 41-page 160X/XT service manual, factory test procedure, original schematic archive, native scans; related models are identified |
| `02_Components_and_Analog_Theory` | Original 2150/2252 data, successor VCA comparisons, dbx/THAT application notes and foundational patents |
| `03_DSP_Papers` | Compressor architecture, RMS timing, VCA nonidealities and model fitting papers |
| `04_Curves_and_Measurement_References` | Graph-page images, curve guide, Plugindoctor manual and REW references |
| `05_Plugin_References` | Vendor manuals, original product images and larger UI images extracted from vendor PDFs |
| `06_Hardware_Visuals` | Manufacturer's 4096 x 2271 hardware reference image |
| `07_Hardware_Measurement_Plan` | Capture protocol, DAW validation plan and a metadata template |
| `DBX-160xt-manual` | Your original 28 JPEG manual scans, unchanged |

The most useful immediate references are:

1. [Complete service manual](01_Hardware_Service/dbx_160X_XT_Service_Manual.pdf): PDF pages 29-33 contain the 160XT factory test procedure, Revision 02, dated August 15, 1990.
2. [Schematic guide](01_Hardware_Service/SCHEMATIC_GUIDE.md): the seven archive scans include overlapping areas and a 160X-marked sheet. They are not seven unique verified XT revisions.
3. [THAT 2252 detector datasheet](02_Components_and_Analog_Theory/Datasheets/THAT_2252_RMS_Detector_Datasheet.pdf) and [2150 VCA datasheet](02_Components_and_Analog_Theory/Datasheets/THAT_2150_Series_VCA_Datasheet.pdf).
4. [Curve and response guide](04_Curves_and_Measurement_References/CURVE_GUIDE.md).
5. [Plugin reference gallery](05_Plugin_References/REFERENCE_GALLERY.md), including a 1900-pixel-wide complete XT plugin interface.

Each downloaded source has a URL, retrieval time, checksum, size and status in [the download manifest](00_Index/download_manifest.json). Extracted images and text have separate [derived-file provenance](00_Index/derived_manifest.json). A failed URL is recorded as unavailable; an HTML error page is never saved under a PDF filename.

This is a broad curated collection, not a claim that the web has been exhausted. No measurements of your hardware or installed plugins have been made. Source illustrations and component graphs are labeled separately from actual XT measurements. Vendor artwork remains reference material; the collection does not grant permission to redistribute it as your plugin's artwork.

Ongoing context and future work are recorded in [PROJECT_MEMORY.md](PROJECT_MEMORY.md).
