(: Name: fn-matchesErr-3 :)
(: Description: single-digit back-reference to non-existent group :)
(: See erratum FO.E24 :)


fn:matches("#abc#1", "^(#)abc\2$")