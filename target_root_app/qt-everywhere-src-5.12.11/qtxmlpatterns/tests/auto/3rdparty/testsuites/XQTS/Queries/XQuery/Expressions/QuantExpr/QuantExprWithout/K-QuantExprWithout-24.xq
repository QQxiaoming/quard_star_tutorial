(:*******************************************************:)
(: Test: K-QuantExprWithout-24                           :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A test whose essence is: `some $var in (true(), false(), true()) satisfies $var`. :)
(:*******************************************************:)
some $var in (true(), false(), true()) satisfies $var