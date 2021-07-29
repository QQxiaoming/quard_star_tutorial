(:*******************************************************:)
(: Test: K-AdjDateToTimezoneFunc-8                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Passing a xs:dayTimeDuration as timezone to adjust-date-to-timezone() which isn't an integral number of minutes. :)
(:*******************************************************:)
adjust-date-to-timezone(xs:date("2001-02-03"),
							xs:dayTimeDuration("PT14H0M0.001S"))