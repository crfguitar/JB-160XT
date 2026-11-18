# Research log, access limits and open questions

Search/retrieval date: 2026-11-15. The collection is broad but finite. No paid purchase, account login, plugin installation or DAW change was required.

## Search coverage

Searched for XT service/calibration documents, schematics, legacy VCA/RMS data, compressor timing mathematics, hard/soft-knee and negative-ratio curves, measured frequency response/distortion, original vendor UI assets, explicit XT emulations and future hardware measurement tools. Prioritized manufacturer, component vendor, author/university and conference sources. Independent archives are used for manufacturer-authored legacy documents.

## Problems encountered and resolutions

| Problem | Resolution / consequence |
| --- | --- |
| Session interrupted unexpectedly | Continued from saved files. Cause was not established; do not attribute it to user action |
| Environment proxy variables pointed to an unavailable local proxy | The research downloader uses an available direct connection with `requests.Session.trust_env = False`; no global settings were changed |
| Manufacturer pages returned inconsistent 403 responses | Retried through normal requests; product and archive-index pages were eventually saved. An individual schematic endpoint remained inaccessible in that attempt |
| Archive download URL returned an HTML interstitial with HTTP 200 | Followed the page's published direct-download link and validated actual file bytes |
| First service scan had 28 pages and lacked calibration appendices | Replaced it with the complete 41-page scan from an alternate archive; verified the XT test procedure at pp29-33 |
| Original university paper URL returned non-PDF content | Downloaded the same paper from the author's GitHub-hosted document mirror |
| Product images were smaller than PDF-embedded screenshots | Extracted native 1900-pixel-wide Comp160 UI images, including the XT interface, from the official manual |
| Schematic archive used XT in its name but contained overlapping and related-model scans | Preserved original filenames and added explicit page/model caveats; no automated circuit transcription was claimed |
| Scanned pages had poor/missing text or OCR errors | Kept originals/native scans, extracted available text with page markers, and warned against trusting OCR for engineering values |
| Generic searches mixed XT, original 160/VU and later 160A information | Marked each source's model scope; separated related models and plugin comparators |

## Unavailable local downloads

The machine-readable manifest records four failed local downloads at the end of collection: HW04 (alternate Audiovias schematic scan), WEB10 (AES bibliography page), WEB15 (ManualsLib XT test index), and WEB16 (ManualsLib service index). Their references remain in the source index. No failed response was saved as the requested file type.

The AES bibliographic information and ManualsLib test/service listings were readable through web search/browsing even though direct local retrieval failed. Equivalent substantive content is downloaded: the author-hosted DSP paper and the complete service manual with the factory procedure. The missing alternate schematic is not the only copy of that subject.

Google Patents pages also returned 503 to direct local requests. Their published PDF links were obtained through browsing, and three foundational Blackmer patent PDFs were successfully downloaded. A fourth potential Glaberson patent was not downloaded or treated as evidence.

## Important unresolved questions

- Exact revision and condition of the user's unit, including optional transformer or service changes.
- Installed VCA and RMS part markings; correspondence between physical PCB and legacy drawing revisions.
- Actual OverEasy curve, knob taper, negative-ratio response, detector timing/ripple and history dependence across settings.
- Main-path versus detector frequency response, and how either changes with gain, level, load and connector.
- Gain-reduction LED count and meter timing on the user's panel; published documents disagree on the count.
- Exact calibrated converter mapping between dBFS and dBu, polarity and round-trip response.
- Stereo strapping behavior on a real pair; one mono unit alone cannot validate every stereo-link detail.
- No verified public measured XT dataset with enough settings and calibration detail was obtained. No traces were invented to fill this gap.

## Useful future leads

- [Harman dbx service-document portal](https://assets.harman.com/portals/fyhfpbwr/dbxServiceDocuments), linked by the current manufacturer pages, may offer revised scans or additional service material.
- [Original dbx schematic index](https://dbxpro.com/en-US/downloads/en/schematic) and the local source snapshot provide official archive context.
- [ManualsLib factory-test listing](https://www.manualslib.com/manual/4544417/Dbx-160xt.html) offers a separately indexed version of the same XT procedure.
- Hardware measurements from the owned specimen will resolve more implementation uncertainty than additional uncalibrated plugin screenshots or subjective comparisons.

## Retrieval and local search

`source_catalog.json` is the curated URL list. `tools/collect_research.py` validates PDFs, archives and images before saving, skips already retrieved sources, and records failures. `tools/build_reference_views.py` rebuilds extracted native images, graph views, searchable text and source index. These tools download documents only; they never execute downloaded plugin software.

Use `rg -n -i 'overeasy|symmetry|time constant|threshold' 00_Index/Extracted_Text` for targeted research. Confirm matches in the original document, especially mathematical expressions, resistor values, connector pin numbers and dBm/dBu notation.
