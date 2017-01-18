set -e
./spec_maker.py > testSpec.structuredslugs
../../tools/StructuredSlugsParser/compiler.py testSpec.structuredslugs > testSpec.slugsin
../../src/slugs --asyncPartitionedTransitions testSpec.slugsin

