(:*******************************************************:)
(: Test: K-DayTimeDurationDivide-15                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The division operator is not available between xs:dayTimeDuration and xs:duration. :)
(:*******************************************************:)
xs:dayTimeDuration("P3D") div
						       xs:duration("P3Y3M")