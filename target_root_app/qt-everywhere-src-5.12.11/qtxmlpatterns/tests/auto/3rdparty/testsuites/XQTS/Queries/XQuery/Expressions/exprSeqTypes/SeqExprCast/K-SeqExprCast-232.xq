(:*******************************************************:)
(: Test: K-SeqExprCast-232                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure casting xs:gYear to xs:string, with timezone '+00:00' is properly handled. :)
(:*******************************************************:)
xs:string(xs:gYear("1999+00:00")) eq "1999Z"