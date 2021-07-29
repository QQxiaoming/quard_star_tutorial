(:*******************************************************:)
(: Test: K-QuantExprWithout-42                           :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A binding in a 'some' quantification shadows global variables. :)
(:*******************************************************:)
declare variable $i := false();
		some $i in (true(), true(), true()) satisfies $i