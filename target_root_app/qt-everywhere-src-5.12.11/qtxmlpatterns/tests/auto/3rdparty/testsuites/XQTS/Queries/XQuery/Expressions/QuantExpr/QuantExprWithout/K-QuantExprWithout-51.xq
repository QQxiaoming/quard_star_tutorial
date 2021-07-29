(:*******************************************************:)
(: Test: K-QuantExprWithout-51                           :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A test whose essence is: `every $aaa in (3, 3, 3), $bbb in (3, 3, 3) satisfies $aaa + $bbb eq 6`. :)
(:*******************************************************:)
every $aaa in (3, 3, 3), $bbb in (3, 3, 3) satisfies $aaa + $bbb eq 6