(:*******************************************************:)
(: Test: K-ForExprPositionalVar-12                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Verify that the position is properly computed for fn:insert-before(). :)
(:*******************************************************:)
deep-equal((1, 2, 3, 4),
	    for $i at $p
	    in insert-before((1, current-time()), 13, (current-date(), 3))
	    return $p)