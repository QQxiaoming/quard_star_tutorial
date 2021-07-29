(:*******************************************************:)
(: Test: K-DayTimeDurationDivide-16                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The division operator is not available between xs:duration and xs:dayTimeDuration. :)
(:*******************************************************:)
xs:duration("P3Y3M") div
						       xs:dayTimeDuration("P3D")