# Hardware source guide

[HW01: complete service manual](dbx_160X_XT_Service_Manual.pdf) has 41 PDF pages. The first archive mirror returned a 28-page file that stopped before the factory procedure. The retained file is the complete alternate scan, with text/OCR on most pages. The manufacturer authored the document; an independent archive hosts this copy.

| PDF pages | Contents |
| --- | --- |
| 1-25 | XT setup, controls, transfer illustrations, applications, routing, connections and specifications |
| 26-28 | 160X description and controls; identify these as related-model content |
| 29-33 | 160XT Test Procedure, Revision 02, 1990-08-15: RMS/VCA symmetry, meters, threshold, infinity, OverEasy and performance tests |
| 34 | Service parts list with handwritten annotations |
| 35-41 | Assembly/circuit appendices, with sparse or missing text layers |

Do not apply internal trim adjustments merely to make a captured unit resemble the paper specifications. First document and measure its current condition. The procedure is preserved as engineering evidence and to interpret expected behavior.

## Separate schematic archive

[HW02 original ZIP](Original_Archives/dbx_160XT_Schematics.zip) is retained, with its seven PDFs extracted under [XT_Schematics](XT_Schematics). The filenames are original archive names. Native embedded scan pixels are also available under [Native_Scans](XT_Schematics/Native_Scans), approximately 5072-5088 x 3295 pixels.

The archive title overstates its uniformity. The scans include cropped edges, overlapping areas, handwritten labels and related-model material. Do not interpret page numbers as seven distinct XT circuit sheets or assume all drawings describe the user's PCB revision.

| Archive page | Visual contents / limitation |
| --- | --- |
| 1 | PCB assembly/component placement; overlaps page 7 |
| 2 | Audio/VCA/output and detector/control areas; handwritten XT label; edges cropped |
| 3 | Overlapping control/detector circuit region; handwritten XT label |
| 4 | Meter/status/control region and drawing border; verify original title/revision at full resolution |
| 5 | Strong overlap with page 3; keep original for differences in scanning/legibility |
| 6 | Rotated circuit drawing explicitly marked **MODEL 160X**; comparison evidence |
| 7 | PCB assembly/component placement; overlaps page 1 |

The [contact sheet](../00_Index/Verification/XT_Schematics_Contact_Sheet.jpg) helps navigation. Use the original PDFs/native scans for component values; thumbnails are insufficient for transcription.

The [related-model folder](Related_Models) contains the separate 160X schematic and manufacturer's 160A manual/cut sheet. The 160A manual specifically warns that its hardware cannot stereo-couple with X/XT units. Similar family names do not establish identical linking circuitry.

Before a circuit transcription, record the physical unit's serial number, PCB/drawing revisions, VCA and RMS markings, any replaced components, transformer option and main output connector in use. Resolve disagreements using the matching revision and measurements.
