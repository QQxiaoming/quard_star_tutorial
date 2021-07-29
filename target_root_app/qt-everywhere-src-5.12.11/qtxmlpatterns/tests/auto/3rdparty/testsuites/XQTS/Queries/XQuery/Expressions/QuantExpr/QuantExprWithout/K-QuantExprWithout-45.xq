(:*******************************************************:)
(: Test: K-QuantExprWithout-45                           :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A binding in a 'every' quantification shadows global variables. :)
(:*******************************************************:)
declare variable $i := false();
		every $i in (true(), true()) satisfies $i