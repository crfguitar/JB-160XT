# Collection verification

Audit date: 2026-11-15.

- 64 successfully downloaded source items from 68 catalog entries.
- Source types: {'.pdf': 37, '.zip': 1, '.webp': 1, '.jpg': 4, '.png': 6, '.html': 15}.
- Downloaded original bytes: 95,027,491 (95.03 MB, decimal).
- 64 tracked derived files: native image extractions, source PDFs extracted from ZIP, graph renders and searchable text.
- Every downloaded original exists and matches its recorded SHA-256 hash.
- All downloaded PDFs reopen, retain the recorded page count, and render their final page at thumbnail scale.
- Downloaded image files pass image decoding verification.
- Derived files match their recorded hashes and their current parent-source hashes.
- All local Markdown links resolve.
- The complete service manual has 41 pages, including the 5-page XT factory procedure at PDF pages 29-33.
- Source graphs, schematic/contact sheets, the XT plugin image and a factory-test page were visually inspected.
- The 28 original manual JPEGs remain in their original folder; no edit or rename operation targeted them.
- No hardware measurements, DSP implementation, plugin installation, DAW edit or paid purchase was performed.

Audit errors: none.

Four unavailable alternative/index downloads remain explicitly marked in the manifest. Their error responses were not retained as documents. Source resolution and OCR limitations remain as documented in the guides.