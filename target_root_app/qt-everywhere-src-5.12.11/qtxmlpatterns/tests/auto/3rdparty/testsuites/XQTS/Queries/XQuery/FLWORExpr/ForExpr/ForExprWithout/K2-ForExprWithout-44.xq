(:*******************************************************:)
(: Test: K2-ForExprWithout-44                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use a computed processing-instruction constructor with name return, inside a for loop. :)
(:*******************************************************:)
for $n in processing-instruction return {()} return 1