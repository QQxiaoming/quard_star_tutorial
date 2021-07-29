(:*******************************************************:)
(: Test: K-ForExprPositionalVar-21                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Verify that the position is properly computed for fn:remove(). :)
(:*******************************************************:)
deep-equal((1, 2, 3),
	    for $i at $p
	    in remove((1, 2, 3, current-time()), 1)
	    return $p)