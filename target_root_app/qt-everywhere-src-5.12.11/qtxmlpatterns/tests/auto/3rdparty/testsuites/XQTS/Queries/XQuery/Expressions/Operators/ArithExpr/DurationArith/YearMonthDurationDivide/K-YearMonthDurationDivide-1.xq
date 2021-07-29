(:*******************************************************:)
(: Test: K-YearMonthDurationDivide-1                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of dividing a xs:yearMonthDuration with 3. :)
(:*******************************************************:)
xs:yearMonthDuration("P3Y36M") div 3
			eq xs:yearMonthDuration("P2Y")