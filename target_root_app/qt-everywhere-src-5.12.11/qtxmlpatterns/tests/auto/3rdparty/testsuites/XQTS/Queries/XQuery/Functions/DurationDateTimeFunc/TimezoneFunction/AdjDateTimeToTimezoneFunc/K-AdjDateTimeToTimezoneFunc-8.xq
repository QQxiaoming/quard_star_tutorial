(:*******************************************************:)
(: Test: K-AdjDateTimeToTimezoneFunc-8                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Passing a too small xs:dayTimeDuration as timezone to adjust-dateTime-to-timezone(). :)
(:*******************************************************:)
adjust-dateTime-to-timezone(xs:dateTime("2001-02-03T08:02:00"),
							xs:dayTimeDuration("-PT14H1M"))