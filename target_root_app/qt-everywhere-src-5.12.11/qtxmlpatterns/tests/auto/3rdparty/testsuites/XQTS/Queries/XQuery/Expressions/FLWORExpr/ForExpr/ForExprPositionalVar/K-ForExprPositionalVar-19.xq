(:*******************************************************:)
(: Test: K-ForExprPositionalVar-19                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Verify that the position is properly computed for fn:remove(). :)
(:*******************************************************:)
deep-equal((1, 2, 3),
	    for $i at $p
	    in remove((1, 2, current-time()), 10)
	    return $p)