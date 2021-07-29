(:*******************************************************:)
(: Test: K-DateTimeEQ-19                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The operator 'ge' is not available between xs:dateTime and xs:date . :)
(:*******************************************************:)
xs:date("1999-12-04") ge
				       xs:dateTime("1999-12-04T12:12:23")