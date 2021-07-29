(: Name: fn-matches-29 :)
(: Description: 2-digits not treated as a back-reference :)
(: See erratum FO.E24 :)


fn:matches("#abc#1", "^(#)abc\11$")