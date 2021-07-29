(:*******************************************************:)
(: Test: K-QuantExprWithout-33                           :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Since EBV cannot be extracted from xs:hexBinary, FORG0006 is allowed. :)
(:*******************************************************:)
some $var in (true(), true(), xs:hexBinary("FF")) satisfies $var