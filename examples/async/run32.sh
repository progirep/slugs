set -e
../../tools/StructuredSlugsParser/compiler.py exampleSpec32.structuredslugs > exampleSpec32.slugsin 
../../src/slugs --asyncPartitionedTransitions exampleSpec32.slugsin

