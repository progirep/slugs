set -e
tools/StructuredSlugsParser/compiler.py exampleSpec.structuredslugs > exampleSpec.slugsin 
src/slugs --asyncPartitionedTransitions exampleSpec.slugsin

