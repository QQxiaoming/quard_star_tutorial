(:*******************************************************:)
(: Test: K-AdjTimeToTimezoneFunc-8                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Passing a xs:dayTimeDuration as timezone to adjust-time-to-timezone() which isn't an integral number of minutes. :)
(:*******************************************************:)
adjust-time-to-timezone(xs:time("08:02:00"),
							xs:dayTimeDuration("PT14H0M0.001S"))