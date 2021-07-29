(:*******************************************************:)
(: Test: K-QuantExprWithout-4                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: $foo has static type xs:integer; which cannot be compared to xs:string. :)
(:*******************************************************:)
every $foo in 1 satisfies $foo eq "1"