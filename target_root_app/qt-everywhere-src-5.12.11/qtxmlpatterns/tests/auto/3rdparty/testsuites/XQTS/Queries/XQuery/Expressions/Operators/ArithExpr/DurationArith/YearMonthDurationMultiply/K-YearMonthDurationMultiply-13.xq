(:*******************************************************:)
(: Test: K-YearMonthDurationMultiply-13                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The multiplication operator is not available between xs:dayTimeDuration and xs:dayTimeDuration. :)
(:*******************************************************:)
xs:dayTimeDuration("P3D") *
						       xs:dayTimeDuration("P3D")