(:*******************************************************:)
(: Test: K-YearMonthDurationEQ-4                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of 'ne' for xs:yearMonthDuration. :)
(:*******************************************************:)
xs:yearMonthDuration("P1999Y01M") ne
		   xs:yearMonthDuration("P1999Y10M")