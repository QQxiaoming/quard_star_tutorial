(:*******************************************************:)
(: Test: K-ForExprPositionalVar-25                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Verify that the position is properly computed for fn:subsequence(). :)
(:*******************************************************:)
empty(for $i at $p
	    in subsequence((1, 2, 3, current-time()), 5, 8)
	    return $p)