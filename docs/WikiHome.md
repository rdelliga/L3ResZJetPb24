# Residual Analysis Wiki

This is the short workflow index for the repository.

## Navigation

- [L2Residual](L2Residual.md)
- [JER](JER.md)
- [L3Residual](L3Residual.md)
- [Systematics](Systematics.md)
- [Batch](Batch.md)

## Setup

```bash
cmsrel CMSSW_15_1_0_patch3
cd CMSSW_15_1_0_patch3/src
cmsenv
git clone <your-project-url>
cd residualanalysis
```

Compile helper classes after changing histogram definitions:

```bash
cd fillhistograms
root -l -b -q compile.C
cd ..
```

## What each page highlights

| Page | Focus |
| --- | --- |
| [L2Residual](L2Residual.md) | the histogram chain from `analyse.cc` to L2 text output |
| [JER](JER.md) | MC resolution and JER scale-factor chains |
| [L3Residual](L3Residual.md) | the photon+jet histogram chain to L3 and L2L3 text output |
| [Systematics](Systematics.md) | centralized L2 uncertainty text production |
| [Batch](Batch.md) | histogram-production batch submission only |