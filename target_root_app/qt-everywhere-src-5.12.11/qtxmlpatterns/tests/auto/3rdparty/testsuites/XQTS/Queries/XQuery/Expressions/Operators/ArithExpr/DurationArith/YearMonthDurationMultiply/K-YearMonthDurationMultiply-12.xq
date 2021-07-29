(:*******************************************************:)
(: Test: K-YearMonthDurationMultiply-12                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The multiplication operator is not available between xs:yearMonthDuration and xs:yearMonthDuration. :)
(:*******************************************************:)
xs:yearMonthDuration("P3Y3M") *
						       xs:yearMonthDuration("P3Y3M")