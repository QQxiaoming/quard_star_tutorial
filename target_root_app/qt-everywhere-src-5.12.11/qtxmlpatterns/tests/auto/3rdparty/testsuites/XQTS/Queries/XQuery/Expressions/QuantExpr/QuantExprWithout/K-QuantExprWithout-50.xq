(:*******************************************************:)
(: Test: K-QuantExprWithout-50                           :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A test whose essence is: `some $aaa in (1, 2, 3), $bbb in (3, 2, 1) satisfies $aaa + $bbb eq 4`. :)
(:*******************************************************:)
some $aaa in (1, 2, 3), $bbb in (3, 2, 1) satisfies $aaa + $bbb eq 4