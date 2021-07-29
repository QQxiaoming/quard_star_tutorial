(:*******************************************************:)
(: Test: K-ForExprWithout-44                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Variable which is not in scope.              :)
(:*******************************************************:)
for $a in (1, 2), $b in (1, 2), $c in (1, 2) return 1, $b