(:*******************************************************:)
(: Test: K-ForExprWithout-9                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A test whose essence is: `3 eq (for $foo in 1 return for $foo in 3 return $foo)`. :)
(:*******************************************************:)
3 eq (for $foo in 1 return
					for $foo in 3 return $foo)