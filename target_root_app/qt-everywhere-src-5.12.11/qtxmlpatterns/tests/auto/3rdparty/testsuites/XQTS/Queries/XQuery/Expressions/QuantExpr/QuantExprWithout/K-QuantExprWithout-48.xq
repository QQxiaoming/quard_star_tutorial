(:*******************************************************:)
(: Test: K-QuantExprWithout-48                           :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A test whose essence is: `every $var in (1, 2, 3) satisfies $var eq 1 or $var eq 2 or $var eq 3`. :)
(:*******************************************************:)
every $var in (1, 2, 3) satisfies $var eq 1 or $var eq 2 or $var eq 3