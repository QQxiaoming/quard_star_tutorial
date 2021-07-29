(:*******************************************************:)
(: Test: K-AdjDateTimeToTimezoneFunc-11                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Example from F&O.                            :)
(:*******************************************************:)

			adjust-dateTime-to-timezone(xs:dateTime("2002-03-07T10:00:00-07:00"),
						    xs:dayTimeDuration("-PT5H0M"))
			eq xs:dateTime("2002-03-07T12:00:00-05:00")
		