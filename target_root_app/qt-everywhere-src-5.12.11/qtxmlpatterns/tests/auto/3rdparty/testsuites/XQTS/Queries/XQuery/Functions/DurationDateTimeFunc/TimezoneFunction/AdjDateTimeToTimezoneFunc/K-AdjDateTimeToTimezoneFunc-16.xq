(:*******************************************************:)
(: Test: K-AdjDateTimeToTimezoneFunc-16                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Example from F&O.                            :)
(:*******************************************************:)

			adjust-dateTime-to-timezone(xs:dateTime("2002-03-07T10:00:00"),
						    ())
			eq xs:dateTime("2002-03-07T10:00:00")
		