(:*******************************************************:)
(: Test: K-YearMonthDurationMultiply-10                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The multiplication operator is not available between xs:dayTimeDuration and xs:yearMonthDuration. :)
(:*******************************************************:)
xs:dayTimeDuration("P3D") *
						       xs:yearMonthDuration("P3Y3M")