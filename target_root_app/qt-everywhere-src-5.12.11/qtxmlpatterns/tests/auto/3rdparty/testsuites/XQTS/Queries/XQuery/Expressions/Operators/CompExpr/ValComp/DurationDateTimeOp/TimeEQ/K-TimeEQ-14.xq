(:*******************************************************:)
(: Test: K-TimeEQ-14                                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The operator 'gt' is not available between xs:dateTime and xs:date . :)
(:*******************************************************:)
xs:time("12:12:23") gt
				       xs:date("1999-12-04")