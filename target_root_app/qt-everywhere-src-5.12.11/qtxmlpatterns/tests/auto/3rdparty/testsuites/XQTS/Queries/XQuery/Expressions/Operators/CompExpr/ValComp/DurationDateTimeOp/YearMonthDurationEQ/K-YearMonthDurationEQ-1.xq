(:*******************************************************:)
(: Test: K-YearMonthDurationEQ-1                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of 'eq' for xs:yearMonthDuration, returning positive. :)
(:*******************************************************:)
xs:yearMonthDuration("P1999Y10M") eq
		   xs:yearMonthDuration("P1999Y10M")