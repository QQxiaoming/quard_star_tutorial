(:*******************************************************:)
(: Test: K-SeqExprCast-101                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A test whose essence is: `count(xs:hexBinary(xs:hexBinary("03"))) eq 1`. :)
(:*******************************************************:)
count(xs:hexBinary(xs:hexBinary("03"))) eq 1