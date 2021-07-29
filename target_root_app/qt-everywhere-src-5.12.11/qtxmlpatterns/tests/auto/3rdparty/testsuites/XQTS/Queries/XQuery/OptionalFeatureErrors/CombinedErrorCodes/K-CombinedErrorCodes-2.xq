(:*******************************************************:)
(: Test: K-CombinedErrorCodes-2                          :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:20+01:00                       :)
(: Purpose: Schema import binding to no namespace, but has a location hint. :)
(:*******************************************************:)
import(::)schema(::) "http://example.com/NSNOTRECOGNIZED" at "http://example.com/DOESNOTEXIST"; 1 eq 1
	