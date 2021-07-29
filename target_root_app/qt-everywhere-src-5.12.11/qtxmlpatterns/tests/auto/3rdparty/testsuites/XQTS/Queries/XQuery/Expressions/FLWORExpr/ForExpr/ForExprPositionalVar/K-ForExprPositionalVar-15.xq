(:*******************************************************:)
(: Test: K-ForExprPositionalVar-15                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Verify that the position is properly computed for the range expression. :)
(:*******************************************************:)
deep-equal((1, 2, 3, 4),
	    for $i at $p
	    in 1 to 4
	    return $p)