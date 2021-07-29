(: Name: fn-matches-30 :)
(: Description: 2-digits treated as a back-reference :)
(: See erratum FO.E24 :)


fn:matches("#abcdefghijklmnopq#1", "^(#)(a)(b)(c)(d)(e)(f)(g)(h)(i)(j)(k)(l)(m)(n)(o)(p)(q)\11$")