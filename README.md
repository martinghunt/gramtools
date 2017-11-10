[![Build Status](https://travis-ci.org/iqbal-lab-org/gramtools.svg?branch=dev)](https://travis-ci.org/iqbal-lab-org/gramtools)

# gramtools
Genome inference using reference graphs: given a new sample, find a personalised reference which is a recombinant of known genomes. Use that mosaic either as an approximation to the sample, or as basis for standard reference-based variant discovery.

## Install
```sudo pip install git+https://github.com/iqbal-lab-org/gramtools```

## Running
First step, done once per species.
* Generate a graph from known variants (VCF file) and the associated reference genome (gramtools supports this)
Second step: given a new sample, quasimap reads to the graph and infer a personalised reference/
* Infer a reference genome from the graph by analysing read coverage with exact matching.

### Build graph
```gramtools build --gram-directory ./gram --vcf ./vcf --reference ./reference --max-read-length 150```

| parameter           | description                                                     |
|---------------------|-----------------------------------------------------------------|
| `--gram-directory`  | output directory for gramtools build files (created if missing) |
| `--vcf`             | input variant call file detailing genetic variants              |
| `--reference`       | generic reference genome which complements the VCF              |
| `--max-read-length` | maximum read length used during the `quasimap` command          |

### Quasimap reads p
```gramtools quasimap --gram-directory ./gram --reads ./reads```

| parameter          | description                                                     |
|--------------------|-----------------------------------------------------------------|
| `--gram-directory` | output directory for gramtools build files (created if missing) |
| `--reads`          | input read samples fastq file                                   |
