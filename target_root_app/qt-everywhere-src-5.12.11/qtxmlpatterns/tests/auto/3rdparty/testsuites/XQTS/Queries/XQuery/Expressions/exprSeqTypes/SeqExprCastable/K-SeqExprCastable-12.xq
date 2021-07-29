(:*******************************************************:)
(: Test: K-SeqExprCastable-12                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: An invalid type for 'castable as' is specified. :)
(:*******************************************************:)
(xs:double(1), xs:double(2), xs:double(3))
		castable as xs:double*