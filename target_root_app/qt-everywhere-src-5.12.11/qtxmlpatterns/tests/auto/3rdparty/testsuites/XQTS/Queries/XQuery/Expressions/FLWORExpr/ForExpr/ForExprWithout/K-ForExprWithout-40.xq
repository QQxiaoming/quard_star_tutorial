(:*******************************************************:)
(: Test: K-ForExprWithout-40                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Variable which is not in scope.              :)
(:*******************************************************:)
for $foo in 1 return 
						$bar + (for $bar in 2 return $bar)