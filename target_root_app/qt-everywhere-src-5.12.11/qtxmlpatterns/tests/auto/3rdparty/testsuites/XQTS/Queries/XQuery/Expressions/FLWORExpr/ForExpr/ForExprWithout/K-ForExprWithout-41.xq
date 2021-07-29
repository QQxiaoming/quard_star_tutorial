(:*******************************************************:)
(: Test: K-ForExprWithout-41                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Variable which is not in scope.              :)
(:*******************************************************:)
for $a in (1, 2, 3), $b in (1, 2, 3, $b)
return $a, $b