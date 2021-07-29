(:*******************************************************:)
(: Test: K-QuantExprWithout-67                           :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Variable which is not in scope.              :)
(:*******************************************************:)
some $foo in 1 satisfies 
						$bar + (every $bar in 2 satisfies $bar)