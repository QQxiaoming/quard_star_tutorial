(:*******************************************************:)
(: Test: K-YearMonthDurationGT-3                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of 'gt' for xs:yearMonthDuration, evaluating to false. :)
(:*******************************************************:)
not(xs:yearMonthDuration("P1999Y10M") gt
			   xs:yearMonthDuration("P1999Y11M"))