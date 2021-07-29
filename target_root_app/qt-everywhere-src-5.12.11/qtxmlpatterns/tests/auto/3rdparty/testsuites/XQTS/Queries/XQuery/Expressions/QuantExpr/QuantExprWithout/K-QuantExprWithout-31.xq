(:*******************************************************:)
(: Test: K-QuantExprWithout-31                           :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: EBV cannot be extracted fro xs:hexBinary.    :)
(:*******************************************************:)
some $var in (xs:hexBinary("FF"), false(), true()) satisfies $var