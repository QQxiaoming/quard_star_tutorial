(:*******************************************************:)
(: Test: K-YearMonthDurationDivide-3                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of dividing a xs:yearMonthDuration with xs:double('INF'). :)
(:*******************************************************:)
xs:yearMonthDuration("P3Y36M") div xs:double("INF") eq
			   xs:yearMonthDuration("P0M")