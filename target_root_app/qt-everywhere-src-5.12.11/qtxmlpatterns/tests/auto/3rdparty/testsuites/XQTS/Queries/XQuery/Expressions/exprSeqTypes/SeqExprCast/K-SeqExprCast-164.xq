(:*******************************************************:)
(: Test: K-SeqExprCast-164                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The canonical form of the xs:duration value -PT0S is PT0S. :)
(:*******************************************************:)
xs:string(xs:duration("-PT0S")) eq "PT0S"