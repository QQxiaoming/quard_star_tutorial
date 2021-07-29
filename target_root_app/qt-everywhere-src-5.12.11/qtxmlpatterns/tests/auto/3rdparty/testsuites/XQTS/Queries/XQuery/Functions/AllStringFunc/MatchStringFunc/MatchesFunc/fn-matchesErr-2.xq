(: Name: fn-matchesErr-2 :)
(: Description: back-reference illegal in square brackets :)
(: See erratum FO.E24 :)


fn:matches("#abc#1", "^(#)abc[\1]1$")