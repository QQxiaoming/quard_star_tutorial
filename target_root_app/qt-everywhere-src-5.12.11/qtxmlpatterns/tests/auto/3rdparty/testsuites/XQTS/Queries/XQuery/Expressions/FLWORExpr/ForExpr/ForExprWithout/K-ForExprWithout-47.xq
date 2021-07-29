(:*******************************************************:)
(: Test: K-ForExprWithout-47                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A variable's for expression causes type error in a value comparison. :)
(:*******************************************************:)
for $foo in ("a string", "another one") return
							1 + subsequence($foo, 1, 1)