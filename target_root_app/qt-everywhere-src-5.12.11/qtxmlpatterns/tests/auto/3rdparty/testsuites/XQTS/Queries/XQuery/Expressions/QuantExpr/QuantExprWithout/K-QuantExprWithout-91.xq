(:*******************************************************:)
(: Test: K-QuantExprWithout-91                           :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Nested variable bindings can reference each other. :)
(:*******************************************************:)
every $a in (1, 2), $b in $a satisfies $b