(:*******************************************************:)
(: Test: K-QuantExprWith-24                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Every-quantification with type-declaration.  :)
(:*******************************************************:)
some $a as xs:integer in (1, 2), $b as xs:integer in $a satisfies $b