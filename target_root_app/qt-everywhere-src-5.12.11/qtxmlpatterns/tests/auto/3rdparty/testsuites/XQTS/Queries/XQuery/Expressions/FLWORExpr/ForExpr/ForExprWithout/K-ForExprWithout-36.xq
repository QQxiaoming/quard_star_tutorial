(:*******************************************************:)
(: Test: K-ForExprWithout-36                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Variable which is not in scope.              :)
(:*******************************************************:)
for $foo in (1, 2, $foo) return 1