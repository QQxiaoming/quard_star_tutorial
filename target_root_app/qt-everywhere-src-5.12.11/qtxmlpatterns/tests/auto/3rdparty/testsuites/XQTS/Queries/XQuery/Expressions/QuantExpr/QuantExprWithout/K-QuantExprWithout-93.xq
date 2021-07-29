(:*******************************************************:)
(: Test: K-QuantExprWithout-93                           :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Nested variable bindings can reference each other. :)
(:*******************************************************:)
deep-equal((for $a in 1, $b in $a, $c in $a, $d in $c return ($a, $b, $c, $d)),
(1, 1, 1, 1))