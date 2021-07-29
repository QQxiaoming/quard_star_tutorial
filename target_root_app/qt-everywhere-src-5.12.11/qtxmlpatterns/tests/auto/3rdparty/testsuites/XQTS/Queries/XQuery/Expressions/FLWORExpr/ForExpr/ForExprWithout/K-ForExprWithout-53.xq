(:*******************************************************:)
(: Test: K-ForExprWithout-53                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure the correct variable is used in an for-expression whose return sequence is only a variable reference. :)
(:*******************************************************:)

		   declare variable $my := 3;

		   (for $i in 1 return $my) eq 3