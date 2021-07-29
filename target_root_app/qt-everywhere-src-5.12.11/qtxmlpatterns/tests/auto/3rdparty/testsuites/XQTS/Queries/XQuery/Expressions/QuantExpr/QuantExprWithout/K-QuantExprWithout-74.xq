(:*******************************************************:)
(: Test: K-QuantExprWithout-74                           :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The 'return' keyword is not valid in a 'some' expression, it must be 'satisfies'. :)
(:*******************************************************:)
some $foo in (1, $2) return 1