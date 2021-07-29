(:*******************************************************:)
(: Test: K-AdjTimeToTimezoneFunc-6                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Passing a too large xs:dayTimeDuration as timezone to adjust-time-to-timezone(). :)
(:*******************************************************:)
adjust-time-to-timezone(xs:time("08:02:00"),
							xs:dayTimeDuration("PT14H1M"))