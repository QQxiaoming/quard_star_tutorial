(:*******************************************************:)
(: Test: K-QuantExprWithout-81                           :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Variable which is not in scope.              :)
(:*******************************************************:)
every $foo in (1, $foo, 3) satisfies 1