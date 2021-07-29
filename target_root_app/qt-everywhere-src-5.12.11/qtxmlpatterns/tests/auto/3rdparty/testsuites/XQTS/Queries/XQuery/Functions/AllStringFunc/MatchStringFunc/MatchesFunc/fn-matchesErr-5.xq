(: Name: fn-matchesErr-5 :)
(: Description: double-digit back-reference to group not yet closed :)
(: See erratum FO.E24 :)


fn:matches("abcdefghijklmnopq", "(a)(b)(c)(d)(e)(f)(g)(h)(i)(j)(k)(l)((m)(n)(o)(p)(q)\13)$")