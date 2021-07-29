(:*******************************************************:)
(: Test: K-ForExprPositionalVar-27                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Verify that the position is properly computed for fn:subsequence(). :)
(:*******************************************************:)
deep-equal((1, 2),
	    for $i at $p
	    in subsequence((1, 2, 3, current-time()), 1, 2)
	    return $p)