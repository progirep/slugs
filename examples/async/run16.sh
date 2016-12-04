set -e
../../tools/StructuredSlugsParser/compiler.py exampleSpec16.structuredslugs > exampleSpec16.slugsin 
../../src/slugs --asyncPartitionedTransitions exampleSpec16.slugsin

