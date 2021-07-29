(:*******************************************************:)
(: Test: K-ForExprPositionalVar-16                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Verify that the position is properly computed for the range expression(#2). :)
(:*******************************************************:)
deep-equal((1, 2, 3, 4),
	    for $i at $p
	    in -10 to -7
	    return $p)