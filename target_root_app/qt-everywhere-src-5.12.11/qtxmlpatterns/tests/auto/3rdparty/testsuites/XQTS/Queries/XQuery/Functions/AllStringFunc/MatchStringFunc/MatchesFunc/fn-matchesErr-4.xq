(: Name: fn-matchesErr-3 :)
(: Description: single-digit back-reference to group not yet closed :)
(: See erratum FO.E24 :)


fn:matches("#abc#1", "^((#)abc\1)$")