(:*******************************************************:)
(: Test: K-QuantExprWithout-43                           :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A binding in a 'some' quantification shadows global variables. :)
(:*******************************************************:)
declare variable $i := false();
declare variable $t := false();

some $i in (true(), true()), $t in (true(), true()) satisfies $i eq $t