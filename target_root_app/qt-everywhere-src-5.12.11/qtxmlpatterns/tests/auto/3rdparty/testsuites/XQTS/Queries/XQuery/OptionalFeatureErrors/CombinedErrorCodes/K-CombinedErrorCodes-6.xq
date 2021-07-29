(:*******************************************************:)
(: Test: K-CombinedErrorCodes-6                          :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:20+01:00                       :)
(: Purpose: Schema import binding to the default element namespace, and has three location hints. :)
(:*******************************************************:)
import(::)schema(::)(::)default(::)element(::)namespace
		(::)"http://example.com/NSNOTRECOGNIZED"(::)
		at(::)"http://example.com/DOESNOTEXIST",(::)"http://example.com/2DOESNOTEXIST",
		"http://example.com/3DOESNOTEXIST"; 1 eq 1
	