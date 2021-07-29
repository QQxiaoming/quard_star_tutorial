(:*******************************************************:)
(: Test: K-CombinedErrorCodes-3                          :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:20+01:00                       :)
(: Purpose: Schema import binding to no namespace, but has three location hints. :)
(:*******************************************************:)
import(::)schema(::)"http://example.com/NSNOTRECOGNIZED"(::)at(::)"http://example.com/DOESNOTEXIST",
		(::)"http://example.com/2",(::)"http://example.com/3"; 1 eq 1
	