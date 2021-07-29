xquery version "1.1";

(: subtle distinction in the positioning of an empty captured group :)

<out>
  <one>{analyze-string("banana", "(b)(x?)")}</one>
  <two>{analyze-string("banana", "(b(x?))")}</two>
</out>