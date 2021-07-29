(:*******************************************************:)
(: Test: K2-ForExprWithout-42                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use a computed attribute constructor with name return, inside a for loop. :)
(:*******************************************************:)
for $n in attribute return {()} return 1