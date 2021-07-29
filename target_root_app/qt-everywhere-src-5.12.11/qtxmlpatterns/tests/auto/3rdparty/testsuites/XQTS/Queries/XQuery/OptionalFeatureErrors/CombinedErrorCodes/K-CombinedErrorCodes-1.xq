(:*******************************************************:)
(: Test: K-CombinedErrorCodes-1                          :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:20+01:00                       :)
(: Purpose: Schema import binding to no namespace, and no location hint. :)
(:*******************************************************:)
import(::)schema(::) "http://example.com/NSNOTRECOGNIZED"; 1 eq 1
	